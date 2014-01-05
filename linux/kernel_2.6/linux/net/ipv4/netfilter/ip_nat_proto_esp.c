#include <linux/config.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_nat_protocol.h>
#include <linux/netfilter_ipv4/ip_conntrack_proto_esp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NK Incifer");
MODULE_DESCRIPTION("Netfilter NAT protocol helper module for ESP");

static int nk_esp_debug=0;

/* is key in given range between min and max */
static int
esp_in_range(const struct ip_conntrack_tuple *tuple,
         enum ip_nat_manip_type maniptype,
         const union ip_conntrack_manip_proto *min,
         const union ip_conntrack_manip_proto *max)
{
    nk_esp_print ( "ESP ALG: %s\n", __func__ );

    return 1;
}

/* generate unique tuple ... */
static int 
esp_unique_tuple(struct ip_conntrack_tuple *tuple,
         const struct ip_nat_range *range,
         enum ip_nat_manip_type maniptype,
         const struct ip_conntrack *conntrack)
{
    nk_esp_print ( "ESP ALG: %s\n", __func__ );

    return 1;
}

/* manipulate a ESP packet according to maniptype */
static int
esp_manip_pkt(struct sk_buff **pskb,
          unsigned int iphdroff,
          const struct ip_conntrack_tuple *tuple,
          enum ip_nat_manip_type maniptype)
{
    nk_esp_print ( "ESP ALG: %s\n", __func__ );

    return 1;
}

/* nat helper struct */
static struct ip_nat_protocol esp = {
    .name           = "ESP", 
    .protonum       = IPPROTO_ESP,
    .manip_pkt      = esp_manip_pkt,
    .in_range       = esp_in_range,
    .unique_tuple   = esp_unique_tuple,
};

int __init ip_nat_proto_esp_init(void)
{
    int result=0;

    if ( ( result = ip_nat_protocol_register ( &esp ) ) != 0 )
    {
        printk ( KERN_EMERG "ESP ALG: ERR: %s: register helper for esp failed(%d).\n", __func__, result );
    }
    else
    {
        nk_esp_print ( "ESP ALG: %s: register helper for esp success(%d).\n", __func__, result );
    }

    return result;
}

void __exit ip_nat_proto_esp_fini(void)
{
    ip_nat_protocol_unregister ( &esp );
}

module_init(ip_nat_proto_esp_init);
module_exit(ip_nat_proto_esp_fini);

