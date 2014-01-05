/*
 * Initially based on linux-2.4.20_mvl31-pnx8xx0/drivers/char/serial_pnx8550.c
 *
 * Complete rewrite to drivers/serial/pnx8550_uart.c by
 * Embedded Alley Solutions, source@embeddedalley.com as part of the
 * PNX8550 2.6 port, and then drivers/serial/ip3106_uart.c to work
 * with other Philips SoCs.
 *
 * Existing copyrights from files used to write this driver:
 * Author: Per Hallsmark per.hallsmark@mvista.com
 *
 * and
 *
 * Based on drivers/char/serial.c, by Linus Torvalds, Theodore Ts'o.
 * Copyright (C) 2000 Deep Blue Solutions Ltd.
 *
 */

#include <linux/config.h>

#if defined(CONFIG_SERIAL_IP3106_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_ip3106.h>

#include <asm/io.h>
#include <asm/irq.h>

#include <uart.h>

/* We've been assigned a range on the "Low-density serial ports" major */
#define SERIAL_IP3106_MAJOR	204
#define MINOR_START		5

#define NR_PORTS		2

#define IP3106_ISR_PASS_LIMIT	256

/*
 * Convert from ignore_status_mask or read_status_mask to FIFO
 * and interrupt status bits
 */
#define SM_TO_FIFO(x)	((x) >> 10)
#define SM_TO_ISTAT(x)	((x) & 0x000001ff)
#define FIFO_TO_SM(x)	((x) << 10)
#define ISTAT_TO_SM(x)	((x) & 0x000001ff)

/*
 * This is the size of our serial port register set.
 */
#define UART_PORT_SIZE	0x1000

/*
 * This determines how often we check the modem status signals
 * for any change.  They generally aren't connected to an IRQ
 * so we have to poll them.  We also check immediately before
 * filling the TX fifo incase CTS has been dropped.
 */
#define MCTRL_TIMEOUT	(250*HZ/1000)


extern struct ip3106_port ip3106_ports[];

static inline int serial_in(struct ip3106_port *sport, int offset)
{
	return (__raw_readl(sport->port.membase + offset));
}

static inline void serial_out(struct ip3106_port *sport, int offset, int value)
{
	__raw_writel(value, sport->port.membase + offset);
}

/*
 * Handle any change of modem status signal since we were last called.
 */
static void ip3106_mctrl_check(struct ip3106_port *sport)
{
	unsigned int status, changed;

	status = sport->port.ops->get_mctrl(&sport->port);
	changed = status ^ sport->old_status;

	if (changed == 0)
		return;

	sport->old_status = status;

	if (changed & TIOCM_RI)
		sport->port.icount.rng++;
	if (changed & TIOCM_DSR)
		sport->port.icount.dsr++;
	if (changed & TIOCM_CAR)
		uart_handle_dcd_change(&sport->port, status & TIOCM_CAR);
	if (changed & TIOCM_CTS)
		uart_handle_cts_change(&sport->port, status & TIOCM_CTS);

	wake_up_interruptible(&sport->port.info->delta_msr_wait);
}

/*
 * This is our per-port timeout handler, for checking the
 * modem status signals.
 */
static void ip3106_timeout(unsigned long data)
{
	struct ip3106_port *sport = (struct ip3106_port *)data;
	unsigned long flags;

	if (sport->port.info) {
		spin_lock_irqsave(&sport->port.lock, flags);
		ip3106_mctrl_check(sport);
		spin_unlock_irqrestore(&sport->port.lock, flags);

		mod_timer(&sport->timer, jiffies + MCTRL_TIMEOUT);
	}
}

/*
 * interrupts disabled on entry
 */
static void ip3106_stop_tx(struct uart_port *port, unsigned int tty_stop)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;
	u32 ien;

	/* Disable TX intr */
	ien = serial_in(sport, IP3106_IEN);
	serial_out(sport, IP3106_IEN, ien & ~IP3106_UART_INT_ALLTX);

	/* Clear all pending TX intr */
	serial_out(sport, IP3106_ICLR, IP3106_UART_INT_ALLTX);
}

/*
 * interrupts may not be disabled on entry
 */
static void ip3106_start_tx(struct uart_port *port, unsigned int tty_start)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;
	unsigned long flags;
	u32 ien;

	spin_lock_irqsave(&sport->port.lock, flags);

	/* Clear all pending TX intr */
	serial_out(sport, IP3106_ICLR, IP3106_UART_INT_ALLTX);

	/* Enable TX intr */
	ien = serial_in(sport, IP3106_IEN);
	serial_out(sport, IP3106_IEN, ien | IP3106_UART_INT_ALLTX);

	spin_unlock_irqrestore(&sport->port.lock, flags);
}

/*
 * Interrupts enabled
 */
static void ip3106_stop_rx(struct uart_port *port)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;
	u32 ien;

	/* Disable RX intr */
	ien = serial_in(sport, IP3106_IEN);
	serial_out(sport, IP3106_IEN, ien & ~IP3106_UART_INT_ALLRX);

	/* Clear all pending RX intr */
	serial_out(sport, IP3106_ICLR, IP3106_UART_INT_ALLRX);
}

/*
 * Set the modem control timer to fire immediately.
 */
static void ip3106_enable_ms(struct uart_port *port)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;

	mod_timer(&sport->timer, jiffies);
}

static void
ip3106_rx_chars(struct ip3106_port *sport, struct pt_regs *regs)
{
	struct tty_struct *tty = sport->port.info->tty;
	unsigned int status, ch, flg, ignored = 0;

	status = FIFO_TO_SM(serial_in(sport, IP3106_FIFO)) |
		 ISTAT_TO_SM(serial_in(sport, IP3106_ISTAT));
	while (status & FIFO_TO_SM(IP3106_UART_FIFO_RXFIFO)) {
		ch = serial_in(sport, IP3106_FIFO);

		if (tty->flip.count >= TTY_FLIPBUF_SIZE)
			goto ignore_char;
		sport->port.icount.rx++;

		flg = TTY_NORMAL;

		/*
		 * note that the error handling code is
		 * out of the main execution path
		 */
		if (status & FIFO_TO_SM(IP3106_UART_FIFO_RXFE |
					IP3106_UART_FIFO_RXPAR))
			goto handle_error;

		if (uart_handle_sysrq_char(&sport->port, ch, regs))
			goto ignore_char;

	error_return:
		tty_insert_flip_char(tty, ch, flg);
	ignore_char:
		serial_out(sport, IP3106_LCR, serial_in(sport, IP3106_LCR) |
				IP3106_UART_LCR_RX_NEXT);
		status = FIFO_TO_SM(serial_in(sport, IP3106_FIFO)) |
			 ISTAT_TO_SM(serial_in(sport, IP3106_ISTAT));
	}
 out:
	tty_flip_buffer_push(tty);
	return;

 handle_error:
	if (status & FIFO_TO_SM(IP3106_UART_FIFO_RXPAR))
		sport->port.icount.parity++;
	else if (status & FIFO_TO_SM(IP3106_UART_FIFO_RXFE))
		sport->port.icount.frame++;
	if (status & ISTAT_TO_SM(IP3106_UART_INT_RXOVRN))
		sport->port.icount.overrun++;

	if (status & sport->port.ignore_status_mask) {
		if (++ignored > 100)
			goto out;
		goto ignore_char;
	}

//	status &= sport->port.read_status_mask;

	if (status & FIFO_TO_SM(IP3106_UART_FIFO_RXPAR))
		flg = TTY_PARITY;
	else if (status & FIFO_TO_SM(IP3106_UART_FIFO_RXFE))
		flg = TTY_FRAME;

	if (status & ISTAT_TO_SM(IP3106_UART_INT_RXOVRN)) {
		/*
		 * overrun does *not* affect the character
		 * we read from the FIFO
		 */
		tty_insert_flip_char(tty, ch, flg);
		ch = 0;
		flg = TTY_OVERRUN;
	}
#ifdef SUPPORT_SYSRQ
	sport->port.sysrq = 0;
#endif
	goto error_return;
}

static void ip3106_tx_chars(struct ip3106_port *sport)
{
	struct circ_buf *xmit = &sport->port.info->xmit;

	if (sport->port.x_char) {
		serial_out(sport, IP3106_FIFO, sport->port.x_char);
		sport->port.icount.tx++;
		sport->port.x_char = 0;
		return;
	}

	/*
	 * Check the modem control lines before
	 * transmitting anything.
	 */
	ip3106_mctrl_check(sport);

	if (uart_circ_empty(xmit) || uart_tx_stopped(&sport->port)) {
		ip3106_stop_tx(&sport->port, 0);
		return;
	}

	/*
	 * TX while bytes available
	 */
	while (((serial_in(sport, IP3106_FIFO) &
					IP3106_UART_FIFO_TXFIFO) >> 16) < 16) {
		serial_out(sport, IP3106_FIFO, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		sport->port.icount.tx++;
		if (uart_circ_empty(xmit))
			break;
	}

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&sport->port);

	if (uart_circ_empty(xmit))
		ip3106_stop_tx(&sport->port, 0);
}

static irqreturn_t ip3106_int(int irq, void *dev_id, struct pt_regs *regs)
{
	struct ip3106_port *sport = dev_id;
	unsigned int status;

	spin_lock(&sport->port.lock);
	/* Get the interrupts */
	status  = serial_in(sport, IP3106_ISTAT) & serial_in(sport, IP3106_IEN);

	/* RX Receiver Holding Register Overrun */
	if (status & IP3106_UART_INT_RXOVRN) {
		sport->port.icount.overrun++;
		serial_out(sport, IP3106_ICLR, IP3106_UART_INT_RXOVRN);
	}

	/* RX Frame Error */
	if (status & IP3106_UART_INT_FRERR) {
		sport->port.icount.frame++;
		serial_out(sport, IP3106_ICLR, IP3106_UART_INT_FRERR);
	}

	/* Break signal received */
	if (status & IP3106_UART_INT_BREAK) {
		sport->port.icount.brk++;
		serial_out(sport, IP3106_ICLR, IP3106_UART_INT_BREAK);
	}

	/* RX Parity Error */
	if (status & IP3106_UART_INT_PARITY) {
		sport->port.icount.parity++;
		serial_out(sport, IP3106_ICLR, IP3106_UART_INT_PARITY);
	}

	/* Byte received */
	if (status & IP3106_UART_INT_RX) {
		ip3106_rx_chars(sport, regs);
		serial_out(sport, IP3106_ICLR, IP3106_UART_INT_RX);
	}

	/* TX holding register empty - transmit a byte */
	if (status & IP3106_UART_INT_TX) {
		ip3106_tx_chars(sport);
		serial_out(sport, IP3106_ICLR, IP3106_UART_INT_TX);
	}

	/* TX shift register and holding register empty  */
	if (status & IP3106_UART_INT_EMPTY) {
		serial_out(sport, IP3106_ICLR, IP3106_UART_INT_EMPTY);
	}

	/* Receiver time out */
	if (status & IP3106_UART_INT_RCVTO) {
		serial_out(sport, IP3106_ICLR, IP3106_UART_INT_RCVTO);
	}
	spin_unlock(&sport->port.lock);
	return IRQ_HANDLED;
}

/*
 * Return TIOCSER_TEMT when transmitter is not busy.
 */
static unsigned int ip3106_tx_empty(struct uart_port *port)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;

	return serial_in(sport, IP3106_FIFO) & IP3106_UART_FIFO_TXFIFO_STA ? 0 : TIOCSER_TEMT;
}

static unsigned int ip3106_get_mctrl(struct uart_port *port)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;
	unsigned int mctrl = TIOCM_DSR;
	unsigned int msr;

	/* REVISIT */

	msr = serial_in(sport, IP3106_MCR);

	mctrl |= msr & IP3106_UART_MCR_CTS ? TIOCM_CTS : 0;
	mctrl |= msr & IP3106_UART_MCR_DCD ? TIOCM_CAR : 0;

	return mctrl;
}

static void ip3106_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
#if	0	/* FIXME */
	struct ip3106_port *sport = (struct ip3106_port *)port;
	unsigned int msr;
#endif
}

/*
 * Interrupts always disabled.
 */
static void ip3106_break_ctl(struct uart_port *port, int break_state)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;
	unsigned long flags;
	unsigned int lcr;

	spin_lock_irqsave(&sport->port.lock, flags);
	lcr = serial_in(sport, IP3106_LCR);
	if (break_state == -1)
		lcr |= IP3106_UART_LCR_TXBREAK;
	else
		lcr &= ~IP3106_UART_LCR_TXBREAK;
	serial_out(sport, IP3106_LCR, lcr);
	spin_unlock_irqrestore(&sport->port.lock, flags);
}

static int ip3106_startup(struct uart_port *port)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;
	int retval;

	/*
	 * Allocate the IRQ
	 */
	retval = request_irq(sport->port.irq, ip3106_int, 0,
			     "ip3106-uart", sport);
	if (retval)
		return retval;

	/*
	 * Finally, clear and enable interrupts
	 */

	serial_out(sport, IP3106_ICLR, IP3106_UART_INT_ALLRX |
			     IP3106_UART_INT_ALLTX);

	serial_out(sport, IP3106_IEN, serial_in(sport, IP3106_IEN) |
			    IP3106_UART_INT_ALLRX |
			    IP3106_UART_INT_ALLTX);

	/*
	 * Enable modem status interrupts
	 */
	spin_lock_irq(&sport->port.lock);
	ip3106_enable_ms(&sport->port);
	spin_unlock_irq(&sport->port.lock);

	return 0;
}

static void ip3106_shutdown(struct uart_port *port)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;

	/*
	 * Stop our timer.
	 */
	del_timer_sync(&sport->timer);

	/*
	 * Disable all interrupts, port and break condition.
	 */
	serial_out(sport, IP3106_IEN, 0);

	/*
	 * Reset the Tx and Rx FIFOS
	 */
	serial_out(sport, IP3106_LCR, serial_in(sport, IP3106_LCR) |
			    IP3106_UART_LCR_TX_RST |
			    IP3106_UART_LCR_RX_RST);

	/*
	 * Clear all interrupts
	 */
	serial_out(sport, IP3106_ICLR, IP3106_UART_INT_ALLRX |
			     IP3106_UART_INT_ALLTX);

	/*
	 * Free the interrupt
	 */
	free_irq(sport->port.irq, sport);
}

static void
ip3106_set_termios(struct uart_port *port, struct termios *termios,
		   struct termios *old)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;
	unsigned long flags;
	unsigned int lcr_fcr, old_ien, baud, quot;
	unsigned int old_csize = old ? old->c_cflag & CSIZE : CS8;

	/*
	 * We only support CS7 and CS8.
	 */
	while ((termios->c_cflag & CSIZE) != CS7 &&
	       (termios->c_cflag & CSIZE) != CS8) {
		termios->c_cflag &= ~CSIZE;
		termios->c_cflag |= old_csize;
		old_csize = CS8;
	}

	if ((termios->c_cflag & CSIZE) == CS8)
		lcr_fcr = IP3106_UART_LCR_8BIT;
	else
		lcr_fcr = 0;

	if (termios->c_cflag & CSTOPB)
		lcr_fcr |= IP3106_UART_LCR_2STOPB;
	if (termios->c_cflag & PARENB) {
		lcr_fcr |= IP3106_UART_LCR_PAREN;
		if (!(termios->c_cflag & PARODD))
			lcr_fcr |= IP3106_UART_LCR_PAREVN;
	}

	/*
	 * Ask the core to calculate the divisor for us.
	 */
	baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk/16);
	quot = uart_get_divisor(port, baud);

	spin_lock_irqsave(&sport->port.lock, flags);

#if	0	/* REVISIT */
	sport->port.read_status_mask &= UTSR0_TO_SM(UTSR0_TFS);
	sport->port.read_status_mask |= UTSR1_TO_SM(UTSR1_ROR);
	if (termios->c_iflag & INPCK)
		sport->port.read_status_mask |=
				UTSR1_TO_SM(UTSR1_FRE | UTSR1_PRE);
	if (termios->c_iflag & (BRKINT | PARMRK))
		sport->port.read_status_mask |=
				UTSR0_TO_SM(UTSR0_RBB | UTSR0_REB);

	/*
	 * Characters to ignore
	 */
	sport->port.ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		sport->port.ignore_status_mask |=
				UTSR1_TO_SM(UTSR1_FRE | UTSR1_PRE);
	if (termios->c_iflag & IGNBRK) {
		sport->port.ignore_status_mask |=
				UTSR0_TO_SM(UTSR0_RBB | UTSR0_REB);
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			sport->port.ignore_status_mask |=
				UTSR1_TO_SM(UTSR1_ROR);
	}
#endif

	del_timer_sync(&sport->timer);

	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout(port, termios->c_cflag, baud);

	/*
	 * disable interrupts and drain transmitter
	 */
	old_ien = serial_in(sport, IP3106_IEN);
	serial_out(sport, IP3106_IEN, old_ien & ~(IP3106_UART_INT_ALLTX |
					IP3106_UART_INT_ALLRX));

	while (serial_in(sport, IP3106_FIFO) & IP3106_UART_FIFO_TXFIFO_STA)
		barrier();

	/* then, disable everything */
	serial_out(sport, IP3106_IEN, 0);

	/* Reset the Rx and Tx FIFOs too */
	lcr_fcr |= IP3106_UART_LCR_TX_RST;
	lcr_fcr |= IP3106_UART_LCR_RX_RST;

	/* set the parity, stop bits and data size */
	serial_out(sport, IP3106_LCR, lcr_fcr);

	/* set the baud rate */
	quot -= 1;
	serial_out(sport, IP3106_BAUD, quot);

	serial_out(sport, IP3106_ICLR, -1);

	serial_out(sport, IP3106_IEN, old_ien);

	if (UART_ENABLE_MS(&sport->port, termios->c_cflag))
		ip3106_enable_ms(&sport->port);

	spin_unlock_irqrestore(&sport->port.lock, flags);
}

static const char *ip3106_type(struct uart_port *port)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;

	return sport->port.type == PORT_IP3106 ? "IP3106" : NULL;
}

/*
 * Release the memory region(s) being used by 'port'.
 */
static void ip3106_release_port(struct uart_port *port)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;

	release_mem_region(sport->port.mapbase, UART_PORT_SIZE);
}

/*
 * Request the memory region(s) being used by 'port'.
 */
static int ip3106_request_port(struct uart_port *port)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;

	return request_mem_region(sport->port.mapbase, UART_PORT_SIZE,
			"ip3106-uart") != NULL ? 0 : -EBUSY;
}

/*
 * Configure/autoconfigure the port.
 */
static void ip3106_config_port(struct uart_port *port, int flags)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;

	if (flags & UART_CONFIG_TYPE &&
	    ip3106_request_port(&sport->port) == 0)
		sport->port.type = PORT_IP3106;
}

/*
 * Verify the new serial_struct (for TIOCSSERIAL).
 * The only change we allow are to the flags and type, and
 * even then only between PORT_IP3106 and PORT_UNKNOWN
 */
static int
ip3106_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	struct ip3106_port *sport = (struct ip3106_port *)port;
	int ret = 0;

	if (ser->type != PORT_UNKNOWN && ser->type != PORT_IP3106)
		ret = -EINVAL;
	if (sport->port.irq != ser->irq)
		ret = -EINVAL;
	if (ser->io_type != SERIAL_IO_MEM)
		ret = -EINVAL;
	if (sport->port.uartclk / 16 != ser->baud_base)
		ret = -EINVAL;
	if ((void *)sport->port.mapbase != ser->iomem_base)
		ret = -EINVAL;
	if (sport->port.iobase != ser->port)
		ret = -EINVAL;
	if (ser->hub6 != 0)
		ret = -EINVAL;
	return ret;
}

struct uart_ops ip3106_pops = {
	.tx_empty	= ip3106_tx_empty,
	.set_mctrl	= ip3106_set_mctrl,
	.get_mctrl	= ip3106_get_mctrl,
	.stop_tx	= ip3106_stop_tx,
	.start_tx	= ip3106_start_tx,
	.stop_rx	= ip3106_stop_rx,
	.enable_ms	= ip3106_enable_ms,
	.break_ctl	= ip3106_break_ctl,
	.startup	= ip3106_startup,
	.shutdown	= ip3106_shutdown,
	.set_termios	= ip3106_set_termios,
	.type		= ip3106_type,
	.release_port	= ip3106_release_port,
	.request_port	= ip3106_request_port,
	.config_port	= ip3106_config_port,
	.verify_port	= ip3106_verify_port,
};


/*
 * Setup the IP3106 serial ports.
 *
 * Note also that we support "console=ttySx" where "x" is either 0 or 1.
 */
static void __init ip3106_init_ports(void)
{
	static int first = 1;
	int i;

	if (!first)
		return;
	first = 0;

	for (i = 0; i < NR_PORTS; i++) {
		init_timer(&ip3106_ports[i].timer);
		ip3106_ports[i].timer.function = ip3106_timeout;
		ip3106_ports[i].timer.data     = (unsigned long)&ip3106_ports[i];
	}
}

#ifdef CONFIG_SERIAL_IP3106_CONSOLE

/*
 * Interrupts are disabled on entering
 */
static void
ip3106_console_write(struct console *co, const char *s, unsigned int count)
{
	struct ip3106_port *sport = &ip3106_ports[co->index];
	unsigned int old_ien, status, i;

	/*
	 *	First, save IEN and then disable interrupts
	 */
	old_ien = serial_in(sport, IP3106_IEN);
	serial_out(sport, IP3106_IEN, old_ien & ~(IP3106_UART_INT_ALLTX |
					IP3106_UART_INT_ALLRX));

	/*
	 *	Now, do each character
	 */
	for (i = 0; i < count; i++) {
		do {
			/* Wait for UART_TX register to empty */
			status = serial_in(sport, IP3106_FIFO);
		} while (status & IP3106_UART_FIFO_TXFIFO);
		serial_out(sport, IP3106_FIFO, s[i]);
		if (s[i] == '\n') {
			do {
				status = serial_in(sport, IP3106_FIFO);
			} while (status & IP3106_UART_FIFO_TXFIFO);
			serial_out(sport, IP3106_FIFO, '\r');
		}
	}

	/*
	 *	Finally, wait for transmitter to become empty
	 *	and restore IEN
	 */
	do {
		/* Wait for UART_TX register to empty */
		status = serial_in(sport, IP3106_FIFO);
	} while (status & IP3106_UART_FIFO_TXFIFO);

	/* Clear TX and EMPTY interrupt */
	serial_out(sport, IP3106_ICLR, IP3106_UART_INT_TX |
			     IP3106_UART_INT_EMPTY);

	serial_out(sport, IP3106_IEN, old_ien);
}

static int __init
ip3106_console_setup(struct console *co, char *options)
{
	struct ip3106_port *sport;
	int baud = 38400;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	/*
	 * Check whether an invalid uart number has been specified, and
	 * if so, search for the first available port that does have
	 * console support.
	 */
	if (co->index == -1 || co->index >= NR_PORTS)
		co->index = 0;
	sport = &ip3106_ports[co->index];

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);

	return uart_set_options(&sport->port, co, baud, parity, bits, flow);
}

extern struct uart_driver ip3106_reg;
static struct console ip3106_console = {
	.name		= "ttyS",
	.write		= ip3106_console_write,
	.device		= uart_console_device,
	.setup		= ip3106_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &ip3106_reg,
};

static int __init ip3106_rs_console_init(void)
{
	ip3106_init_ports();
	register_console(&ip3106_console);
	return 0;
}
console_initcall(ip3106_rs_console_init);

#define IP3106_CONSOLE	&ip3106_console
#else
#define IP3106_CONSOLE	NULL
#endif

static struct uart_driver ip3106_reg = {
	.owner			= THIS_MODULE,
	.driver_name		= "ttyS",
	.dev_name		= "ttyS",
	.devfs_name		= "tts/",
	.major			= SERIAL_IP3106_MAJOR,
	.minor			= MINOR_START,
	.nr			= NR_PORTS,
	.cons			= IP3106_CONSOLE,
};

static int ip3106_serial_suspend(struct device *_dev, u32 state, u32 level)
{
	struct ip3106_port *sport = dev_get_drvdata(_dev);

	if (sport && level == SUSPEND_DISABLE)
		uart_suspend_port(&ip3106_reg, &sport->port);

	return 0;
}

static int ip3106_serial_resume(struct device *_dev, u32 level)
{
	struct ip3106_port *sport = dev_get_drvdata(_dev);

	if (sport && level == RESUME_ENABLE)
		uart_resume_port(&ip3106_reg, &sport->port);

	return 0;
}

static int ip3106_serial_probe(struct device *_dev)
{
	struct platform_device *dev = to_platform_device(_dev);
	struct resource *res = dev->resource;
	int i;

	for (i = 0; i < dev->num_resources; i++, res++) {
		if (!(res->flags & IORESOURCE_MEM))
			continue;

		for (i = 0; i < NR_PORTS; i++) {
			if (ip3106_ports[i].port.mapbase != res->start)
				continue;

			ip3106_ports[i].port.dev = _dev;
			uart_add_one_port(&ip3106_reg, &ip3106_ports[i].port);
			dev_set_drvdata(_dev, &ip3106_ports[i]);
			break;
		}
	}

	return 0;
}

static int ip3106_serial_remove(struct device *_dev)
{
	struct ip3106_port *sport = dev_get_drvdata(_dev);

	dev_set_drvdata(_dev, NULL);

	if (sport)
		uart_remove_one_port(&ip3106_reg, &sport->port);

	return 0;
}

static struct device_driver ip3106_serial_driver = {
	.name		= "ip3106-uart",
	.bus		= &platform_bus_type,
	.probe		= ip3106_serial_probe,
	.remove		= ip3106_serial_remove,
	.suspend	= ip3106_serial_suspend,
	.resume		= ip3106_serial_resume,
};

static int __init ip3106_serial_init(void)
{
	int ret;

	printk(KERN_INFO "Serial: IP3106 driver $Revision: 2 $\n");

	ip3106_init_ports();

	ret = uart_register_driver(&ip3106_reg);
	if (ret == 0) {
		ret = driver_register(&ip3106_serial_driver);
		if (ret)
			uart_unregister_driver(&ip3106_reg);
	}
	return ret;
}

static void __exit ip3106_serial_exit(void)
{
	driver_unregister(&ip3106_serial_driver);
	uart_unregister_driver(&ip3106_reg);
}

module_init(ip3106_serial_init);
module_exit(ip3106_serial_exit);

MODULE_AUTHOR("Embedded Alley Solutions, Inc.");
MODULE_DESCRIPTION("IP3106 generic serial port driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_CHARDEV_MAJOR(SERIAL_IP3106_MAJOR);
