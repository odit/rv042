#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/list.h>
#include <linux/seq_file.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/netdevice.h>
#include <linux/mii.h>
#include "../../../../cavium-ethernet/cavium-ethernet.h"

static DEFINE_RWLOCK(ip_ct_esp_lock);
#define ASSERT_READ_LOCK(x)
#define ASSERT_WRITE_LOCK(x)

#include <linux/netfilter_ipv4/listhelp.h>
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>

#include <linux/netfilter_ipv4/ip_conntrack_proto_esp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NK Incifer");
MODULE_DESCRIPTION("netfilter connection tracking protocol helper for ESP");


static struct _esp_table esp_table[MAX_PORTS];
static uint16_t cur_spi = 0;

static int nk_esp_debug=0;
int nk_esp_err;
dev_t nk_esp_dev;
struct cdev *nk_esp_cdev;
u_int32_t nk_esp_last_spi=0;


/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 *               get device index, ex: eth0->0, eth1->1
 *               better than get device name
 */
#if 1
int nk_esp_get_dev_idx ( const struct net_device *dev )
{
    if ( !dev )
    {
	printk ( KERN_EMERG "%s: dev == NULL\n", __func__ );
        return -1;
    }
    if ( !((cvm_oct_private_t *)dev->priv) )
    {
	    printk ( KERN_EMERG "%s: dev[%s], priv == NULL\n", __func__, dev->name );
	    return -1;    
    }
    return ( ( (cvm_oct_private_t *)dev->priv )->nk_vid & 0xFFFF ) - 1;
}
#endif
					    
/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 *               return free entry else return null
 */
struct _esp_table *alloc_esp_entry ( const struct net_device *dev, const struct esphdr *esph,
                         u_int32_t daddr, u_int32_t saddr )
{
    int idx = 0;

    struct _esp_table *esp_entry = esp_table;

/* purpose     : 0012641
    * author      : incifer
    * date        : 2010-08-19
    * description : Only LAN to WAN ESP can insert entry, to fix table chaos and QVPN pass through disconnect
 */
    /* first pkt must from lan */
     if ( nk_esp_get_dev_idx ( dev ) != 0 )
     {
	     return NULL;
     }

    for ( idx = 0; idx < MAX_PORTS; esp_entry++, idx++ )
    {
        if ( esp_entry->inuse == IPSEC_FREE )
        {
            continue;
        }

        /* prevent allocate the same entry */
        if ( esp_entry->l_spi == ntohl(esph->spi) && esp_entry->l_ip == saddr )
        {
		return esp_entry;
        }
    }

    esp_entry = esp_table;
    for ( idx = 0; idx < MAX_PORTS; esp_entry++, idx++ )
    {
        if ( esp_entry->inuse == IPSEC_FREE )
        {
            esp_entry->tspi  = cur_spi = TEMP_SPI_START + idx;
            esp_entry->inuse = IPSEC_INUSE;

            esp_entry->l_spi = ntohl( esph->spi );
            esp_entry->l_ip  = ntohl( saddr );
            esp_entry->seq   = ntohl( esph->seq );

	    return esp_entry;
        }
    }

    return NULL;
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 *               search a entry by spi
 */
struct _esp_table *search_esp_entry_by_spi ( const struct esphdr *esph,
                         u_int32_t daddr, u_int32_t saddr,int which_func_call )
{
    int idx = (MAX_PORTS - 1);
    struct _esp_table *esp_entry;
    u_int32_t da = ntohl(daddr);
    u_int32_t sa = ntohl(saddr);

    esp_entry = esp_table;
    for ( idx = 0; idx < MAX_PORTS; idx++, esp_entry++ )
    {
        if ( esp_entry->inuse == IPSEC_FREE )
        {
            continue;
        }

        /* orignal */
        if ( esp_entry->l_spi == esph->spi && esp_entry->l_ip == saddr )
        {
            return esp_entry;
        }

	/* purpose     : 0012641
	* author      : incifer
	* date        : 2010-07-16
	* description : Fix the reply IP record error
	 */
	/* reply */
        if ( esp_entry->r_spi == esph->spi && esp_entry->r_ip == saddr )
        {
            return esp_entry;
        }
    }

    esp_entry = esp_table;
    for ( idx = 0; idx < MAX_PORTS; idx++, esp_entry++ ) {
        if ( esp_entry->inuse == IPSEC_FREE )
        {
            continue;
        }

        nk_esp_print ( "ESP ALG: %s: entry info: which_func_call[%d], idx[%d], [%u.%u.%u.%u]/[0x%08x] -> [%u.%u.%u.%u]/[0x%08x], seq[%u], tspi[%u]\n", __func__,
            which_func_call, idx,
            NIPQUAD ( esp_entry->l_ip ), esp_entry->l_spi,
            NIPQUAD ( esp_entry->r_ip ), esp_entry->r_spi,
            esp_entry->seq, esp_entry->tspi);
        nk_esp_print ( "ESP ALG: %s: esph info: spi[0x%08x], seq[%d]\n", __func__,
            esph->spi, esph->seq );

        /* If we have seen traffic both ways */
        if ( esp_entry->l_spi != 0 && esp_entry->r_spi != 0 )
        {
            if ( esp_entry->l_spi == ntohl ( esph->spi ) &&
                 esp_entry->l_ip == sa )
            {
                nk_esp_print ( "ESP ALG: %s: find esp entry(0), [%u.%u.%u.%u]/[0x%08x] -> [%u.%u.%u.%u]/[0x%08x], seq[%u], tspi[%u], idx[%u]\n", __func__,
                    NIPQUAD ( esp_entry->l_ip ), esp_entry->l_spi,
                    NIPQUAD ( esp_entry->r_ip ), esp_entry->r_spi,
                    esp_entry->seq, esp_entry->tspi, idx );
                return esp_entry;
            }
            else if ( esp_entry->r_spi == ntohl ( esph->spi ) &&
                      esp_entry->r_ip == da)
            {
                nk_esp_print ( "ESP ALG: %s: find esp entry(0), [%u.%u.%u.%u]/[0x%08x] -> [%u.%u.%u.%u]/[0x%08x], seq[%u], tspi[%u], idx[%u]\n", __func__,
                    NIPQUAD ( esp_entry->l_ip ), esp_entry->l_spi,
                    NIPQUAD ( esp_entry->r_ip ), esp_entry->r_spi,
                    esp_entry->seq, esp_entry->tspi, idx );
                return esp_entry;
            }

            continue;
        }

        /* If we have seen traffic only one way */
        if ( esp_entry->l_spi == 0 || esp_entry->r_spi == 0 )
        {
            /* We have traffic from local */
            if ( esp_entry->l_spi )
            {
                if ( esp_entry->tspi == cur_spi && esp_entry->l_spi != ntohl(esph->spi) && esp_entry->l_ip != saddr )
                {
                    esp_entry->r_spi = ntohl(esph->spi);
                    esp_entry->r_ip = ntohl(saddr);
                    cur_spi = 0;
                    nk_esp_print ( "ESP ALG: %s: find esp entry(L), [%u.%u.%u.%u]/[0x%08x] -> [%u.%u.%u.%u]/[0x%08x], seq[%u], tspi[%u], idx[%u]\n", __func__,
                        NIPQUAD ( esp_entry->l_ip ), esp_entry->l_spi,
                        NIPQUAD ( esp_entry->r_ip ), esp_entry->r_spi,
                        esp_entry->seq, esp_entry->tspi, idx );
                    return esp_entry;
                }

                continue;
            }
            /* We have seen traffic only from remote */
            else if ( esp_entry->r_spi )
            {
                if ( esp_entry->tspi == cur_spi && esp_entry->r_spi != ntohl(esph->spi) )
                {
                    esp_entry->l_spi = ntohl(esph->spi);
                    cur_spi = 0;
                    nk_esp_print ( "ESP ALG: %s: find esp entry(R), [%u.%u.%u.%u]/[0x%08x] -> [%u.%u.%u.%u]/[0x%08x], seq[%u], tspi[%u], idx[%u]\n", __func__,
                        NIPQUAD ( esp_entry->l_ip ), esp_entry->l_spi,
                        NIPQUAD ( esp_entry->r_ip ), esp_entry->r_spi,
                        esp_entry->seq, esp_entry->tspi, idx );
                    return esp_entry;
                }

                continue;
            }
        }
    }

    return NULL;
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 *               invert tuple
 */
static int esp_invert_tuple(struct ip_conntrack_tuple *tuple,
                const struct ip_conntrack_tuple *orig)
{
    nk_esp_print ( "ESP ALG: %s\n", __func__ );

    tuple->src.u.esp.spi = orig->dst.u.esp.spi;
    tuple->dst.u.esp.spi = orig->src.u.esp.spi;

    return NF_ACCEPT;
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 *               return a tuple
 */
static int esp_pkt_to_tuple(const struct sk_buff *skb,
               unsigned int dataoff,
               struct ip_conntrack_tuple *tuple)
{
    struct esphdr *esph, _esph;
    struct iphdr *iph;
    struct _esp_table *esp_entry=NULL;

    nk_esp_print ( "ESP ALG: %s\n", __func__ );

    iph = skb->nh.iph;
    //esph = skb_header_pointer ( skb, dataoff, sizeof ( struct esphdr ), &_esph );
    esph = skb->data + dataoff;
    if ( !esph )
    {
        return NF_ACCEPT;
    }

    nk_esp_print ( "ESP ALG: %s: ip [%u.%u.%u.%u]->[%u.%u.%u.%u], spi [0x%08x]/[%u]\n", __func__,
        NIPQUAD ( iph->saddr ), NIPQUAD ( iph->daddr ),
        esph->spi, esph->seq );

    read_lock_bh(&ip_ct_esp_lock);

    if ( ( esp_entry = search_esp_entry_by_spi ( esph, tuple->dst.ip, tuple->src.ip, Esp_Pkt_To_Tuple_0 ) ) == NULL ) {
        esp_entry = alloc_esp_entry( skb->dev, esph, tuple->dst.ip, tuple->src.ip );
        if ( esp_entry == NULL )
        {
            nk_esp_print ( "ESP ALG: %s: esp table is NULL\n", __func__ );
            read_unlock_bh(&ip_ct_esp_lock);
            return NF_DROP;
        }

        nk_esp_print ( "ESP ALG: %s: new esp session, [%u.%u.%u.%u]/[0x%08x] -> [%u.%u.%u.%u]/[0x%08x], seq[%u], tspi[%u]\n", __func__,
            NIPQUAD ( esp_entry->l_ip ), esp_entry->l_spi,
            NIPQUAD ( esp_entry->r_ip ), esp_entry->r_spi,
            esp_entry->seq, esp_entry->tspi );
    }
    else
    {
        nk_esp_print ( "ESP ALG: %s: old esp session, [%u.%u.%u.%u]/[0x%08x] -> [%u.%u.%u.%u]/[0x%08x], seq[%u], tspi[%u]\n", __func__,
            NIPQUAD ( esp_entry->l_ip ), esp_entry->l_spi,
            NIPQUAD ( esp_entry->r_ip ), esp_entry->r_spi,
            esp_entry->seq, esp_entry->tspi );
    }

    tuple->dst.u.esp.spi = esp_entry->tspi;
    tuple->src.u.esp.spi = esp_entry->tspi;

    read_unlock_bh(&ip_ct_esp_lock);

    DUMP_TUPLE_ESP(tuple);

    return NF_ACCEPT;
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 */
static int esp_print_tuple(struct seq_file *s,
               const struct ip_conntrack_tuple *tuple)
{
    return seq_printf(s, "srcspi=%u dstspi=%u ",
              ntohs(tuple->src.u.esp.spi),
              ntohs(tuple->dst.u.esp.spi));
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 */
static int esp_print_conntrack(struct seq_file *s,
                   const struct ip_conntrack *ct)
{
    return seq_printf(s, "timeout=%u, stream_timeout=%u ",
              (ct->proto.esp.timeout / HZ),
              (ct->proto.esp.stream_timeout / HZ));
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 */
static int esp_packet(struct ip_conntrack *ct,
              const struct sk_buff *skb,
              enum ip_conntrack_info conntrackinfo)
{
    nk_esp_print ( "ESP ALG: %s\n", __func__ );

    /* If we've seen traffic both ways, this is a ESP connection.
     * Extend timeout. */
    if ( ct->status & IPS_SEEN_REPLY )
    {
	/* purpose     : IPSec performance
	    * author      : David
	    * date        : 2010-09-97
	    * description : Only 2/16 ESP packet refresh conntrack aging time, to reduce ALG effort.
	*/
	    if((jiffies&0x0f) <= 1)
	    {
		    ip_ct_refresh_acct ( ct, conntrackinfo, skb, ct->proto.esp.stream_timeout );
        	    /* Also, more likely to be important, and not a probe. */
        	    set_bit ( IPS_ASSURED_BIT, &ct->status );
        	    ip_conntrack_event_cache ( IPCT_STATUS, skb );
	    }
    }
    else
    {
        ip_ct_refresh_acct(ct, conntrackinfo, skb, ct->proto.esp.timeout);
    }

    return NF_ACCEPT;
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 */
static int esp_new(struct ip_conntrack *ct,
           const struct sk_buff *skb)
{
    nk_esp_print ( "ESP ALG: %s\n", __func__ );
    DUMP_TUPLE_ESP ( &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple );
    DUMP_TUPLE_ESP ( &ct->tuplehash[IP_CT_DIR_REPLY].tuple );

    ct->proto.esp.stream_timeout = ESP_STREAM_TIMEOUT;
    ct->proto.esp.timeout = ESP_TIMEOUT;

    return 1;
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 */
static void esp_destroy(struct ip_conntrack *ct)
{
    int idx = 0;
    struct _esp_table *esp_entry = esp_table;
    struct ip_conntrack_tuple *tuple = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;

    nk_esp_print ( "ESP ALG: %s\n", __func__ );
    DUMP_TUPLE_ESP ( &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple );
    DUMP_TUPLE_ESP ( &ct->tuplehash[IP_CT_DIR_REPLY].tuple );

    write_lock_bh(&ip_ct_esp_lock);

    for ( idx = 0; idx < MAX_PORTS; idx++, esp_entry++ )
    {
        if ( esp_entry->inuse == IPSEC_FREE )
        {
            continue;
        }

        if ( esp_entry->tspi == tuple->src.u.esp.spi )
        {
            nk_esp_print ( "ESP ALG: %s: Del Entry [%u.%u.%u.%u]/[0x%08x] -> [%u.%u.%u.%u]/[0x%08x], tspi[%u], seq[%u]\n", __func__,
                NIPQUAD ( esp_entry->l_ip ), esp_entry->l_spi,
                NIPQUAD ( esp_entry->r_ip ), esp_entry->r_spi,
                esp_entry->seq, esp_entry->tspi );
            memset ( esp_entry, 0, sizeof ( struct _esp_table ) );

            write_unlock_bh(&ip_ct_esp_lock);

            return;
        }
    }

    write_unlock_bh(&ip_ct_esp_lock);
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 *               - idx: -1: print all table except free
 *                      >0: print specific idx entry
 */
static void nk_esp_print_table ( int idx )
{
    int i;
    struct _esp_table *esp_entry;

    read_lock_bh(&ip_ct_esp_lock);

    if ( idx == -1 )
    {
        esp_entry = esp_table;
        for ( i = 0; i < MAX_PORTS; esp_entry++, i++)
        {
            if ( esp_entry->inuse == IPSEC_FREE )
            {
                continue;
            }

            printk ( KERN_EMERG "ESP ALG: %s: %03d: [%u.%u.%u.%u]/[0x%08x] -> [%u.%u.%u.%u]/[0x%08x], tspi[%u], seq[%u]\n", __func__, i,
                NIPQUAD(esp_entry->l_ip), esp_entry->l_spi,
                NIPQUAD(esp_entry->r_ip), esp_entry->r_spi,
                esp_entry->tspi, esp_entry->seq );
        }
    }
    else
    {
        if ( idx < 0 || idx >= MAX_PORTS )
        {
            printk ( KERN_EMERG "ESP ALG: %s: idx(%d) range is [0:%d]\n", __func__, idx, MAX_PORTS );
            read_unlock_bh(&ip_ct_esp_lock);
            return;
        }

        esp_entry = &(esp_table[idx]);
        printk ( KERN_EMERG "ESP ALG: %s: %03d: [%u.%u.%u.%u]/[0x%08x] -> [%u.%u.%u.%u]/[0x%08x], tspi[%u], seq[%u], inuse[%u]\n", __func__, idx,
                NIPQUAD(esp_entry->l_ip), esp_entry->l_spi,
                NIPQUAD(esp_entry->r_ip), esp_entry->r_spi,
                esp_entry->tspi, esp_entry->seq, esp_entry->inuse );
    }

    read_unlock_bh(&ip_ct_esp_lock);
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 */
static void nk_esp_del_table ( int idx )
{
    write_lock_bh(&ip_ct_esp_lock);

    if ( idx == -1 )
    {
        memset ( esp_table, 0, sizeof ( struct _esp_table ) * MAX_PORTS );
    }
    else
    {
        if ( idx < 0 || idx >= MAX_PORTS )
        {
            printk ( KERN_EMERG "ESP ALG: %s: idx(%d) range is [0:%d]\n", __func__, idx, MAX_PORTS );
            write_unlock_bh(&ip_ct_esp_lock);
            return;
        }

        memset ( &(esp_table[idx]), 0, sizeof ( struct _esp_table ) );
    }

    write_unlock_bh(&ip_ct_esp_lock);
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 */
static int nk_esp_device_ioctl ( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg )
{
    switch ( cmd )
    {
        case NK_ESP_PRINT_TABLE:
        {
            int idx;

            copy_from_user ( &idx, (void *)arg, sizeof ( int ) );
            nk_esp_print_table ( idx );

            break;
        }
        case NK_ESP_DEL_TABLE:
        {
            int idx;

            copy_from_user ( &idx, (void *)arg, sizeof ( int ) );
            nk_esp_del_table ( idx );

            break;
        }
        case NK_ESP_SET_DEBUG:
        {
            copy_from_user ( &nk_esp_debug, (void *)arg, sizeof ( int ) );

            if ( nk_esp_debug )
            {
                printk ( KERN_EMERG "AGL ESP: %s: Enable Debug\n", __func__ );
            }
            else
            {
                printk ( KERN_EMERG "AGL ESP: %s: Dsiable Debug\n", __func__ );
            }
            break;
        }
        default:
            printk ( KERN_EMERG "Inappropriate esp ioctl[%x] for device...\n", cmd );
            return ENOTTY;
    }

    return 0;
}

static struct file_operations nk_esp_device_fops = {
    .owner      = THIS_MODULE,
    .ioctl      = nk_esp_device_ioctl,
};

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 */
static int nk_esp_reg_char_device(void)
{
    nk_esp_dev = MKDEV ( ESP_MAJOR_NUM, ESP_MINOR_NUM );

    if ( ( nk_esp_err = register_chrdev_region ( nk_esp_dev, 1, "nk_esp" ) ) != 0)
    {
        printk ( KERN_EMERG "ESP ALG: %s: register_chrdev_region() failed (%d)\n", __func__, nk_esp_err );
        return nk_esp_err;
    }

    nk_esp_cdev = cdev_alloc();
    nk_esp_cdev->owner = THIS_MODULE;
    nk_esp_cdev->ops = &nk_esp_device_fops;

    if ( ( nk_esp_err = cdev_add ( nk_esp_cdev, nk_esp_dev, 1 ) ) != 0 )
    {
        printk ( KERN_EMERG "ESP ALG: %s: cdev_add() failed (%d)\n", __func__, nk_esp_err );
        return nk_esp_err;
    }
    return 0;
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 *               protocol helper struct
 */
struct ip_conntrack_protocol esp = {
    .proto              = IPPROTO_ESP,
    .name               = "esp",
    .pkt_to_tuple       = esp_pkt_to_tuple,
    .invert_tuple       = esp_invert_tuple,
    .print_tuple        = esp_print_tuple,
    .print_conntrack    = esp_print_conntrack,
    .packet             = esp_packet,
    .new                = esp_new,
    .destroy            = esp_destroy,
    .me                 = THIS_MODULE,
};

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 */
int __init ip_ct_proto_esp_init ( void )
{
    int result=0;

    memset ( esp_table, 0, sizeof ( struct _esp_table ) * MAX_PORTS );

    if ( ( result = ip_conntrack_protocol_register ( &esp ) ) != 0 )
    {
        printk ( KERN_EMERG "ESP ALG: ERR: %s: register helper for esp failed(%d).\n", __func__, result );
    }
    else
    {
        nk_esp_print ( "ESP ALG: %s: register helper for esp success(%d).\n", __func__, result );
    }

    printk ( KERN_EMERG "nk_esp_reg_char_device() result: %d\n", nk_esp_reg_char_device() );

    return result;
}

/* purpose     : 0012641
 * author      : incifer
 * date        : 2010-07-16
 * description : esp alg
 */
void ip_ct_proto_esp_fini ( void )
{
    ip_conntrack_protocol_unregister ( &esp );

    cdev_del(nk_esp_cdev);
    unregister_chrdev_region(nk_esp_dev, 1);
}

module_init(ip_ct_proto_esp_init);
module_exit(ip_ct_proto_esp_fini);

