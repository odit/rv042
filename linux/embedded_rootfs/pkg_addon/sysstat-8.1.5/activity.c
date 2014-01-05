/*
 * activity.c: Define system activities
 * (C) 1999-2008 by Sebastien GODARD (sysstat <at> orange.fr)
 *
 ***************************************************************************
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published  by  the *
 * Free Software Foundation; either version 2 of the License, or (at  your *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it  will  be  useful,  but *
 * WITHOUT ANY WARRANTY; without the implied warranty  of  MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License *
 * for more details.                                                       *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                   *
 ***************************************************************************
 */

#include "sa.h"
#include "rd_stats.h"
#include "pr_stats.h"
#include "prf_stats.h"


/*
 ***************************************************************************
 * Definitions of system activities.
 * See sa.h file for activity structure definition.
 ***************************************************************************
 */

/*
 * CPU statistics.
 * This is the only activity which *must* be collected by sadc
 * so that uptime can be filled.
 */
struct activity cpu_act = {
	.id		= A_CPU,
	.options	= AO_COLLECTED + AO_REMANENT + AO_GLOBAL_ITV + AO_MULTIPLE_OUTPUTS,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_cpu_nr,
	.f_read		= wrap_read_stat_cpu,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_cpu_stats,
	.f_print_avg	= print_cpu_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_cpu_stats,
	.f_xml_print	= xml_print_cpu_stats,
#endif
	.nr		= -1,
	.fsize		= STATS_CPU_SIZE,
	.msize		= STATS_CPU_SIZE,
	.opt_flags	= AO_F_CPU_DEF,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= NR_CPUS,
	.hdr_line	= "CPU;%user;%nice;%system;%iowait;%steal;%idle|CPU;%usr;%nice;%sys;%iowait;%steal;%irq;%soft;%guest;%idle"
};

/* Process (task) creation and context switch activity */
struct activity pcsw_act = {
	.id		= A_PCSW,
	.options	= AO_COLLECTED,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_read		= wrap_read_stat_pcsw,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_pcsw_stats,
	.f_print_avg	= print_pcsw_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_pcsw_stats,
	.f_xml_print	= xml_print_pcsw_stats,
#endif
	.nr		= 1,
	.fsize		= STATS_PCSW_SIZE,
	.msize		= STATS_PCSW_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "proc/s;cswch/s"
};

/* Interrupts statistics */
struct activity irq_act = {
	.id		= A_IRQ,
	.options	= AO_NULL,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_irq_nr,
	.f_read		= wrap_read_stat_irq,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_irq_stats,
	.f_print_avg	= print_irq_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_irq_stats,
	.f_xml_print	= xml_print_irq_stats,
#endif
	.nr		= -1,
	.fsize		= STATS_IRQ_SIZE,
	.msize		= STATS_IRQ_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= NR_IRQS,
	.hdr_line	= "INTR;intr/s"
};

/* Swapping activity */
struct activity swap_act = {
	.id		= A_SWAP,
	.options	= AO_COLLECTED,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_read		= wrap_read_swap,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_swap_stats,
	.f_print_avg	= print_swap_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_swap_stats,
	.f_xml_print	= xml_print_swap_stats,
#endif
	.nr		= 1,
	.fsize		= STATS_SWAP_SIZE,
	.msize		= STATS_SWAP_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "pswpin/s;pswpout/s"
};

/* Paging activity */
struct activity paging_act = {
	.id		= A_PAGE,
	.options	= AO_COLLECTED,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_read		= wrap_read_paging,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_paging_stats,
	.f_print_avg	= print_paging_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_paging_stats,
	.f_xml_print	= xml_print_paging_stats,
#endif
	.nr		= 1,
	.fsize		= STATS_PAGING_SIZE,
	.msize		= STATS_PAGING_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "pgpgin/s;pgpgout/s;fault/s;majflt/s;pgfree/s;pgscank/s;pgscand/s;pgsteal/s;%vmeff"
};

/* I/O and transfer rate activity */
struct activity io_act = {
	.id		= A_IO,
	.options	= AO_COLLECTED,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_read		= wrap_read_io,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_io_stats,
	.f_print_avg	= print_io_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_io_stats,
	.f_xml_print	= xml_print_io_stats,
#endif
	.nr		= 1,
	.fsize		= STATS_IO_SIZE,
	.msize		= STATS_IO_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "tps;rtps;wtps;bread/s;bwrtn/s"
};

/* Memory and swap space utilization activity */
struct activity memory_act = {
	.id		= A_MEMORY,
	.options	= AO_COLLECTED + AO_MULTIPLE_OUTPUTS,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_read		= wrap_read_meminfo,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_memory_stats,
	.f_print_avg	= print_avg_memory_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_memory_stats,
	.f_xml_print	= xml_print_memory_stats,
#endif
	.nr		= 1,
	.fsize		= STATS_MEMORY_SIZE,
	.msize		= STATS_MEMORY_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "frmpg/s;bufpg/s;campg/s|kbmemfree;kbmemused;%memused;kbbuffers;kbcached;kbcommit;%commit|kbswpfree;kbswpused;%swpused;kbswpcad;%swpcad"
};

/* Kernel tables activity */
struct activity ktables_act = {
	.id		= A_KTABLES,
	.options	= AO_COLLECTED,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_read		= wrap_read_kernel_tables,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_ktables_stats,
	.f_print_avg	= print_avg_ktables_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_ktables_stats,
	.f_xml_print	= xml_print_ktables_stats,
#endif
	.nr		= 1,
	.fsize		= STATS_KTABLES_SIZE,
	.msize		= STATS_KTABLES_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "dentunusd;file-nr;inode-nr;pty-nr"
};

/* Queue and load activity */
struct activity queue_act = {
	.id		= A_QUEUE,
	.options	= AO_COLLECTED,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_read		= wrap_read_loadavg,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_queue_stats,
	.f_print_avg	= print_avg_queue_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_queue_stats,
	.f_xml_print	= xml_print_queue_stats,
#endif
	.nr		= 1,
	.fsize		= STATS_QUEUE_SIZE,
	.msize		= STATS_QUEUE_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "runq-sz;plist-sz;ldavg-1;ldavg-5;ldavg-15"
};

/* Serial lines activity */
struct activity serial_act = {
	.id		= A_SERIAL,
	.options	= AO_COLLECTED,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_serial_nr,
	.f_read		= wrap_read_tty_driver_serial,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_serial_stats,
	.f_print_avg	= print_serial_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_serial_stats,
	.f_xml_print	= xml_print_serial_stats,
#endif
	.nr		= -1,
	.fsize		= STATS_SERIAL_SIZE,
	.msize		= STATS_SERIAL_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "TTY;rcvin/s;txmtin/s;framerr/s;prtyerr/s;brk/s;ovrun/s"
};

/* Block devices activity */
struct activity disk_act = {
	.id		= A_DISK,
	.options	= AO_NULL,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_disk_nr,
	.f_read		= wrap_read_disk,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_disk_stats,
	.f_print_avg	= print_disk_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_disk_stats,
	.f_xml_print	= xml_print_disk_stats,
#endif
	.nr		= -1,
	.fsize		= STATS_DISK_SIZE,
	.msize		= STATS_DISK_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "DEV;tps;rd_sec/s;wr_sec/s;avgrq-sz;avgqu-sz;await;svctm;%util"
};

/* Network interfaces activity */
struct activity net_dev_act = {
	.id		= A_NET_DEV,
	.options	= AO_COLLECTED,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_iface_nr,
	.f_read		= wrap_read_net_dev,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_dev_stats,
	.f_print_avg	= print_net_dev_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_dev_stats,
	.f_xml_print	= xml_print_net_dev_stats,
#endif
	.nr		= -1,
	.fsize		= STATS_NET_DEV_SIZE,
	.msize		= STATS_NET_DEV_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "IFACE;rxpck/s;txpck/s;rxkB/s;txkB/s;rxcmp/s;txcmp/s;rxmcst/s"
};

/* Network interfaces activity */
struct activity net_edev_act = {
	.id		= A_NET_EDEV,
	.options	= AO_COLLECTED,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_iface_nr,
	.f_read		= wrap_read_net_edev,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_edev_stats,
	.f_print_avg	= print_net_edev_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_edev_stats,
	.f_xml_print	= xml_print_net_edev_stats,
#endif
	.nr		= -1,
	.fsize		= STATS_NET_EDEV_SIZE,
	.msize		= STATS_NET_EDEV_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "IFACE;rxerr/s;txerr/s;coll/s;rxdrop/s;txdrop/s;txcarr/s;rxfram/s;rxfifo/s;txfifo/s"
};

/* NFS client activity */
struct activity net_nfs_act = {
	.id		= A_NET_NFS,
	.options	= AO_COLLECTED,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_read		= wrap_read_net_nfs,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_nfs_stats,
	.f_print_avg	= print_net_nfs_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_nfs_stats,
	.f_xml_print	= xml_print_net_nfs_stats,
#endif
	.nr		= 1,
	.fsize		= STATS_NET_NFS_SIZE,
	.msize		= STATS_NET_NFS_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "call/s;retrans/s;read/s;write/s;access/s;getatt/s"
};

/* NFS server activity */
struct activity net_nfsd_act = {
	.id		= A_NET_NFSD,
	.options	= AO_COLLECTED,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_read		= wrap_read_net_nfsd,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_nfsd_stats,
	.f_print_avg	= print_net_nfsd_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_nfsd_stats,
	.f_xml_print	= xml_print_net_nfsd_stats,
#endif
	.nr		= 1,
	.fsize		= STATS_NET_NFSD_SIZE,
	.msize		= STATS_NET_NFSD_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "scall/s;badcall/s;packet/s;udp/s;tcp/s;hit/s;miss/s;sread/s;swrite/s;saccess/s;sgetatt/s"
};

/* Network sockets activity */
struct activity net_sock_act = {
	.id		= A_NET_SOCK,
	.options	= AO_COLLECTED + AO_CLOSE_MARKUP,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_read		= wrap_read_net_sock,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_sock_stats,
	.f_print_avg	= print_avg_net_sock_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_sock_stats,
	.f_xml_print	= xml_print_net_sock_stats,
#endif
	.nr		= 1,
	.fsize		= STATS_NET_SOCK_SIZE,
	.msize		= STATS_NET_SOCK_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL,
	.bitmap_size	= 0,
	.hdr_line	= "totsck;tcpsck;udpsck;rawsck;ip-frag;tcp-tw"
};


/*
 * Array of activities.
 */
struct activity *act[NR_ACT] = {
	&cpu_act,
	&pcsw_act,
	&irq_act,
	&swap_act,
	&paging_act,
	&io_act,
	&memory_act,
	&ktables_act,
	&queue_act,
	&serial_act,
	&disk_act,
	&net_dev_act,
	&net_edev_act,
	&net_nfs_act,
	&net_nfsd_act,
	&net_sock_act
};
