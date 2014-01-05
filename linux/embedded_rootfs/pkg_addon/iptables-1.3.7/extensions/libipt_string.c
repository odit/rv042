/* Shared library add-on to iptables to add string matching support. 
 * 
 * Copyright (C) 2000 Emmanuel Roger  <winfield@freegates.be>
 *
 * 2005-08-05 Pablo Neira Ayuso <pablo@eurodev.net>
 * 	- reimplemented to use new string matching iptables match
 * 	- add functionality to match packets by using window offsets
 * 	- add functionality to select the string matching algorithm
 *
 * ChangeLog
 *     29.12.2003: Michael Rash <mbr@cipherdyne.org>
 *             Fixed iptables save/restore for ascii strings
 *             that contain space chars, and hex strings that
 *             contain embedded NULL chars.  Updated to print
 *             strings in hex mode if any non-printable char
 *             is contained within the string.
 *
 *     27.01.2001: Gianni Tedesco <gianni@ecsc.co.uk>
 *             Changed --tos to --string in save(). Also
 *             updated to work with slightly modified
 *             ipt_string_info.
 */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <iptables.h>
#include <stddef.h>
#include <linux/netfilter_ipv4/ipt_string.h>
#include <linux/netfilter_ipv4/ipt_webstr.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"webstr99 match v%s options:\n"
"--from                       Offset to start searching from\n"
"--to                         Offset to stop searching\n"
"--algo	                      Algorithm\n"
"--url [!] string          Match a http string in a packet\n"
"--host [!] string      Match a http string in a packet\n"
"--content [!] string      Match a http string in a packet\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "from", 1, 0, '1' },
	{ "to", 1, 0, '2' },
	{ "algo", 1, 0, '3' },
	{ "url", 1, 0, '4' },
	{ "host", 1, 0, '5' },
	{ "content", 1, 0, '6' },
	{0}
};

static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	struct ipt_string_info *i = (struct ipt_string_info *) m->data;

	if (i->to_offset == 0)
		i->to_offset = (u_int16_t) ~0UL;
}

static void
parse_string(const char *s, struct ipt_string_info *info)
{	
	if (strlen(s) <= IPT_STRING_MAX_PATTERN_SIZE) {
		strncpy(info->pattern, s, IPT_STRING_MAX_PATTERN_SIZE);
		info->patlen = strlen(s);
		return;
	}
	exit_error(PARAMETER_PROBLEM, "STRING too long `%s'", s);
}

static void
parse_algo(const char *s, struct ipt_string_info *info)
{
	if (strlen(s) <= IPT_STRING_MAX_ALGO_NAME_SIZE) {
		strncpy(info->algo, s, IPT_STRING_MAX_ALGO_NAME_SIZE);
		return;
	}
	exit_error(PARAMETER_PROBLEM, "ALGO too long `%s'", s);
}


#define STRING 0x1
#define ALGO   0x2
#define FROM   0x4
#define TO     0x8


/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_string_info *stringinfo = (struct ipt_string_info *)(*match)->data;

	switch (c) {
	case '1':
		if (*flags & FROM)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify multiple --from");
		stringinfo->from_offset = atoi(optarg);
		*flags |= FROM;
		stringinfo->type = 0;
		break;
	case '2':
		if (*flags & TO)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify multiple --to");
		stringinfo->to_offset = atoi(optarg);
		*flags |= TO;
		stringinfo->type = 0;
		break;
	case '3':
		if (*flags & ALGO)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify multiple --algo");
		parse_algo(optarg, stringinfo);
		*flags |= ALGO;
		stringinfo->type = 0;
		break;
	case '4':
		check_inverse(optarg, &invert, &optind, 0);
		parse_string(argv[optind-1], stringinfo);
		if (invert)
			stringinfo->invert = 1;
		stringinfo->patlen=strlen((char *)&stringinfo->pattern);
		stringinfo->type = IPT_WEBSTR_URL;
		break;

	case '5':
		check_inverse(optarg, &invert, &optind, 0);
		parse_string(argv[optind-1], stringinfo);
		if (invert)
			stringinfo->invert = 1;
		stringinfo->patlen=strlen((char *)&stringinfo->pattern);
		stringinfo->type = IPT_WEBSTR_HOST;
		break;
        case '6':
               check_inverse(optarg, &invert, &optind, 0);
               parse_string(argv[optind-1], stringinfo);
                if (invert)
                        stringinfo->invert = 1;
	                stringinfo->patlen=strlen((char *)&stringinfo->pattern);
                stringinfo->type = IPT_WEBSTR_CONTENT;
                break;
	default:
		return 0;
	}
	*flags = 1;
	return 1;
}


/* Final check; must have specified --string. */
static void
final_check(unsigned int flags)
{
	if (!(flags & STRING))
		exit_error(PARAMETER_PROBLEM,
			   "STRING match: You must specify `--url' or "
			   "`--host' or '--content'");
//	if (!(flags & ALGO))
//		exit_error(PARAMETER_PROBLEM,
//			   "STRING match: You must specify `--algo'");
}

static void
print_string(const char *str, const unsigned short int len)
{
	unsigned int i;
	printf("\"");
	for (i=0; i < len; i++) {
		if ((unsigned char) str[i] == 0x22)  /* escape any embedded quotes */
			printf("%c", 0x5c);
		printf("%c", (unsigned char) str[i]);
	}
	printf("\" ");  /* closing space and quote */
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	const struct ipt_string_info *info = (const struct ipt_string_info*) match->data;
	//struct ipt_string_info *stringinfo = (struct ipt_string_info *)(*match)->data;

        printf("WEBSTR match ");

        switch (info->type) {
        case IPT_WEBSTR_HOST:
                printf("host ");
                break;

        case IPT_WEBSTR_URL:
                printf("url ");
                break;

        case IPT_WEBSTR_CONTENT:
                printf("content ");
                break;

	default:
                printf("ERROR ");
                break;
        }

	print_string(info->pattern, info->patlen);

}


/* Saves the union ipt_matchinfo in parseable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	const struct ipt_string_info *info =
	    (const struct ipt_string_info*) match->data;

	printf("save\n");

	printf("--webstr %s", (info->invert) ? "! ": "");
	print_string(info->pattern, info->patlen);

//	printf("--algo %s ", info->algo);
//	if (info->from_offset != 0)
//		printf("--from %u ", info->from_offset);
//	if (info->to_offset != 0)
//		printf("--to %u ", info->to_offset);
}

static struct iptables_match string = {
    .next		= NULL,
    .name		= "string",
    .version		= IPTABLES_VERSION,
    .size			= IPT_ALIGN(sizeof(struct ipt_string_info)),
    .userspacesize		= IPT_ALIGN(sizeof(struct ipt_string_info)),
    .help		= help,
    .init		= init,
    .parse		= parse,
    .final_check		= final_check,
    .print		= print,
    .save		= save,
    .extra_opts		= opts
};


void _init(void)
{
	register_match(&string);
}
