/* String matching match for iptables
 * 
 * (C) 2005 Pablo Neira Ayuso <pablo@eurodev.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_string.h>
#include <linux/netfilter_ipv4/ipt_webstr.h>
#include <linux/textsearch.h>
#include <net/tcp.h>
#include <net/sock.h>

MODULE_AUTHOR("Pablo Neira Ayuso <pablo@eurodev.net>");
MODULE_DESCRIPTION("IP tables string match module");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_string");
MODULE_ALIAS("ip6t_string");

int nk_denypolicy = 0;
EXPORT_SYMBOL(nk_denypolicy);

#define split(word, wordlist, next, delim) \
	    for (next = wordlist, \
        strncpy(word, next, sizeof(word)), \
	        word[(next=strstr(next, delim)) ? strstr(word, delim) - word : sizeof(word) - 1] = '\0', \
          next = next ? next + sizeof(delim) - 1 : NULL ; \
	        strlen(word); \
	        next = next ? : "", \
	        strncpy(word, next, sizeof(word)), \
	        word[(next=strstr(next, delim)) ? strstr(word, delim) - word : sizeof(word) - 1] = '\0', \
	        next = next ? next + sizeof(delim) - 1 : NULL)


#define BUFSIZE         1024
	
/* Flags for get_http_info() */
#define HTTP_HOST       0x01
#define HTTP_URL        0x02
/* Flags for mangle_http_header() */
#define HTTP_COOKIE     0x04

#if 0
#define SPARQ_LOG       printk
#else
#define SPARQ_LOG(format, args...)
#endif

//webstr ----------------------------
typedef struct httpinfo {
    char host[BUFSIZE + 1];
    int hostlen;
    char url[BUFSIZE + 1];
    int urllen;
} httpinfo_t;

/* Return 1 for match, 0 for accept, -1 for partial. */
static int find_pattern2(const char *data, size_t dlen,
       const char *pattern, size_t plen,
       char term,
       unsigned int *numoff,
       unsigned int *numlen)
{
	size_t i, j, k;
	int state = 0;
	*numoff = *numlen = 0;

	if (dlen == 0)
        	return 0;

	if (dlen <= plen)
	{ /* Short packet: try for partial? */
		if (strnicmp(data, pattern, dlen) == 0)
	            return -1;
        	else
	            return 0;
	}
	for (i = 0; i <= (dlen - plen); i++)
	{
	        /* DFA : \r\n\r\n :: 1234 */
	        if (*(data + i) == '\r') {
	            if (!(state % 2)) state++;  /* forwarding move */
	            else state = 0;             /* reset */
	        }
	        else if (*(data + i) == '\n') {
	            if (state % 2) state++;
	            else state = 0;
	        }
	        else state = 0;
	
	        if (state >= 4)
	            break;
	
	        /* pattern compare */
	        if (memcmp(data + i, pattern, plen ) != 0)
	            continue;
	
	        /* Here, it means patten match!! */
	        *numoff=i + plen;
	        for (j = *numoff, k = 0; data[j] != term; j++, k++)
	            if (j > dlen) return -1 ;   /* no terminal char */
	
	        *numlen = k;
	        return 1;
	}
	return 0;
}
	
static void nk_modifyPkt_for_reset(const struct sk_buff *skb)
{
	struct iphdr *iph = (skb)->nh.iph;
	struct tcphdr *tcph = (void *)iph + iph->ihl*4;
	unsigned int datalen = (skb)->len - (iph->ihl*4) - (tcph->doff*4);
	typedef struct tcphdr tcphdr_t;
	// start to modify tcp header
	tcp_flag_word(tcph) = (TCP_FLAG_RST);
	tcph->doff = (sizeof(tcphdr_t)>>2);
	tcph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, datalen, IPPROTO_TCP,csum_partial((char *)tcph,datalen,0));
	return;
}

static void nk_return_url_block(const struct sk_buff *skb)
{
	struct iphdr *iph = (skb)->nh.iph;
	struct tcphdr *tcph = (void *)iph + iph->ihl*4;
#if 0 /* original src */
	unsigned char *data = (void *)tcph + tcph->doff*4;
#endif
	unsigned int datalen = (skb)->len - (iph->ihl*4) - (tcph->doff*4);
	struct sk_buff *mc=NULL;
	struct rtable *rt;
	struct flowi fl={ .oif = 0,
			.nl_u = { .ip4_u =
					{ .daddr = iph->saddr,
					.saddr = 0,
					.tos = 0 } },
			.proto = IPPROTO_TCP,
			.uli_u = { .ports =
					{ .sport = tcph->dest,
					.dport =tcph->source  } } };
	mc = skb_copy(skb,GFP_ATOMIC);
	mc->input_dev=NULL;
	mc->dev=NULL;
	mc->pkt_type = PACKET_HOST;
	if( __ip_route_output_key(&rt,&fl) )
	{
		kfree_skb(mc);
		SPARQ_LOG("%s: __ip_route_output_key fail",__FUNCTION__);
		return;
	}
	mc->csum=0;
	mc->dst = &rt->u.dst;
	mc->dev = mc->dst->dev;

	// start to modify skb content
	typedef struct tcphdr tcphdr_t;
	static char block_info[500];
	static char block_htm[1000];
	char tmp[100];
	snprintf(block_info, sizeof(block_info),
		"<html>\n<head>\n  <title></title>\n"
		"</head>\n<body bgcolor=\"ffffff\">\n  <h2><h2>\n"
		"  <p>\n  This URLs or Page has been blocked. \n</body>\n</html>\n");
 
	sprintf(block_htm, "HTTP/1.1 200 OK\r\n");
	strcat(block_htm, "Content-type: text/html\r\n");
	strcat(block_htm, "Pragma: no cache\r\n");
	sprintf(tmp, "Content-length: %d\r\n", strlen(block_info));
	strcat(block_htm, tmp);
	strcat(block_htm, "Connection: close\r\n\r\n");
	strcat(block_htm, block_info);

	//header definition
	struct iphdr *new_iph = (mc)->nh.iph;
	struct tcphdr *new_tcph = (void *)new_iph + new_iph->ihl*4;
	unsigned char *new_data = (void *)new_tcph + new_tcph->doff*4;
	/* clear memory */
	memset((char *)new_data,0, datalen);
	/* fill ip header */
	iph->protocol = IPPROTO_TCP;
	new_iph->saddr = iph->daddr;
	new_iph->daddr = iph->saddr;
	new_iph->ttl = 127;
	new_iph->check =0;
	new_iph->check = ip_fast_csum((void *)new_iph, new_iph->ihl);
	/* fill tcp header */
	new_tcph->source = tcph->dest;
	new_tcph->dest = tcph->source;
	tcp_flag_word(new_tcph) = (TCP_FLAG_ACK | TCP_FLAG_PSH);
	if (tcp_flag_word(new_tcph) & TCP_FLAG_ACK)
		new_tcph->seq = tcph->ack_seq;
	else
		new_tcph->seq = htonl(1);
	new_tcph->doff = (sizeof(tcphdr_t)>>2);
	/* the ack should acknowledge all the packet, find the size */
	new_tcph->ack_seq = htonl(ntohl(tcph->seq) + datalen);
	memcpy((char *)new_tcph + sizeof(tcphdr_t), block_htm, strlen(block_htm));
	new_tcph->check = csum_tcpudp_magic(new_iph->saddr, new_iph->daddr, datalen, IPPROTO_TCP,csum_partial((char *)new_tcph,datalen,0));
	
	ip_finish_output(mc);
	nk_modifyPkt_for_reset(skb);

	return;
}

	static int mangle_http_header(const struct sk_buff *skb, int flags)
	{
	    struct iphdr *iph = (skb)->nh.iph;
	    struct tcphdr *tcph = (void *)iph + iph->ihl*4;
	    unsigned char *data = (void *)tcph + tcph->doff*4;
	    unsigned int datalen = (skb)->len - (iph->ihl*4) - (tcph->doff*4);
	
	    int found, offset, len;
	    int ret = 0;

	    /* Basic checking, is it HTTP packet? */
	    if (datalen < 10)
	        return ret;     /* Not enough length, ignore it */
	    if (memcmp(data, "GET ", sizeof("GET ") - 1) != 0 &&
	        memcmp(data, "POST ", sizeof("POST ") - 1) != 0)
	        return ret;     /* Pass it */

	    /* COOKIE modification */
	    if (flags & HTTP_COOKIE) {
	        found = find_pattern2(data, datalen, "Cookie: ",
	                sizeof("Cookie: ")-1, '\r', &offset, &len);

	    	if (found){
#if 0 /* original src */
	            char c;
	            offset -= (sizeof("Cookie: ") - 1);
	            /* Swap the 2rd and 4th bit */
	            c = *(data + offset + 2) ;
	            *(data + offset + 2) = *(data + offset + 4) ;
	            *(data + offset + 4) = c ;
		    ret++;
#else
			nk_return_url_block(skb);
			ret = 0;
#endif
	        }
	    }
	    return ret;
	}
	
static int get_http_info(const struct sk_buff *skb, int flags, httpinfo_t *info)
{
	    struct iphdr *iph = (skb)->nh.iph;
	    struct tcphdr *tcph = (void *)iph + iph->ihl*4;
	    unsigned char *data = (void *)tcph + tcph->doff*4;
	    unsigned int datalen = (skb)->len - (iph->ihl*4) - (tcph->doff*4);
	
	    int found, offset;
	    int hostlen, pathlen;
	    int ret = 0;
	
	    //SPARQ_LOG("%s: seq=%u\n", __FUNCTION__, ntohl(tcph->seq));
	    /* Basic checking, is it HTTP packet? */
	    if (datalen < 10)
	        return ret;     /* Not enough length, ignore it */
	    if (memcmp(data, "GET ", sizeof("GET ") - 1) != 0 &&
	        memcmp(data, "POST ", sizeof("POST ") - 1) != 0)
        return ret;     /* Pass it */
	
	    if (!(flags & (HTTP_HOST | HTTP_URL)))
	        return ret;
	
	    /* find the 'Host: ' value */
	    found = find_pattern2(data, datalen, "Host: ",
	            sizeof("Host: ") - 1, '\r', &offset, &hostlen);
	    SPARQ_LOG("Host found=%d\n", found);
	
	    if (!found || !hostlen)
	        return ret;
	
	    ret++;      /* Host found, increase the return value */
	    hostlen = (hostlen < BUFSIZE) ? hostlen : BUFSIZE;
	    strncpy(info->host, data + offset, hostlen);
	    *(info->host + hostlen) = 0;                /* null-terminated */
	    info->hostlen = hostlen;
	    SPARQ_LOG("HOST=%s, hostlen=%d\n", info->host, info->hostlen);
	
	    if (!(flags & HTTP_URL))
	        return ret;
	
	    /* find the 'GET ' or 'POST ' value */
	    found = find_pattern2(data, datalen, "GET ",
	            sizeof("GET ") - 1, '\r', &offset, &pathlen);
	    if (!found)
	        found = find_pattern2(data, datalen, "POST ",
	                sizeof("POST ") - 1, '\r', &offset, &pathlen);
	    SPARQ_LOG("GET/POST found=%d\n", found);
	
	    if (!found || (pathlen -= (sizeof(" HTTP/x.x") - 1)) <= 0)/* ignor this field */
	        return ret;
	
	    ret++;      /* GET/POST found, increase the return value */
	    pathlen = ((pathlen + hostlen) < BUFSIZE) ? pathlen : BUFSIZE - hostlen;
	    strncpy(info->url, info->host, hostlen);
	    strncpy(info->url + hostlen, data + offset, pathlen);
	    *(info->url + hostlen + pathlen) = 0;       /* null-terminated */
	    info->urllen = hostlen + pathlen;
	    SPARQ_LOG("URL=%s, urllen=%d\n", info->url, info->urllen);
	
	    return ret;
}
	
/* Linear string search based on memcmp() */
static char *search_linear (char *needle, char *haystack, int needle_len, int haystack_len)
{
	        char *k = haystack + (haystack_len-needle_len);
	        char *t = haystack;
	       
	        SPARQ_LOG("%s: haystack=%s, needle=%s\n", __FUNCTION__, t, needle);
	        for(; t <= k; t++) {
	                //SPARQ_LOG("%s: haystack=%s, needle=%s\n", __FUNCTION__, t, needle);
	                if (strnicmp(t, needle, needle_len) == 0) return t;
	                //if ( memcmp(t, needle, needle_len) == 0 ) return t;
        }

        return NULL;
}
//-----------------------------webstr

static int match(const struct sk_buff *skb,
		 const struct net_device *in,
		 const struct net_device *out,
		 const void *matchinfo,
		 int offset,
		 unsigned int protoff,
		 int *hotdrop)
{
	struct ts_state state;
	struct iphdr *ip = skb->nh.iph;
	struct xt_string_info *conf = (struct xt_string_info *) matchinfo;
	int flags = 0;
	httpinfo_t htinfo;
	long int opt = 0;
	char *wordlist = (char *)&conf->pattern;
	int found = 0;
	char token[] = "<&nbsp;>";
	proc_ipt_search search=search_linear;

	if (!ip || conf->patlen < 1)
	    return 0;

	//printk("type=%d\n", conf->type);
	switch(conf->type)
	{
		case IPT_WEBSTR_URL:        /* fall through */
	    	flags |= HTTP_URL;
			break;

	    case IPT_WEBSTR_HOST:
	        flags |= HTTP_HOST;
	        break;
	
	    case IPT_WEBSTR_CONTENT:
	        opt = simple_strtol(wordlist, (char **)NULL, 10);
	        //SPARQ_LOG("%s: string=%s, opt=%#lx\n", __FUNCTION__, wordlist, opt);
	
	        if (opt & (BLK_JAVA | BLK_ACTIVE | BLK_PROXY))
	    	    flags |= HTTP_URL;

	        if (opt & BLK_PROXY)
	            flags |= HTTP_HOST;

	        if (opt & BLK_COOKIE)
			{
            	if((found=mangle_http_header(skb, HTTP_COOKIE))==1)
					goto match_ret;
				else
					return 0;
			}
	        break;
	
	    default:
	        printk("%s: Sorry! Cannot find this match option.\n", __FILE__);
	        return 0;
	}

	/* Get the http header info */
    if (get_http_info(skb, flags, &htinfo) < 1)
            return 0;

	/* Check if the http header content contains the forbidden keyword */
	if (conf->type == IPT_WEBSTR_HOST || conf->type == IPT_WEBSTR_URL)
	{
        	int nlen = 0, hlen = 0;
		char needle[BUFSIZE], *haystack = NULL;
        	char *next;

		if (conf->type == IPT_WEBSTR_HOST)
		{
			haystack = htinfo.host;
			hlen = htinfo.hostlen;
		}
        	else
		{
			haystack = htinfo.url;
			hlen = htinfo.urllen;
		}
		split(needle, wordlist, next, token)
		{
			nlen = strlen(needle);
			SPARQ_LOG("keyword=%s, nlen=%d, hlen=%d\n", needle, nlen, hlen);
			if (!nlen || !hlen || nlen > hlen) continue;
			if (search(needle, haystack, nlen, hlen) != NULL) 
			{
				found = 1;
				/*purpose:0013264, author:selena*/
				if(nk_denypolicy)
				{
					struct tcphdr *tcp = (void *)ip + ip->ihl*4;
					if(conf->type == IPT_WEBSTR_HOST)
						printk(KERN_ALERT "#warn<4> Forbidden Domain:TCP " NIPQUAD_FMT ":%d->" NIPQUAD_FMT ":%d on %s url= %s\n",NIPQUAD(ip->saddr),tcp->source,NIPQUAD(ip->daddr),tcp->dest,skb->input_dev->name,needle);
					else
						printk(KERN_ALERT "#warn<4> Website Blocking by Keywords:TCP " NIPQUAD_FMT ":%d->" NIPQUAD_FMT ":%d on %s keyword= %s\n",NIPQUAD(ip->saddr),tcp->source,NIPQUAD(ip->daddr),tcp->dest,skb->input_dev->name,needle);
				}
				break;
			}
		}
	}
	else 
	{		/* IPT_WEBSTR_CONTENT */
		int vicelen;
		if (opt & BLK_JAVA) 
		{
			vicelen = sizeof(".java") - 1;
			if (strnicmp(htinfo.url + htinfo.urllen - vicelen, ".java", vicelen) == 0) {
		    		SPARQ_LOG("%s: MATCH....java\n", __FUNCTION__);
		    		found = 1;
				nk_return_url_block(skb);
		    		goto match_ret;
			}
			vicelen = sizeof(".jar") - 1;
			if (strnicmp(htinfo.url + htinfo.urllen - vicelen, ".jar", vicelen) == 0) {
		    		SPARQ_LOG("%s: MATCH....java\n", __FUNCTION__);
		    		found = 1;
				nk_return_url_block(skb);
		    		goto match_ret;
			}
			vicelen = sizeof(".class") - 1;
			if (strnicmp(htinfo.url + htinfo.urllen - vicelen, ".class", vicelen) == 0) {
		    		SPARQ_LOG("%s: MATCH....java\n", __FUNCTION__);
		    		found = 1;
				nk_return_url_block(skb);
		    		goto match_ret;
			}
			vicelen = sizeof(".js") - 1;
			if (strnicmp(htinfo.url + htinfo.urllen - vicelen, ".js", vicelen) == 0) {
		    		SPARQ_LOG("%s: MATCH....java\n", __FUNCTION__);
		    		found = 1;
				nk_return_url_block(skb);
		    		goto match_ret;
			}
		}
		if (opt & BLK_ACTIVE)
		{
			vicelen = sizeof(".ocx") - 1;
			if (strnicmp(htinfo.url + htinfo.urllen - vicelen, ".ocx", vicelen) == 0) {
		    		SPARQ_LOG("%s: MATCH....activex\n", __FUNCTION__);
		    		found = 1;
				nk_return_url_block(skb);
		    		goto match_ret;
			}
			vicelen = sizeof(".cab") - 1;
			if (strnicmp(htinfo.url + htinfo.urllen - vicelen, ".cab", vicelen) == 0) {
		    		SPARQ_LOG("%s: MATCH....activex\n", __FUNCTION__);
		    		found = 1;
				nk_return_url_block(skb);
		    		goto match_ret;
			}
		}
		if (opt & BLK_PROXY)
		{
			if (strnicmp(htinfo.url + htinfo.hostlen, "http://", sizeof("http://") - 1) == 0) {
		    		SPARQ_LOG("%s: MATCH....proxy\n", __FUNCTION__);
		    		found = 0;
				nk_return_url_block(skb);
		    		goto match_ret;
			}
	    }
	}
#if 0
match_ret:
	memset(&state, 0, sizeof(struct ts_state));
        SPARQ_LOG("%s: Verdict =======> %s \n",__FUNCTION__
                , found ? "DROP" : "ACCEPT");
	return (skb_find_text((struct sk_buff *)skb, conf->from_offset, 
			     conf->to_offset, conf->config, &state) 
			     != UINT_MAX) && !conf->invert;
#endif
#if 1
match_ret:
	memset(&state, 0, sizeof(struct ts_state));
        SPARQ_LOG("%s: Verdict =======> %s \n",__FUNCTION__
                , found ? "DROP" : "ACCEPT");

        return (found ^ conf->invert);
#endif
}

#define STRING_TEXT_PRIV(m) ((struct xt_string_info *) m)

static int checkentry(const char *tablename,
		      const void *ip,
		      void *matchinfo,
		      unsigned int matchsize,
		      unsigned int hook_mask)
{
	struct xt_string_info *conf = matchinfo;
	struct ts_config *ts_conf;

	if (matchsize != XT_ALIGN(sizeof(struct xt_string_info)))
		return 0;

	/* Damn, can't handle this case properly with iptables... */
	if (conf->from_offset > conf->to_offset)
		return 0;

	ts_conf = textsearch_prepare(conf->algo, conf->pattern, conf->patlen,
				     GFP_KERNEL, TS_AUTOLOAD);
	if (IS_ERR(ts_conf))
		return 0;

	conf->config = ts_conf;

	return 1;
}

static void destroy(void *matchinfo, unsigned int matchsize)
{
	textsearch_destroy(STRING_TEXT_PRIV(matchinfo)->config);
}

static struct xt_match string_match = {
	.name 		= "string",
	.match 		= match,
	.checkentry	= checkentry,
	.destroy 	= destroy,
	.me 		= THIS_MODULE
};
static struct xt_match string6_match = {
	.name 		= "string",
	.match 		= match,
	.checkentry	= checkentry,
	.destroy 	= destroy,
	.me 		= THIS_MODULE
};

static int __init init(void)
{
	int ret;

	ret = xt_register_match(AF_INET, &string_match);
	if (ret)
		return ret;
	ret = xt_register_match(AF_INET6, &string6_match);
	if (ret)
		xt_unregister_match(AF_INET, &string_match);

	return ret;
}

static void __exit fini(void)
{
	xt_unregister_match(AF_INET, &string_match);
	xt_unregister_match(AF_INET6, &string6_match);
}

module_init(init);
module_exit(fini);
