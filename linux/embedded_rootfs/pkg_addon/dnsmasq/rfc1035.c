/* dnsmasq is Copyright (c) 2000 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include "dnsmasq.h"
#include <nkutil.h>

#ifdef CONFIG_NK_LOCAL_DNS_DATABASE
#define NK_DEFAULT_TTL 3600 /* 1 hour */
struct LOCAL_DNS_DB {
	char hostname[32];
	struct in_addr ip;
	struct LOCAL_DNS_DB *prev;
	struct LOCAL_DNS_DB *next;
};
struct LOCAL_DNS_DB *local_dns_db_list = NULL;
#ifdef NK_CONFIG_IPV6

struct LOCAL_DNS_DB_IPV6 {
	char hostname[32];
	struct in6_addr ipv6_address;
	struct LOCAL_DNS_DB_IPV6 *prev;
	struct LOCAL_DNS_DB_IPV6 *next;
};
struct LOCAL_DNS_DB_IPV6 *local_dns_db_ipv6_list = NULL;

#endif
#endif

static int extract_name(HEADER *header, unsigned int plen, unsigned char **pp, 
			char *name, int isExtract)
{
  char *cp = name;
  unsigned char *p = *pp, *p1 = NULL;
  unsigned int j, l, hops = 0;
  int retvalue = 1;
  
  while ((l = *p++))
    {
      if ((l & 0xc0) == 0xc0) /* pointer */
	{ 
	  if (p - (unsigned char *)header + 1u >= plen)
	    return 0;
	      
	  /* get offset */
	  l = (l&0x3f) << 8;
	  l |= *p++;
	  if (l >= (unsigned int)plen) 
	    return 0;
	  
	  if (!p1) /* first jump, save location to go back to */
	    p1 = p;
	      
	  hops++; /* break malicious infinite loops */
	  if (hops > 255)
	    return 0;
	  
	  p = l + (unsigned char *)header;
	}
      else
	{
	  if (cp-name+l+1 >= MAXDNAME)
	    return 0;
	  if (p - (unsigned char *)header + l >= plen)
	    return 0;
	  for(j=0; j<l; j++, p++)
	    if (isExtract)
	      *cp++ = tolower(*p);
	    else if (!*cp || *cp++ != tolower(*p))
	      retvalue =  2;
	     
	  if (isExtract)
	    *cp++ = '.';
	  else
	    if (*cp && *cp++ != '.')
	      retvalue = 2;
	}
      
      if ((unsigned int)(p - (unsigned char *)header) >= plen)
	return 0;
    }

  if (isExtract)
  {
  	if (cp == name)	/* bad packet! */
		return 0;
    *--cp = 0; /* terminate: lose final period */
  }
  
  if (p1) /* we jumped via compression */
    *pp = p1;
  else
    *pp = p;

  return retvalue;
}

static int in_arpa_name_2_addr(char *namein, struct all_addr *addrp)
{
  /* Max size of input string (for IPv6) is 40 chars.) */
  int j;
  char name[45], *cp1;
  unsigned char *addr = (unsigned char *)addrp;
  char *lastchunk = NULL, *penchunk = NULL;
  
  if (strlen(namein) > 44)
    return 0; /* too long */

  memset(addrp, 0, sizeof(struct all_addr));

  /* turn name into a series of asciiz strings */
  /* j counts no of labels */
  for(j = 1,cp1 = name; *namein; cp1++, namein++)
    if (*namein == '.')
      {
	penchunk = lastchunk;
        lastchunk = cp1 + 1;
	*cp1 = 0;
	j++;
      }
    else
      *cp1 = *namein;
  
  *cp1 = 0;

  if (j<3)
    return 0;

  if (strcmp(lastchunk, "arpa") == 0 && strcmp(penchunk, "in-addr") == 0)
    {
      /* IP v4 */
      for (cp1 = name; cp1 != penchunk; cp1 += strlen(cp1)+1)
	{
	  /* check for digits only (weeds out things like
	     50.0/24.67.28.64.in-addr.arpa which are used 
	     as CNAME targets according to RFC 2317 */
	  char *cp;
	  for (cp = cp1; *cp; cp++)
	    if (!isdigit((int)*cp))
	      return 0;
	  
	  addr[3] = addr[2];
	  addr[2] = addr[1];
	  addr[1] = addr[0];
	  addr[0] = atoi(cp1);
	}

      return F_IPV4;
    }
#ifdef HAVE_IPV6
  else if ((strcmp(lastchunk, "arpa") == 0 || strcmp(lastchunk, "int") == 0) &&
	   strcmp(penchunk, "ip6") == 0)
    {
      /* IP v6 */
      /* address arrives as 0.1.2.3.4.5.6.7.8.9.a.b.c.d.e.f.ip6.int
         or 0.1.2.3.4.5.6.7.8.9.a.b.c.d.e.f.ip6.arpa
      */
      for (cp1 = name; cp1 != penchunk; cp1 += strlen(cp1)+1)
	{
	  if (*(cp1+1) || !isxdigit((int)*cp1))
	    return 0;
	  
	  for (j = sizeof(struct all_addr)-1; j>0; j--)
	    addr[j] = (addr[j] >> 4) | (addr[j-1] << 4);
	  addr[0] = (addr[0] >> 4) | (strtol(cp1, NULL, 16) << 4);
	}

      return F_IPV6;
    }
#endif
  
  return 0;
}

static unsigned char *skip_questions(HEADER *header, unsigned int plen)
{
  int q, qdcount = ntohs(header->qdcount);
  unsigned char *ansp = (unsigned char *)(header+1);

  for (q=0; q<qdcount; q++)
    {
      while (1)
	{
          if ((unsigned int)(ansp - (unsigned char *)header) >= plen)
	    return NULL;
	  if (((*ansp) & 0xc0) == 0xc0) /* pointer for name compression */
	    {
              ansp += 2;	
	      break;
	    }
	  else if (*ansp) 
	    { /* another segment */
	      ansp += (*ansp) + 1;
	    }
	  else            /* end */
	    {
	      ansp++;
	      break;
	    }
	}
      ansp += 4; /* class and type */
    }
  if ((unsigned int)(ansp - (unsigned char *)header) > plen) 
     return NULL;
  
  return ansp;
}

/* is addr in the non-globally-routed IP space? */ 
static int private_net(struct all_addr *addrp) 
{
  struct in_addr addr = *(struct in_addr *)addrp;
  if (inet_netof(addr) == 0xA ||
      (inet_netof(addr) >= 0xAC10 && inet_netof(addr) < 0xAC20) ||
      (inet_netof(addr) >> 8) == 0xC0A8) 
    return 1;
  else 
    return 0;
}
 
static unsigned char *add_text_record(unsigned int nameoffset, unsigned char *p, 
				      unsigned long ttl, unsigned short pref, 
				      unsigned short type, char *name)
{
  unsigned char *sav, *cp;
  int j;
  
  PUTSHORT(nameoffset | 0xc000, p); 
  PUTSHORT(type, p);
  PUTSHORT(C_IN, p);
  PUTLONG(ttl, p); /* TTL */
  
  sav = p;
  PUTSHORT(0, p); /* dummy RDLENGTH */

  if (pref)
    PUTSHORT(pref, p);

  while (*name) 
    {
      cp = p++;
      for (j=0; *name && (*name != '.'); name++, j++)
	*p++ = *name;
      *cp = j;
      if (*name)
	name++;
    }
  *p++ = 0;
  j = p - sav - 2;
  PUTSHORT(j, sav); /* Real RDLENGTH */
  
  return p;
}

/* On receiving an NXDOMAIN or NODATA reply, determine which names are known
   not to exist for negative caching. name if a working buffer passed in. */
static void extract_neg_addrs(HEADER *header, unsigned int qlen, char *name) 
{
  unsigned char *p;
  int i, cache_fail, found_soa = 0;
  int qtype, qclass, rdlen;
  unsigned long ttl, minttl = 0;
  time_t now = time(NULL);
  
  /* there may be more than one question with some questions
     answered. We don't generate negative entries from those. */
  if (ntohs(header->ancount) != 0)
    return;
  
  if (!(p = skip_questions(header, qlen)))
    return; /* bad packet */
  
  /* we first need to find SOA records, to get min TTL, then we
     add a NEG cache entry for each question. */

  for (i=0; i<ntohs(header->nscount); i++)
    {
      if (!extract_name(header, qlen, &p, name, 1))
	return; /* bad packet */

      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);
      GETLONG(ttl, p);
      GETSHORT(rdlen, p);
       
      if ((qclass == C_IN) && (qtype == T_SOA))
	{
	  int dummy;
	  /* MNAME */
	  if (!extract_name(header, qlen, &p, name, 1))
	    return;
	  /* RNAME */
	  if (!extract_name(header, qlen, &p, name, 1))
	    return;
	  GETLONG(dummy, p); /* SERIAL */
	  GETLONG(dummy, p); /* REFRESH */
	  GETLONG(dummy, p); /* RETRY */
	  GETLONG(dummy, p); /* EXPIRE */
	  if (!found_soa)
	    {
	      found_soa = 1;
	      minttl = ttl;
	    }
	  else if (ttl < minttl)
	    minttl = ttl;
	  GETLONG(ttl, p); /* minTTL */
	  if (ttl < minttl)
	    minttl = ttl;
	}
      else
	p += rdlen;

      if ((unsigned int)(p - (unsigned char *)header) > qlen)
	return; /* bad packet */
    }
  
  if (!found_soa)
    return; /* failed to find SOA */
  
  p = (unsigned char *)(header+1);
  
  cache_start_insert();
  cache_fail = 0;

  for (i=0; i<ntohs(header->qdcount); i++)
    {
      if (!extract_name(header, qlen, &p, name, 1))
	return; /* bad packet */
      
      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);
      
      if (qclass == C_IN && qtype == T_A) 
	cache_insert(name, NULL, now, minttl, F_IPV4 | F_FORWARD, &cache_fail);
#ifdef HAVE_IPV6	      
      if (qclass == C_IN && qtype == T_AAAA) 
	cache_insert(name, NULL, now, minttl, F_IPV6 | F_FORWARD, &cache_fail);
#endif
    }
  
  cache_end_insert(cache_fail);
}

void extract_addresses(HEADER *header, unsigned int qlen, char *name)
{
  unsigned char *p, *psave, *endrr;
  int qtype, qclass, rdlen;
  unsigned long ttl;
  int i, cache_fail;
  time_t now = time(NULL);

  /* detect errors or NODATA packets and cache negative answer */
  if (header->rcode == NXDOMAIN || ntohs(header->ancount) == 0)
    extract_neg_addrs(header, qlen, name);
  
   /* skip over questions */
  if (!(p = skip_questions(header, qlen)))
    return; /* bad packet */

  /* Strategy: mark all entries as old and suitable for replacement.
     new entries made here are not removed while processing this packet.
     Makes multiple addresses per name work. */
  
  cache_start_insert();
  cache_fail = 0;

  psave = p;
  
  for (i=0; i<ntohs(header->ancount); i++)
    {
      unsigned char *origname = p;
      if (!extract_name(header, qlen, &p, name, 1))
	return; /* bad packet */

      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);
      GETLONG(ttl, p);
      GETSHORT(rdlen, p);
	
      endrr = p + rdlen;
      if ((unsigned int)(endrr - (unsigned char *)header) > qlen)
	return; /* bad packet */
      
      if (qclass != C_IN)
	{
	  p = endrr;
	  continue;
	}

      if (qtype == T_A){ /* A record. */
	cache_insert(name, (struct all_addr *)p, now, 
		     ttl, F_IPV4 | F_FORWARD, &cache_fail);
      }
#ifdef HAVE_IPV6
      else if (qtype == T_AAAA) /* IPV6 address record. */
	cache_insert(name, (struct all_addr *)p, now,
		     ttl, F_IPV6 | F_FORWARD, &cache_fail);
#endif
      else if (qtype == T_PTR)
	{
	  /* PTR record */
	  struct all_addr addr;
	  int name_encoding = in_arpa_name_2_addr(name, &addr);
	  if (name_encoding)
	    {
	      if (!extract_name(header, qlen, &p, name, 1))
		return; /* bad packet */
	      cache_insert(name, &addr, now, 
			   ttl, name_encoding | F_REVERSE, &cache_fail); 
	    }
	}
      else if (qtype == T_CNAME)
	{
	  /* CNAME, search whole answer section again */
	  unsigned char *endrr1;
	  unsigned long cttl;
	  int j;
	  unsigned char *targp = p;
	  	  
	  p = psave; /* rewind p */
	  for (j=0; j<ntohs(header->ancount); j++)
	    {
	      int res;
	      unsigned char *tmp = targp; 
	      /* copy since it gets altered by extract_name */
	      /* get CNAME target each time round */
	      if (!extract_name(header, qlen, &tmp, name, 1))
		return; /* bad packet */
	      /* compare this name with target of CNAME in name buffer */
	      if (!(res = extract_name(header, qlen, &p, name, 0)))
		return; /* bad packet */
	      
	      GETSHORT(qtype, p); 
	      GETSHORT(qclass, p);
	      GETLONG(cttl, p);
	      GETSHORT(rdlen, p);
	      
	      endrr1 = p+rdlen;
	      if ((unsigned int)(endrr1 - (unsigned char *)header) > qlen)
		return; /* bad packet */

	      /* is this RR name same as target of CNAME */
	      if ((qclass != C_IN) || (res == 2))
		{
		  p = endrr1;
		  continue;
		}

	      /* match, use name of CNAME, data from this RR
		 use min TTL of two */

	      if (ttl < cttl)
		cttl = ttl;

	      /* get orig. name back again */
	      tmp = origname;
	      if (!extract_name(header, qlen, &tmp, name, 1))
		return;

	      if (qtype == T_A) /* A record. */
		cache_insert(name, (struct all_addr *)p, now, 
			     cttl, F_IPV4 | F_FORWARD, &cache_fail);
#ifdef HAVE_IPV6
	      else if (qtype == T_AAAA) /* IPV6 address record. */
		cache_insert(name, (struct all_addr *)p, now, 
			     cttl, F_IPV6 | F_FORWARD, &cache_fail);
#endif
	      else if (qtype == T_PTR)
		{
		  /* PTR record extract address from CNAME name */
		  struct all_addr addr;
		  int name_encoding = in_arpa_name_2_addr(name, &addr);
		  if (name_encoding)
		    {
		      if (!extract_name(header, qlen, &p, name, 1))
			return; /* bad packet */
		      cache_insert(name, &addr, now, cttl, 
				   name_encoding | F_REVERSE, &cache_fail);
		    } 
		}
	      p = endrr1;
	    }
	} 
      p = endrr;
    }

  cache_end_insert(cache_fail);
}

/* If the packet holds extactly one query
   return 1 and leave the name from the query in name. */

int extract_request(HEADER *header,unsigned int qlen, char *name)
{
  unsigned char *p = (unsigned char *)(header+1);
  
  if (ntohs(header->qdcount) != 1 || header->opcode != QUERY)
    return 0; /* must be exactly one query. */
  
  if (!extract_name(header, qlen, &p, name, 1))
    return 0; /* bad packet */
  
  return 1;
}
#ifdef CONFIG_NK_LOCAL_DNS_DATABASE
static char *strncpyz(char *dest, char const *src, size_t size)
{
    if (!size--)
	return dest;
    strncpy(dest, src, size);
    dest[size] = 0; /* Make sure the string is null terminated */
    return dest;
}


/*purpose     : 0012882 author : michael lu date : 2010-07-23*/
/*description : support more special character              */
static int find_special_word(char *offset, int len, int i)
{
f_again:
    if (((offset[i+len+1] == '\"')&&(offset[i+len+2] == '&'))||((offset[i+len+1] == '\"')&&(offset[i+len+2] == '\"')))
    {
	i++;
	for (i; (((offset[i+len+1] == '\"')&&(offset[i+len+2] == '&'))||((offset[i+len+1] == '\"')&&(offset[i+len+2] == '\"'))); i++);
	goto f_again;
    }
    return i;
}
/*purpose     : 0012882 author : michael lu date : 2010-07-21*/
/*description : support more special character           */
static char* name_get_value(char *string, char *varname, char *retval, int buf_len, char *offset)
{
	int i=0, len=strlen(varname), break_loop=0, ValueEmpty=0;
	char searchName[257];
	char checkValueEnd[257];
	
	strcpy(searchName, varname);
	strcpy(checkValueEnd, varname);
	strcat(checkValueEnd, "=\"\"");
	strcat(searchName,"=\"");
	len+=2;
	
	if (!string)
	{
		retval[0] = '\0';
		return NULL;
	}
	offset = offset ? offset : string;
	
	char *p=NULL,*q=NULL;
	if(p=strstr(offset, checkValueEnd))
	{
		q=strstr( offset , searchName );
		if(p == q)
		ValueEmpty=1;
	}
	if (((offset=strstr(offset, searchName)) != 0) && (ValueEmpty==0))
	{
		for (i=0; ((offset+i+len)<(string+strlen(string)) && (break_loop==0)); i++)
		{
			if(((offset[i+len+1] == '\"')&&(offset[i+len+2] == '&'))||((offset[i+len+1] == '\"')&&(offset[i+len+2] == '\"')))
			{
				break_loop=1;
			}
		}
		
		i = find_special_word(offset, len, i);
		if (buf_len >= (i+1))
		{
			strncpyz(retval, (offset+len), i+1);
		}
		return (offset+len+2+(i+1));
	}
	retval[0] = '\0';
	return NULL;
}

void free_local_dns_db(struct LOCAL_DNS_DB *free_entry)
{
	if(free_entry)
	{
		free_local_dns_db(free_entry->prev);
		free_local_dns_db(free_entry->next);
		safe_free(free_entry);
	}
}

#ifdef NK_CONFIG_IPV6
struct LOCAL_DNS_DB_IPV6 * add_local_dns_db_ipv6(struct LOCAL_DNS_DB_IPV6 *add_entry, char *hostname, char *ip)
{
	int ans = 0;
	struct LOCAL_DNS_DB_IPV6 *new_entry = NULL;

	if(add_entry)
	{
		ans = strcmp(hostname, add_entry->hostname);
		if(ans > 0)
		{
			new_entry = add_local_dns_db_ipv6(add_entry->next, hostname, ip);
			if(new_entry)
			{
				add_entry->next = new_entry;
				new_entry = NULL;
			}
		}
		else if(ans < 0)
		{
			new_entry = add_local_dns_db_ipv6(add_entry->prev, hostname, ip);
			if(new_entry)
			{
				add_entry->prev = new_entry;
				new_entry = NULL;
			}
		}
	}
	else
	{
		new_entry = (struct LOCAL_DNS_DB_IPV6 *)safe_malloc(sizeof(struct LOCAL_DNS_DB_IPV6));
		inet_pton (AF_INET6, ip, &new_entry->ipv6_address);
		strcpy(new_entry->hostname, hostname);
		new_entry->prev = NULL;
		new_entry->next = NULL;
	}

	return new_entry;
}

struct LOCAL_DNS_DB_IPV6 * search_local_dns_db_ipv6(struct LOCAL_DNS_DB_IPV6 *search_entry, char *hostname)
{
	int ans = 0;
	struct LOCAL_DNS_DB_IPV6 *ans_entry = NULL;

	if(search_entry)
	{
		ans = strcmp(hostname, search_entry->hostname);
		if(ans > 0)
		{
			ans_entry = search_local_dns_db_ipv6(search_entry->next, hostname);
		}
		else if(ans < 0)
		{
			ans_entry = search_local_dns_db_ipv6(search_entry->prev, hostname);
		}
		else
		{
			ans_entry = search_entry;
		}
	}

	return ans_entry;
}

void free_local_dns_db_ipv6(struct LOCAL_DNS_DB_IPV6 *free_entry) //oolong
{
	if(free_entry)
	{
		free_local_dns_db_ipv6(free_entry->prev);
		free_local_dns_db_ipv6(free_entry->next);
		safe_free(free_entry);
	}
}

#endif
struct LOCAL_DNS_DB * add_local_dns_db(struct LOCAL_DNS_DB *add_entry, char *hostname, char *ip)
{
	int ans = 0;
	struct LOCAL_DNS_DB *new_entry = NULL;

	if(add_entry)
	{
		ans = strcmp(hostname, add_entry->hostname);
		if(ans > 0)
		{
			new_entry = add_local_dns_db(add_entry->next, hostname, ip);
			if(new_entry)
			{
				add_entry->next = new_entry;
				new_entry = NULL;
			}
		}
		else if(ans < 0)
		{
			new_entry = add_local_dns_db(add_entry->prev, hostname, ip);
			if(new_entry)
			{
				add_entry->prev = new_entry;
				new_entry = NULL;
			}
		}
	}
	else
	{
		new_entry = (struct LOCAL_DNS_DB *)safe_malloc(sizeof(struct LOCAL_DNS_DB));
		strcpy(new_entry->hostname, hostname);
		new_entry->ip.s_addr = inet_addr(ip);
		new_entry->prev = NULL;
		new_entry->next = NULL;
	}

	return new_entry;
}

struct LOCAL_DNS_DB * search_local_dns_db(struct LOCAL_DNS_DB *search_entry, char *hostname)
{
	int ans = 0;
	struct LOCAL_DNS_DB *ans_entry = NULL;

	if(search_entry)
	{
		ans = strcmp(hostname, search_entry->hostname);
		if(ans > 0)
		{
			ans_entry = search_local_dns_db(search_entry->next, hostname);
		}
		else if(ans < 0)
		{
			ans_entry = search_local_dns_db(search_entry->prev, hostname);
		}
		else
		{
			ans_entry = search_entry;
		}
	}

	return ans_entry;
}

void load_local_dns_db(void)
{
	char buf[100], tmp[100], HOSTNAME[32], IP[20];
	int i, num = 0;
	
	#ifdef NK_CONFIG_IPV6
	char  IPV6[INET6_ADDRSTRLEN],tmp2[INET6_ADDRSTRLEN];
	#endif

	free_local_dns_db(local_dns_db_list);
	local_dns_db_list = NULL;

	kd_doCommand("DNS_LOCAL_DATABASE NUMBER", CMD_PRINT, ASH_DO_NOTHING, buf);
	sscanf(buf, "%d", &num);

	for (i=0; i<num; i++)
	{
		sprintf(tmp,"DNS_LOCAL_DATABASE ID %d",(i+1));
		kd_doCommand(tmp, CMD_PRINT, ASH_DO_NOTHING, buf);
		bzero( HOSTNAME, sizeof(HOSTNAME) );
		bzero( IP, sizeof(IP) );
		name_get_value(buf, "HOSTNAME", HOSTNAME, sizeof(HOSTNAME), NULL);
		name_get_value(buf, "IP", IP, sizeof(IP), NULL);
		if(local_dns_db_list)
		{
			add_local_dns_db(local_dns_db_list, HOSTNAME, IP);
		}
		else
		{
			local_dns_db_list = (struct LOCAL_DNS_DB *)safe_malloc(sizeof(struct LOCAL_DNS_DB));
			strcpy(local_dns_db_list->hostname, HOSTNAME);
			local_dns_db_list->ip.s_addr = inet_addr(IP);
			local_dns_db_list->prev = NULL;
			local_dns_db_list->next = NULL;
		}
	}
	
#ifdef NK_CONFIG_IPV6
	
	free_local_dns_db_ipv6(local_dns_db_ipv6_list); //oolong
	local_dns_db_ipv6_list = NULL;

	kd_doCommand("DNS_LOCAL_DATABASE_V6 NUMBER", CMD_PRINT, ASH_DO_NOTHING, buf);
	sscanf(buf, "%d", &num);

	for (i=0; i<num; i++)
	{
		sprintf(tmp,"DNS_LOCAL_DATABASE_V6 ID %d",(i+1));
		kd_doCommand(tmp, CMD_PRINT, ASH_DO_NOTHING, buf);
		bzero( HOSTNAME, sizeof(HOSTNAME) );
		bzero( IPV6, sizeof(IPV6) );
		name_get_value(buf, "HOSTNAME", HOSTNAME, sizeof(HOSTNAME), NULL);
		name_get_value(buf, "IP", IPV6, sizeof(IPV6), NULL);
		if(local_dns_db_ipv6_list)
		{
			add_local_dns_db_ipv6(local_dns_db_ipv6_list, HOSTNAME, IPV6);
		}
		else
		{
			local_dns_db_ipv6_list = (struct LOCAL_DNS_DB_IPV6 *)safe_malloc(sizeof(struct LOCAL_DNS_DB_IPV6));
			strcpy(local_dns_db_ipv6_list->hostname, HOSTNAME);
			inet_pton (AF_INET6, &IPV6, &local_dns_db_ipv6_list->ipv6_address);
			local_dns_db_ipv6_list->prev = NULL;
			local_dns_db_ipv6_list->next = NULL;
		}
	}
	
#endif
}

/* query_local_answer() performs address search on DNS static local database.
 * A matching entry is one whose name matches the query or query concatenated
 * with local DNS static local database entry using case insensitive comparison.
 * Returns 0 on error, number found otherwise.
 */
static int query_local_answer(unsigned int nameoffset,unsigned char **pp,char *domain,int qtype)
{
	unsigned char *ansp = *pp;
	int cnt=0, i;
	struct LOCAL_DNS_DB *ans_entry = NULL;
#ifdef NK_CONFIG_IPV6
	struct LOCAL_DNS_DB_IPV6 *ans_entry_ipv6 = NULL;
#endif

	if(!ansp||!domain)
	{
		return 0;
	}
	
#ifdef NK_CONFIG_IPV6
	if(qtype == T_AAAA)
	{
		ans_entry_ipv6 = search_local_dns_db_ipv6(local_dns_db_ipv6_list, domain);
		if(ans_entry_ipv6)
		{
			PUTSHORT(nameoffset | 0xc000, ansp);
			PUTSHORT(T_AAAA, ansp);
			PUTSHORT(C_IN, ansp);
			PUTLONG(NK_DEFAULT_TTL, ansp);
			PUTSHORT(IN6ADDRSZ, ansp);
			memcpy(ansp, &(ans_entry_ipv6->ipv6_address.s6_addr), IN6ADDRSZ);
			ansp += IN6ADDRSZ;
			*pp = ansp;
			cnt++;
		}
	}
	else if(qtype == T_A)
	{
#endif
		ans_entry = search_local_dns_db(local_dns_db_list, domain);
		if(ans_entry)
		{
			PUTSHORT(nameoffset | 0xc000, ansp);
			PUTSHORT(T_A, ansp);
			PUTSHORT(C_IN, ansp);
			PUTLONG(NK_DEFAULT_TTL, ansp);

			PUTSHORT(4, ansp);
			memcpy(ansp, &(ans_entry->ip), 4);
			ansp += 4;
			*pp = ansp;
			cnt++;
		}
#ifdef NK_CONFIG_IPV6
	}
#endif
	return cnt;
}
#endif
/* return zero if we can't answer from cache, or packet size if we can */
int answer_request(HEADER *header, char *limit, unsigned int qlen, char *mxname, 
		   char *mxtarget, unsigned int options, char *name)
{
  unsigned char *p, *ansp;
  int qtype, qclass, is_arpa;
  struct all_addr addr;
  unsigned int nameoffset;
  int q, qdcount = ntohs(header->qdcount); 
  int ans, anscount = 0;
  time_t now = time(NULL);
  struct crec *crecp;
#ifdef CONFIG_NK_LOCAL_DNS_DATABASE
  int localanscount = 0;
#endif

  if (!qdcount || header->opcode != QUERY )
    return 0;

  /* determine end of question section (we put answers there) */
  if (!(ansp = skip_questions(header, qlen)))
    return 0; /* bad packet */
   
  /* now process each question, answers go in RRs after the question */
  p = (unsigned char *)(header+1);
  
  for (q=0; q<qdcount; q++)
    {
      /* save pointer to name for copying into answers */
      nameoffset = p - (unsigned char *)header;

      /* now extract name as .-concatenated string into name */
      if (!extract_name(header, qlen, &p, name, 1))
	return 0; /* bad packet */
      
      /* see if it's w.z.y.z.in-addr.arpa format */

      is_arpa = in_arpa_name_2_addr(name, &addr);
      
      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);

      if (qclass != C_IN)
	return 0; /* we can't answer non-inet queries */

#ifdef CONFIG_NK_LOCAL_DNS_DATABASE
	localanscount = query_local_answer(nameoffset,&ansp,name,qtype);

	if(localanscount)
	{
		anscount += localanscount;
		if (((unsigned char *)limit - ansp) < 0)
			return 0;
		continue;
	}
#endif

      ans = 0; /* have we answered this question */
      
      if ((options & OPT_FILTER) &&
      		(qtype == T_SOA
#ifdef T_SRV
			|| qtype == T_SRV
#endif
		))
	ans = 1;
      
      if (((qtype == T_PTR) || (qtype == T_ANY)) && is_arpa)
	{
	  crecp = NULL;
	  while ((crecp = cache_find_by_addr(crecp, &addr, now, is_arpa)))
	    { 
	      unsigned long ttl = (crecp->flags & F_IMMORTAL) ? 0 : crecp->ttd - now;
	      
	      /* don't answer wildcard queries with data not from /etc/hosts 
		 or dhcp leases */
	      if (qtype == T_ANY && !(crecp->flags & (F_HOSTS | F_DHCP)))
		return 0;
	      
	      ans = 1;
	      ansp = add_text_record(nameoffset, ansp, ttl, 0, T_PTR, 
				     cache_get_name(crecp));

	      log_query(crecp->flags & ~F_FORWARD, cache_get_name(crecp), &addr);
	      anscount++;
	      
	      /* if last answer exceeded packet size, give up */
	      if (((unsigned char *)limit - ansp) < 0)
		return 0;
	    }
	  
	  /* if not in cache, enabled and private IPV4 address, fake up answer */
	  if (ans == 0 && is_arpa == F_IPV4 && 
	      (options & OPT_BOGUSPRIV) && 
	      private_net(&addr))
	    {
	      struct in_addr addr4 = *((struct in_addr *)&addr);
	      char sa[INET_ADDRSTRLEN];
		  inet_ntop(AF_INET, &addr4, sa, INET_ADDRSTRLEN);
	      ansp = add_text_record(nameoffset, ansp, 0, 0, T_PTR, sa);
	      log_query(F_REVERSE | F_IPV4, sa, &addr);
	      anscount++;
	      ans = 1;

	      if (((unsigned char *)limit - ansp) < 0)
		return 0;
	    }
	}
	  
      if (((qtype == T_A) || qtype == T_ANY) && !is_arpa)
	{
	  /* T_ANY queries for hostnames with underscores are spam
             from win2k - don't forward them. */
	  if ((options & OPT_FILTER) && 
	      qtype == T_ANY && 
	      (strchr(name, '_') != NULL))
	    ans = 1;
	  else
	    { 
	      crecp = NULL;
	      while ((crecp = cache_find_by_name(crecp, name, now, F_IPV4)))
		{ 
		  unsigned long ttl = (crecp->flags & F_IMMORTAL) ? 0 : crecp->ttd - now;
		  /* don't answer wildcard queries with data not from /etc/hosts
		     or DHCP leases */
		  if (qtype == T_ANY && !(crecp->flags & (F_HOSTS | F_DHCP)))
		    return 0;
		  
		  /* If we have negative cache entry, it's OK
		     to return no answer. */
		  ans = 1;
		  
		  if (crecp->flags & F_NEG)
		    log_query(F_FORWARD | F_IPV4, name, NULL);
		  else
		    {
		      log_query(crecp->flags & ~F_REVERSE, name, &crecp->addr);
		      
		      /* copy question as first part of answer (use compression) */
		      PUTSHORT(nameoffset | 0xc000, ansp); 
		      PUTSHORT(T_A, ansp);
		      PUTSHORT(C_IN, ansp);
		      PUTLONG(ttl, ansp); /* TTL */
		      
		      PUTSHORT(INADDRSZ, ansp);
		      memcpy(ansp, &crecp->addr, INADDRSZ);
		      ansp += INADDRSZ;
		      anscount++;
		      
		      if (((unsigned char *)limit - ansp) < 0)
			return 0;
		    }
		  
		}
	    }
	}

#ifdef HAVE_IPV6
      if (((qtype == T_AAAA) || qtype == T_ANY) && !is_arpa)
	{
	  /* T_ANY queries for hostnames with underscores are spam
             from win2k - don't forward them. */
	  if ((options && OPT_FILTER) &&
	      qtype == T_ANY 
	      && (strchr(name, '_') != NULL))
	    ans = 1;
	  else
	    { 
	      crecp = NULL;
	      while ((crecp = cache_find_by_name(crecp, name, now, F_IPV6)))
		{ 
		  unsigned long ttl = (crecp->flags & F_IMMORTAL) ? 0 : crecp->ttd - now;
		  /* don't answer wildcard queries with data not from /etc/hosts
		     or DHCP leases */
		  if (qtype == T_ANY && !(crecp->flags & (F_HOSTS | F_DHCP)))
		    return 0;
		  
		  /* If we have negative cache entry, it's OK
		     to return no answer. */
		  ans = 1;
		  
		  if (crecp->flags & F_NEG)
		    log_query(F_FORWARD | F_IPV6, name, NULL);
		  else
		    {
		      log_query(crecp->flags & ~F_REVERSE, name, &crecp->addr);

		      /* copy question as first part of answer (use compression) */
		      PUTSHORT(nameoffset | 0xc000, ansp); 
		      PUTSHORT(T_AAAA, ansp);
		      PUTSHORT(C_IN, ansp);
		      PUTLONG(ttl, ansp); /* TTL */
		      
		      PUTSHORT(IN6ADDRSZ, ansp);
		      memcpy(ansp, &crecp->addr, IN6ADDRSZ);
		      ansp += IN6ADDRSZ;
		      anscount++;
		      
		      if (((unsigned char *)limit - ansp) < 0)
			return 0;
		    }
		}
	    }
	}
#endif

      if (((qtype == T_MX) || (qtype == T_ANY)) && !is_arpa)
	{
	  if (mxname && (strcmp(name, mxname) == 0))
	    {
	      ansp = add_text_record(nameoffset, ansp, 0, 1, T_MX, mxtarget);
	      anscount++;
	      ans = 1;
	    }
	  else if ((options && OPT_SELFMX) && 
		   cache_find_by_name(NULL, name, now, F_HOSTS | F_DHCP))
	    { 
	      ansp = add_text_record(nameoffset, ansp, 0, 1, T_MX, name);
	      anscount++;
	      ans = 1;
	    }
	  if (((unsigned char *)limit - ansp) < 0)
	      return 0;
	}
      if (!ans)
	return 0; /* failed to answer a question */

    }
  
  /* done all questions, set up header and return length of result */
  header->qr = 1; /* response */
  header->aa = 0; /* authoritive - never */
  header->ra = 1; /* recursion if available */
  header->tc = 0; /* truncation */
  header->rcode = NOERROR; /* no error */
  header->ancount = htons(anscount);
  header->nscount = htons(0);
  header->arcount = htons(0);
  return ansp - (unsigned char *)header;
}
