/*
 * rd_stats.h: Include file used to read system statistics
 * (C) 1999-2008 by Sebastien Godard (sysstat <at> orange.fr)
 */

#ifndef _RD_STATS_H
#define _RD_STATS_H

#include "common.h"


/*
 ***************************************************************************
 * Miscellaneous constants
 ***************************************************************************
 */

/* Get IFNAMSIZ */
#include <net/if.h>
#ifndef IFNAMSIZ
#define IFNAMSIZ	16
#endif

/* Maximum length of network interface name */
#define MAX_IFACE_LEN	IFNAMSIZ

#define CNT_DEV		0
#define CNT_PART	1
#define CNT_ALL_DEV	0
#define CNT_USED_DEV	1

#define READ_DISKSTATS		1
#define READ_PPARTITIONS	2

/*
 ***************************************************************************
 * System files containing statistics
 ***************************************************************************
 */

/* Files */
#define PROC		"/proc"
#define SERIAL		"/proc/tty/driver/serial"
#define FDENTRY_STATE	"/proc/sys/fs/dentry-state"
#define FFILE_NR	"/proc/sys/fs/file-nr"
#define FINODE_STATE	"/proc/sys/fs/inode-state"
#define PTY_NR		"/proc/sys/kernel/pty/nr"
#define NET_DEV		"/proc/net/dev"
#define NET_SOCKSTAT	"/proc/net/sockstat"
#define NET_RPC_NFS	"/proc/net/rpc/nfs"
#define NET_RPC_NFSD	"/proc/net/rpc/nfsd"
#define LOADAVG		"/proc/loadavg"
#define VMSTAT		"/proc/vmstat"


/*
 ***************************************************************************
 * Definitions of structures for system statistics
 ***************************************************************************
 */

/*
 * Structure for CPU statistics.
 * In activity buffer: First structure is for global CPU utilisation ("all").
 * Following structures are for each individual CPU (0, 1, etc.)
 */
struct stats_cpu {
	unsigned long long cpu_user	__attribute__ ((aligned (16)));
	unsigned long long cpu_nice	__attribute__ ((aligned (16)));
	unsigned long long cpu_sys	__attribute__ ((aligned (16)));
	unsigned long long cpu_idle	__attribute__ ((aligned (16)));
	unsigned long long cpu_iowait	__attribute__ ((aligned (16)));
	unsigned long long cpu_steal	__attribute__ ((aligned (16)));
	unsigned long long cpu_hardirq	__attribute__ ((aligned (16)));
	unsigned long long cpu_softirq	__attribute__ ((aligned (16)));
	unsigned long long cpu_guest	__attribute__ ((aligned (16)));
};

#define STATS_CPU_SIZE	(sizeof(struct stats_cpu))

/*
 * Structure for task creation and context switch statistics.
 * The attribute (aligned(16)) is necessary so that sizeof(structure) has
 * the same value on 32 and 64-bit architectures.
 */
struct stats_pcsw {
	unsigned long long context_switch	__attribute__ ((aligned (16)));
	unsigned long processes			__attribute__ ((aligned (16)));
};

#define STATS_PCSW_SIZE	(sizeof(struct stats_pcsw))

/*
 * Structure for interrupts statistics.
 * In activity buffer: First structure is for total number of interrupts ("SUM").
 * Following structures are for each individual interrupt (0, 1, etc.)
 *
 * NOTE: The total number of interrupts is saved as a %llu by the kernel,
 * whereas individual interrupts are saved as %u.
 */
struct stats_irq {
	unsigned long long irq_nr	__attribute__ ((aligned (16)));
};

#define STATS_IRQ_SIZE	(sizeof(struct stats_irq))

/* Structure for swapping statistics */
struct stats_swap {
	unsigned long pswpin	__attribute__ ((aligned (8)));
	unsigned long pswpout	__attribute__ ((aligned (8)));
};

#define STATS_SWAP_SIZE	(sizeof(struct stats_swap))

/* Structure for paging statistics */
struct stats_paging {
	unsigned long pgpgin		__attribute__ ((aligned (8)));
	unsigned long pgpgout		__attribute__ ((aligned (8)));
	unsigned long pgfault		__attribute__ ((aligned (8)));
	unsigned long pgmajfault	__attribute__ ((aligned (8)));
	unsigned long pgfree		__attribute__ ((aligned (8)));
	unsigned long pgscan_kswapd	__attribute__ ((aligned (8)));
	unsigned long pgscan_direct	__attribute__ ((aligned (8)));
	unsigned long pgsteal		__attribute__ ((aligned (8)));
};

#define STATS_PAGING_SIZE	(sizeof(struct stats_paging))

/* Structure for I/O and transfer rate statistics */
struct stats_io {
	unsigned int  dk_drive			__attribute__ ((aligned (4)));
	unsigned int  dk_drive_rio		__attribute__ ((packed));
	unsigned int  dk_drive_wio		__attribute__ ((packed));
	unsigned int  dk_drive_rblk		__attribute__ ((packed));
	unsigned int  dk_drive_wblk		__attribute__ ((packed));
};

#define STATS_IO_SIZE	(sizeof(struct stats_io))

/* Structure for memory and swap space utilization statistics */
struct stats_memory {
	unsigned long frmkb	__attribute__ ((aligned (8)));
	unsigned long bufkb	__attribute__ ((aligned (8)));
	unsigned long camkb	__attribute__ ((aligned (8)));
	unsigned long tlmkb	__attribute__ ((aligned (8)));
	unsigned long frskb	__attribute__ ((aligned (8)));
	unsigned long tlskb	__attribute__ ((aligned (8)));
	unsigned long caskb	__attribute__ ((aligned (8)));
	unsigned long comkb	__attribute__ ((aligned (8)));
};

#define STATS_MEMORY_SIZE	(sizeof(struct stats_memory))

/* Structure for kernel tables statistics */
struct stats_ktables {
	unsigned int  file_used		__attribute__ ((aligned (4)));
	unsigned int  inode_used	__attribute__ ((packed));
	unsigned int  dentry_stat	__attribute__ ((packed));
	unsigned int  pty_nr		__attribute__ ((packed));
};

#define STATS_KTABLES_SIZE	(sizeof(struct stats_ktables))

/* Structure for queue and load statistics */
struct stats_queue {
	unsigned long nr_running	__attribute__ ((aligned (8)));
	unsigned int  load_avg_1	__attribute__ ((aligned (8)));
	unsigned int  load_avg_5	__attribute__ ((packed));
	unsigned int  load_avg_15	__attribute__ ((packed));
	unsigned int  nr_threads	__attribute__ ((packed));
};

#define STATS_QUEUE_SIZE	(sizeof(struct stats_queue))

/* Structure for serial statistics */
struct stats_serial {
	unsigned int rx		__attribute__ ((aligned (4)));
	unsigned int tx		__attribute__ ((packed));
	unsigned int frame	__attribute__ ((packed));
	unsigned int parity	__attribute__ ((packed));
	unsigned int brk	__attribute__ ((packed));
	unsigned int overrun	__attribute__ ((packed));
	/*
	 * A value of 0 means that the structure is unused.
	 * To avoid the confusion, the line number is saved as (line# + 1)
	 */
	unsigned int line	__attribute__ ((packed));
};

#define STATS_SERIAL_SIZE	(sizeof(struct stats_serial))

/* Structure for block devices statistics */
struct stats_disk {
	unsigned long long rd_sect	__attribute__ ((aligned (16)));
	unsigned long long wr_sect	__attribute__ ((aligned (16)));
	unsigned long rd_ticks		__attribute__ ((aligned (16)));
	unsigned long wr_ticks		__attribute__ ((aligned (8)));
	unsigned long tot_ticks		__attribute__ ((aligned (8)));
	unsigned long rq_ticks		__attribute__ ((aligned (8)));
	unsigned long nr_ios		__attribute__ ((aligned (8)));
	unsigned int  major		__attribute__ ((aligned (8)));
	unsigned int  minor		__attribute__ ((packed));
};

#define STATS_DISK_SIZE	(sizeof(struct stats_disk))

/* Structure for network interfaces statistics */
struct stats_net_dev {
	unsigned long rx_packets		__attribute__ ((aligned (8)));
	unsigned long tx_packets		__attribute__ ((aligned (8)));
	unsigned long rx_bytes			__attribute__ ((aligned (8)));
	unsigned long tx_bytes			__attribute__ ((aligned (8)));
	unsigned long rx_compressed		__attribute__ ((aligned (8)));
	unsigned long tx_compressed		__attribute__ ((aligned (8)));
	unsigned long multicast			__attribute__ ((aligned (8)));
	char	      interface[MAX_IFACE_LEN]	__attribute__ ((aligned (8)));
};

#define STATS_NET_DEV_SIZE	(sizeof(struct stats_net_dev))

/* Structure for network interface errors statistics */
struct stats_net_edev {
	unsigned long collisions		__attribute__ ((aligned (8)));
	unsigned long rx_errors			__attribute__ ((aligned (8)));
	unsigned long tx_errors			__attribute__ ((aligned (8)));
	unsigned long rx_dropped		__attribute__ ((aligned (8)));
	unsigned long tx_dropped		__attribute__ ((aligned (8)));
	unsigned long rx_fifo_errors		__attribute__ ((aligned (8)));
	unsigned long tx_fifo_errors		__attribute__ ((aligned (8)));
	unsigned long rx_frame_errors		__attribute__ ((aligned (8)));
	unsigned long tx_carrier_errors		__attribute__ ((aligned (8)));
	char	      interface[MAX_IFACE_LEN]	__attribute__ ((aligned (8)));
};

#define STATS_NET_EDEV_SIZE	(sizeof(struct stats_net_edev))

/* Structure for NFS client statistics */
struct stats_net_nfs {
	unsigned int  nfs_rpccnt	__attribute__ ((aligned (4)));
	unsigned int  nfs_rpcretrans	__attribute__ ((packed));
	unsigned int  nfs_readcnt	__attribute__ ((packed));
	unsigned int  nfs_writecnt	__attribute__ ((packed));
	unsigned int  nfs_accesscnt	__attribute__ ((packed));
	unsigned int  nfs_getattcnt	__attribute__ ((packed));
};

#define STATS_NET_NFS_SIZE	(sizeof(struct stats_net_nfs))

/* Structure for NFS server statistics */
struct stats_net_nfsd {
	unsigned int  nfsd_rpccnt	__attribute__ ((aligned (4)));
	unsigned int  nfsd_rpcbad	__attribute__ ((packed));
	unsigned int  nfsd_netcnt	__attribute__ ((packed));
	unsigned int  nfsd_netudpcnt	__attribute__ ((packed));
	unsigned int  nfsd_nettcpcnt	__attribute__ ((packed));
	unsigned int  nfsd_rchits	__attribute__ ((packed));
	unsigned int  nfsd_rcmisses	__attribute__ ((packed));
	unsigned int  nfsd_readcnt	__attribute__ ((packed));
	unsigned int  nfsd_writecnt	__attribute__ ((packed));
	unsigned int  nfsd_accesscnt	__attribute__ ((packed));
	unsigned int  nfsd_getattcnt	__attribute__ ((packed));
};

#define STATS_NET_NFSD_SIZE	(sizeof(struct stats_net_nfsd))

/* Structure for network sockets statistics */
struct stats_net_sock {
	unsigned int  sock_inuse	__attribute__ ((aligned (4)));
	unsigned int  tcp_inuse		__attribute__ ((packed));
	unsigned int  tcp_tw		__attribute__ ((packed));
	unsigned int  udp_inuse		__attribute__ ((packed));
	unsigned int  raw_inuse		__attribute__ ((packed));
	unsigned int  frag_inuse	__attribute__ ((packed));
};

#define STATS_NET_SOCK_SIZE	(sizeof(struct stats_net_sock))


/*
 ***************************************************************************
 * Prototypes for functions used to read system statistics
 ***************************************************************************
 */

extern void
	read_stat_cpu(struct stats_cpu *, int,
		      unsigned long long *, unsigned long long *);
extern void
	read_stat_pcsw(struct stats_pcsw *);
extern void
	read_stat_irq(struct stats_irq *, int);
extern void
	read_loadavg(struct stats_queue *);
extern void
	read_meminfo(struct stats_memory *);
extern unsigned int
	read_vmstat_swap(struct stats_swap *);
extern void
	read_stat_swap(struct stats_swap *);
extern int
	read_vmstat_paging(struct stats_paging *);
extern void
	read_stat_paging(struct stats_paging *);
extern void
	read_diskstats_io(struct stats_io *);
extern void
	read_ppartitions_io(struct stats_io *);
extern void
	read_stat_io(struct stats_io *);
extern void
	read_diskstats_disk(struct stats_disk *, int);
extern void
	read_partitions_disk(struct stats_disk *, int);
extern void
	read_stat_disk(struct stats_disk *, int);
extern void
	read_tty_driver_serial(struct stats_serial *, int);
extern void
	read_kernel_tables(struct stats_ktables *);
extern void
	read_net_dev(struct stats_net_dev *, int);
extern void
	read_net_edev(struct stats_net_edev *, int);
extern void
	read_net_nfs(struct stats_net_nfs *);
extern void
	read_net_nfsd(struct stats_net_nfsd *);
extern void
	read_net_sock(struct stats_net_sock *);
extern void
	read_uptime(unsigned long long *);

/*
 ***************************************************************************
 * Prototypes for functions used to count number of items
 ***************************************************************************
 */

extern int
	get_irq_nr(void);
extern int
	get_serial_nr(void);
extern int
	get_iface_nr(void);
extern int
	get_diskstats_dev_nr(int, int);
extern int
	get_ppartitions_dev_nr(int);
extern unsigned int
	get_disk_io_nr(void);
extern int
	get_disk_nr(unsigned int *);
extern int
	get_cpu_nr(unsigned int);
extern int
	get_irqcpu_nr(int, int);


#endif /* _RD_STATS_H */
