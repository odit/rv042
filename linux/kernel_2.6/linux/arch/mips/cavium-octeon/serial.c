/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004, 2005 Cavium Networks
 */
#include <linux/console.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <asm/time.h>
#include <linux/serial_core.h>
#include <asm/gdb-stub.h>
#include "hal.h"

extern int serial8250_register_port(struct uart_port *);

#ifdef CONFIG_GDB_CONSOLE
    #define DEBUG_UART 0
#else
    #define DEBUG_UART 1
#endif

#ifdef CONFIG_KGDB

extern void breakpoint(void);

char getDebugChar(void)
{
    uint64_t lsrval;

    octeon_write_lcd("kgdb");

    /* Spin until data is available */
    do
    {
	    lsrval = octeon_read_csr(OCTEON_MIO_UARTX_LSR(DEBUG_UART));
    } while ((lsrval & 0x1) == 0);

    octeon_write_lcd("");

    /* Read and return the data */
    return octeon_read_csr(OCTEON_MIO_UARTX_RBR(DEBUG_UART));
}

void putDebugChar(char ch)
{
    uint64_t lsrval;

    /* Spin until there is room */
    do
    {
        lsrval = octeon_read_csr(OCTEON_MIO_UARTX_LSR(DEBUG_UART));
    }
    while ((lsrval & 0x20) == 0);

    /* Write the byte */
    octeon_write_csr(OCTEON_MIO_UARTX_THR(DEBUG_UART), ch);
}

#endif

#if defined(CONFIG_KGDB) || defined(CONFIG_CAVIUM_GDB)

static irqreturn_t interruptDebugChar(int cpl, void *dev_id, struct pt_regs *regs)
{
    uint64_t lsrval;
    lsrval = octeon_read_csr(OCTEON_MIO_UARTX_LSR(1));
    if (lsrval & 1)
    {
        #ifdef CONFIG_KGDB
            putDebugChar(getDebugChar());
            set_async_breakpoint(&regs->cp0_epc);
        #else
            register uint64_t tmp;
            /* Pulse MCD0 signal on Ctrl-C to stop all the cores. Also
                set the MCD0 to be not masked by this core so we know
                the signal is received by someone */
            octeon_write_lcd("brk");
            asm volatile (
                "dmfc0 %0, $22\n"
                "ori   %0, %0, 0x10\n"
                "dmtc0 %0, $22\n"
                : "=r" (tmp));
            octeon_write_lcd("");
        #endif
        return IRQ_HANDLED;
    }
    return IRQ_NONE;
}

#endif

static int octeon_serial_init(void)
{
    int port;

    #if defined(CONFIG_KGDB) || defined(CONFIG_CAVIUM_GDB)
        const int max_port=1;
        uint64_t ier;
    #else
        /* Change the following to "2" to have both serial ports available in
            Linux. The second port isn't enabled by default because a simple
            exec application might be running on another core. Linux control
            of the second uart breaks the simple exec debugger interface
            through the second uart. */
        const int max_port=1 + octeon_get_boot_uart();  /* Init second serial port if console on uart 1 */
    #endif

    struct uart_port octeon_port;
    memset(&octeon_port, 0, sizeof(octeon_port));
    octeon_port.flags           = ASYNC_SKIP_TEST | UPF_SHARE_IRQ;
    octeon_port.iotype          = UPIO_MEM;
    octeon_port.regshift        = 3;                  /* I/O addresses are every 8 bytes */
    octeon_port.uartclk         = mips_hpt_frequency; /* Clock rate of the chip */
    octeon_port.fifosize        = 64;

    for (port=0; port< max_port; port++)
    {
        octeon_port.mapbase = 0x0001180000000800ull + (1024 * port);
        octeon_port.membase = octeon_phys_to_ptr(octeon_port.mapbase);
        if ((current_cpu_data.cputype != CPU_CAVIUM_CN38XX) || /* Only CN38XXp{1,2} has errata with uart interrupt */
            ((current_cpu_data.processor_id & 0xff) > 1))
            octeon_port.irq = 42+port;
        serial8250_register_port(&octeon_port);
    }

    #if defined(CONFIG_KGDB) || defined(CONFIG_CAVIUM_GDB)
        request_irq(42 + DEBUG_UART, interruptDebugChar, SA_SHIRQ, "KGDB", interruptDebugChar);

        /* Enable uart1 interrupts for debugger Control-C processing */
        ier = octeon_read_csr(OCTEON_MIO_UARTX_IER(DEBUG_UART));
        octeon_write_csr(OCTEON_MIO_UARTX_IER(DEBUG_UART), ier|1);
    #endif
    return 0;
}

late_initcall(octeon_serial_init);

