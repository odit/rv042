/*
 * sysstat - sa_stub.c: Functions used in activity.c
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

extern unsigned int flags;
extern struct record_header record_hdr;

/*
 ***************************************************************************
 * Count number of interrupts that are in /proc/stat file.
 * Truncate the number of different individual interrupts to NR_IRQS.
 *
 * IN:
 * @a	Activity structure.
 *
 * RETURNS:
 * Number of interrupts, including total number of interrupts.
 * Value in [0, NR_IRQS + 1].
 ***************************************************************************
 */
__nr_t wrap_get_irq_nr(struct activity *a)
{
	__nr_t n;

	if ((n = get_irq_nr()) > (a->bitmap_size + 1)) {
		n = a->bitmap_size + 1;
	}
	
	return n;
}

/*
 ***************************************************************************
 * Find number of serial lines that support tx/rx accounting
 * in /proc/tty/driver/serial file.
 *
 * IN:
 * @a	Activity structure.
 *
 * RETURNS:
 * Number of serial lines supporting tx/rx accouting + a pre-allocation
 * constant.
 ***************************************************************************
 */
__nr_t wrap_get_serial_nr(struct activity *a)
{
	__nr_t n = 0;
	
	if ((n = get_serial_nr()) > 0)
		return n + NR_SERIAL_PREALLOC;
	
	return 0;
}

/*
 ***************************************************************************
 * Find number of interfaces (network devices) that are in /proc/net/dev
 * file
 *
 * IN:
 * @a	Activity structure.
 *
 * RETURNS:
 * Number of network interfaces + a pre-allocation constant.
 ***************************************************************************
 */
__nr_t wrap_get_iface_nr(struct activity *a)
{
	__nr_t n = 0;
	
	if ((n = get_iface_nr()) > 0)
		return n + NR_IFACE_PREALLOC;
	
	return 0;
}

/*
 ***************************************************************************
 * Compute number of structures to allocate for CPUs.
 *
 * IN:
 * @a	Activity structure.
 *
 * RETURNS:
 * Number of structures (value in [1, NR_CPUS + 1]).
 * 1 means that there is only one proc and non SMP kernel.
 * 2 means one proc and SMP kernel.
 * Etc.
 ***************************************************************************
 */
__nr_t wrap_get_cpu_nr(struct activity *a)
{
	return (get_cpu_nr(a->bitmap_size) + 1);
}

/*
 ***************************************************************************
 * Get number of devices in /proc/{diskstats,partitions}
 * or number of disk_io entries in /proc/stat.
 * Always done, since disk stats must be read at least for sar -b
 * if not for sar -d.
 *
 * IN:
 * @a	Activity structure.
 *
 * RETURNS:
 * Number of devices + a pre-allocation constant.
 ***************************************************************************
 */
__nr_t wrap_get_disk_nr(struct activity *a)
{
	__nr_t n = 0;
	unsigned int f = 0;
	
	n = get_disk_nr(&f);

	if (f == READ_DISKSTATS) {
		flags |= S_F_HAS_DISKSTATS;
	}
	else if (f == READ_PPARTITIONS) {
		flags |= S_F_HAS_PPARTITIONS;
	}
	
	if (n > 0)
		return n + NR_DISK_PREALLOC;
	
	return 0;
}

/*
 ***************************************************************************
 * Read CPU statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_stat_cpu(struct activity *a)
{
	struct stats_cpu *st_cpu
		= (struct stats_cpu *) a->_buf0;
	
	/* Read CPU statistics */
	read_stat_cpu(st_cpu, a->nr, &(record_hdr.uptime), &(record_hdr.uptime0));
	
	return;
}

/*
 ***************************************************************************
 * Read process (task) creation and context switch statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_stat_pcsw(struct activity *a)
{
	struct stats_pcsw *st_pcsw
		= (struct stats_pcsw *) a->_buf0;

	/* Read process and context switch stats */
	read_stat_pcsw(st_pcsw);
	
	return;
}

/*
 ***************************************************************************
 * Read interrupt statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_stat_irq(struct activity *a)
{
	struct stats_irq *st_irq
		= (struct stats_irq *) a->_buf0;

	/* Read interrupts stats */
	read_stat_irq(st_irq, a->nr);
	
	return;
}

/*
 ***************************************************************************
 * Read queue and load statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_loadavg(struct activity *a)
{
	struct stats_queue *st_queue
		= (struct stats_queue *) a->_buf0;

	/* Read queue and load stats */
	read_loadavg(st_queue);
	
	return;
}

/*
 ***************************************************************************
 * Read memory statistics
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_meminfo(struct activity *a)
{
	struct stats_memory *st_memory
		= (struct stats_memory *) a->_buf0;

	/* Read memory stats */
	read_meminfo(st_memory);
	
	return;
}

/*
 ***************************************************************************
 * Read swapping statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_swap(struct activity *a)
{
	struct stats_swap *st_swap
		= (struct stats_swap *) a->_buf0;

	/* Try to read stats from /proc/vmstat, otherwise from /proc/stat */
	if (!read_vmstat_swap(st_swap)) {
		read_stat_swap(st_swap);
	}
	
	return;
}

/*
 ***************************************************************************
 * Read paging statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_paging(struct activity *a)
{
	struct stats_paging *st_paging
		= (struct stats_paging *) a->_buf0;

	/* Try to read stats from /proc/vmstat, otherwise from /proc/stat */
	if (!read_vmstat_paging(st_paging)) {
		read_stat_paging(st_paging);
	}
	
	return;
}

/*
 ***************************************************************************
 * Read I/O and transfer rates statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_io(struct activity *a)
{
	struct stats_io *st_io
		= (struct stats_io *) a->_buf0;

	/* Try to read stats from /proc/diskstats, /proc/partitions or /proc/stat */
	if (HAS_DISKSTATS(flags)) {
		read_diskstats_io(st_io);
	}
	else if (HAS_PPARTITIONS(flags)) {
		read_ppartitions_io(st_io);
	}
	else {
		read_stat_io(st_io);
	}

	return;
}

/*
 ***************************************************************************
 * Read block devices statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_disk(struct activity *a)
{
	struct stats_disk *st_disk
		= (struct stats_disk *) a->_buf0;

	/* Try to read stats from /proc/diskstats, /proc/partitions or /proc/stat */
	if (HAS_DISKSTATS(flags)) {
		read_diskstats_disk(st_disk, a->nr);
	}
	else if (HAS_PPARTITIONS(flags)) {
		read_partitions_disk(st_disk, a->nr);
	}
	else {
		read_stat_disk(st_disk, a->nr);
	}

	return;
}

/*
 ***************************************************************************
 * Read serial lines statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_tty_driver_serial(struct activity *a)
{
	struct stats_serial *st_serial
		= (struct stats_serial *) a->_buf0;

	/* Read serial lines stats */
	read_tty_driver_serial(st_serial, a->nr);
	
	return;
}

/*
 ***************************************************************************
 * Read kernel tables statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_kernel_tables(struct activity *a)
{
	struct stats_ktables *st_ktables
		= (struct stats_ktables *) a->_buf0;

	/* Read kernel tables stats */
	read_kernel_tables(st_ktables);
	
	return;
}

/*
 ***************************************************************************
 * Read network interfaces statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_dev(struct activity *a)
{
	struct stats_net_dev *st_net_dev
		= (struct stats_net_dev *) a->_buf0;

	/* Read network interfaces stats */
	read_net_dev(st_net_dev, a->nr);
	
	return;
}

/*
 ***************************************************************************
 * Read network interfaces errors statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_edev(struct activity *a)
{
	struct stats_net_edev *st_net_edev
		= (struct stats_net_edev *) a->_buf0;

	/* Read network interfaces errors stats */
	read_net_edev(st_net_edev, a->nr);
	
	return;
}

/*
 ***************************************************************************
 * Read NFS client statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_nfs(struct activity *a)
{
	struct stats_net_nfs *st_net_nfs
		= (struct stats_net_nfs *) a->_buf0;

	/* Read NFS client stats */
	read_net_nfs(st_net_nfs);
	
	return;
}

/*
 ***************************************************************************
 * Read NFS server statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_nfsd(struct activity *a)
{
	struct stats_net_nfsd *st_net_nfsd
		= (struct stats_net_nfsd *) a->_buf0;

	/* Read NFS server stats */
	read_net_nfsd(st_net_nfsd);
	
	return;
}

/*
 ***************************************************************************
 * Read network sockets statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_sock(struct activity *a)
{
	struct stats_net_sock *st_net_sock
		= (struct stats_net_sock *) a->_buf0;

	/* Read network sockets stats */
	read_net_sock(st_net_sock);
	
	return;
}

