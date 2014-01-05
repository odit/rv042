/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004 Cavium Networks
 */

/**
 * The IOCTL numbers supported on /proc/octeon_perf
 */
#define PROC_PERF_IOCTL_SETUP_COUNTER0      _IOW(0x81, 0, int)
#define PROC_PERF_IOCTL_SETUP_COUNTER1      _IOW(0x81, 1, int)
#define PROC_PERF_IOCTL_SETUP_L2COUNTER0    _IOW(0x81, 2, int)
#define PROC_PERF_IOCTL_SETUP_L2COUNTER1    _IOW(0x81, 3, int)
#define PROC_PERF_IOCTL_SETUP_L2COUNTER2    _IOW(0x81, 4, int)
#define PROC_PERF_IOCTL_SETUP_L2COUNTER3    _IOW(0x81, 5, int)
#define PROC_PERF_IOCTL_READ_COUNTER0       _IOR(0x81, 6, long long)
#define PROC_PERF_IOCTL_READ_COUNTER1       _IOR(0x81, 7, long long)
#define PROC_PERF_IOCTL_READ_L2COUNTER0     _IOR(0x81, 8, long long)
#define PROC_PERF_IOCTL_READ_L2COUNTER1     _IOR(0x81, 9, long long)
#define PROC_PERF_IOCTL_READ_L2COUNTER2     _IOR(0x81, 10, long long)
#define PROC_PERF_IOCTL_READ_L2COUNTER3     _IOR(0x81, 11, long long)

