/*
 * prf_stats.c: Funtions used by sadf to display statistics
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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "sa.h"
#include "ioconf.h"
#include "prf_stats.h"

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

static char *seps[] =  {"\t", ";"};

extern unsigned int flags;

/*
 ***************************************************************************
 * cons() -
 *   encapsulate a pair of ints or pair of char * into a static Cons and
 *   return a pointer to it.
 *
 * given:   t - type of Cons {iv, sv}
 *	    arg1 - unsigned long int (if iv), char * (if sv) to become
 *		   element 'a'
 *	    arg2 - unsigned long int (if iv), char * (if sv) to become
 *		   element 'b'
 *
 * does:    load a static Cons with values using the t parameter to
 *	    guide pulling values from the arglist
 *
 * return:  the address of it's static Cons.  If you need to keep
 *	    the contents of this Cons, copy it somewhere before calling
 *	    cons() against to avoid overwrite.
 *	    ie. don't do this:  f( cons( iv, i, j ), cons( iv, a, b ) );
 ***************************************************************************
 */
static Cons *cons(tcons t, ...)
{
	va_list ap;
	static Cons c;

	c.t = t;

	va_start(ap, t);
	if (t == iv) {
		c.a.i = va_arg(ap, unsigned long int);
		c.b.i = va_arg(ap, unsigned long int);
	}
	else {
		c.a.s = va_arg(ap, char *);
		c.b.s = va_arg(ap, char *);
	}
	va_end(ap);
	return(&c);
}

/*
 ***************************************************************************
 * render():
 *
 * given:    isdb - flag, true if db printing, false if ppc printing
 *	     pre  - prefix string for output entries
 *	     rflags - PT_.... rendering flags
 *	     pptxt - printf-format text required for ppc output (may be null)
 *	     dbtxt - printf-format text required for db output (may be null)
 *	     mid - pptxt/dbtxt format args as a Cons.
 *	     luval - %lu printable arg (PT_USEINT must be set)
 *	     dval  - %.2f printable arg (used unless PT_USEINT is set)
 *
 * does:     print [pre<sep>]([dbtxt,arg,arg<sep>]|[pptxt,arg,arg<sep>]) \
 *                     (luval|dval)(<sep>|\n)
 *
 * return:   void.
 ***************************************************************************
 */
static void render(int isdb, char *pre, int rflags, const char *pptxt,
		   const char *dbtxt, Cons *mid, unsigned long int luval,
		   double dval)
{
	static int newline = 1;
	const char *txt[]  = {pptxt, dbtxt};

	/* Start a new line? */
	if (newline && !DISPLAY_HORIZONTALLY(flags)) {
		printf("%s", pre);
	}

	/* Terminate this one ? ppc always gets a newline */
	newline = ((rflags & PT_NEWLIN) || !isdb);

	if (txt[isdb]) {
		/* pp/dbtxt? */

		printf("%s", seps[isdb]);	/* Only if something actually gets printed */

		if (mid) {
			/* Got format args? */
			switch(mid->t) {
			case iv:
				printf(txt[isdb], mid->a.i, mid->b.i);
				break;
			case sv:
				printf(txt[isdb], mid->a.s, mid->b.s);
				break;
			}
		}
		else {
			printf(txt[isdb]);	/* No args */
		}
	}

	if (rflags & PT_USEINT) {
		printf("%s%lu", seps[isdb], luval);
	}
	else {
		printf("%s%.2f", seps[isdb], dval);
	}
	if (newline) {
		printf("\n");
	}
}

/*
 ***************************************************************************
 * Display CPU statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @g_itv	Interval of time in jiffies multiplied by the number
 *		of processors.
 ***************************************************************************
 */
__print_funct_t render_cpu_stats(struct activity *a, int isdb, char *pre,
				 int curr, unsigned long long g_itv)
{
	int i;
	struct stats_cpu *scc, *scp;
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);

	for (i = 0; (i < a->nr) && (i < a->bitmap_size + 1); i++) {
		
		scc = (struct stats_cpu *) ((char *) a->buf[curr]  + i * a->msize);
		scp = (struct stats_cpu *) ((char *) a->buf[!curr] + i * a->msize);

		/* Should current CPU (including CPU "all") be displayed? */
		if (a->bitmap[i >> 3] & (1 << (i & 0x07))) {
			
			if (!i) {
				/* This is CPU "all" */
				if (DISPLAY_CPU_DEF(a->opt_flags)) {
					render(isdb, pre,
					       PT_NOFLAG,	/* that's zero but you know what it means */
					       "all\t%%user",	/* all ppctext is used as format, thus '%%' */
					       "-1",		/* look! dbtext */
					       NULL,		/* no args */
					       NOVAL,		/* another 0, named for readability */
					       ll_sp_value(scp->cpu_user, scc->cpu_user, g_itv));
				}
				else if (DISPLAY_CPU_ALL(a->opt_flags)) {
					render(isdb, pre, PT_NOFLAG,
					       "all\t%%usr", "-1", NULL,
					       NOVAL,
					       ll_sp_value(scp->cpu_user - scp->cpu_guest,
							   scc->cpu_user - scc->cpu_guest,
							   g_itv));
				}
				
				render(isdb, pre, PT_NOFLAG,
				       "all\t%%nice", NULL, NULL,
				       NOVAL,
				       ll_sp_value(scp->cpu_nice, scc->cpu_nice, g_itv));

				if (DISPLAY_CPU_DEF(a->opt_flags)) {
					render(isdb, pre, PT_NOFLAG,
					       "all\t%%system", NULL, NULL,
					       NOVAL,
					       ll_sp_value(scp->cpu_sys + scp->cpu_hardirq + scp->cpu_softirq,
							   scc->cpu_sys + scc->cpu_hardirq + scc->cpu_softirq,
							   g_itv));
				}
				else if (DISPLAY_CPU_ALL(a->opt_flags)) {
					render(isdb, pre, PT_NOFLAG,
					       "all\t%%sys", NULL, NULL,
					       NOVAL,
					       ll_sp_value(scp->cpu_sys, scc->cpu_sys, g_itv));
				}
				
				render(isdb, pre, PT_NOFLAG,
				       "all\t%%iowait", NULL, NULL,
				       NOVAL,
				       ll_sp_value(scp->cpu_iowait, scc->cpu_iowait, g_itv));

				render(isdb, pre, PT_NOFLAG,
				       "all\t%%steal", NULL, NULL,
				       NOVAL,
				       ll_sp_value(scp->cpu_steal, scc->cpu_steal, g_itv));

				if (DISPLAY_CPU_ALL(a->opt_flags)) {
					render(isdb, pre, PT_NOFLAG,
					       "all\t%%irq", NULL, NULL,
					       NOVAL,
					       ll_sp_value(scp->cpu_hardirq, scc->cpu_hardirq, g_itv));

					render(isdb, pre, PT_NOFLAG,
					       "all\t%%soft", NULL, NULL,
					       NOVAL,
					       ll_sp_value(scp->cpu_softirq, scc->cpu_softirq, g_itv));

					render(isdb, pre, PT_NOFLAG,
					       "all\t%%guest", NULL, NULL,
					       NOVAL,
					       ll_sp_value(scp->cpu_guest, scc->cpu_guest, g_itv));
				}

				render(isdb, pre, pt_newlin,
				       "all\t%%idle", NULL, NULL,
				       NOVAL,
				       (scc->cpu_idle < scp->cpu_idle) ?
				       0.0 :
				       ll_sp_value(scp->cpu_idle, scc->cpu_idle, g_itv));
			}
			else {
				/* Recalculate itv for current proc */
				g_itv = get_per_cpu_interval(scc, scp);

				if (DISPLAY_CPU_DEF(a->opt_flags)) {
					render(isdb, pre, PT_NOFLAG,
					       "cpu%d\t%%user",		/* ppc text with formatting */
					       "%d",			/* db text with format char */
					       cons(iv, i - 1, NOVAL),	/* how we pass format args  */
					       NOVAL,
					       !g_itv ?
					       0.0 :			/* CPU is offline */
					       ll_sp_value(scp->cpu_user, scc->cpu_user, g_itv));
				}
				else if (DISPLAY_CPU_ALL(a->opt_flags)) {
					render(isdb, pre, PT_NOFLAG,
					       "cpu%d\t%%usr", "%d", cons(iv, i - 1, NOVAL),
					       NOVAL,
					       !g_itv ?
					       0.0 :			/* CPU is offline */
					       ll_sp_value(scp->cpu_user - scp->cpu_guest,
							   scc->cpu_user - scc->cpu_guest, g_itv));
				}
				
				render(isdb, pre, PT_NOFLAG,
				       "cpu%d\t%%nice", NULL, cons(iv, i - 1, NOVAL),
				       NOVAL,
				       !g_itv ?
				       0.0 :
				       ll_sp_value(scp->cpu_nice, scc->cpu_nice, g_itv));

				if (DISPLAY_CPU_DEF(a->opt_flags)) {
					render(isdb, pre, PT_NOFLAG,
					       "cpu%d\t%%system", NULL, cons(iv, i - 1, NOVAL),
					       NOVAL,
					       !g_itv ?
					       0.0 :
					       ll_sp_value(scp->cpu_sys + scp->cpu_hardirq + scp->cpu_softirq,
							   scc->cpu_sys + scc->cpu_hardirq + scc->cpu_softirq,
							   g_itv));
				}
				else if (DISPLAY_CPU_ALL(a->opt_flags)) {
					render(isdb, pre, PT_NOFLAG,
					       "cpu%d\t%%sys", NULL, cons(iv, i - 1, NOVAL),
					       NOVAL,
					       !g_itv ?
					       0.0 :
					       ll_sp_value(scp->cpu_sys, scc->cpu_sys, g_itv));
				}
				
				render(isdb, pre, PT_NOFLAG,
				       "cpu%d\t%%iowait", NULL, cons(iv, i - 1, NOVAL),
				       NOVAL,
				       !g_itv ?
				       0.0 :
				       ll_sp_value(scp->cpu_iowait, scc->cpu_iowait, g_itv));

				render(isdb, pre, PT_NOFLAG,
				       "cpu%d\t%%steal", NULL, cons(iv, i - 1, NOVAL),
				       NOVAL,
				       !g_itv ?
				       0.0 :
				       ll_sp_value(scp->cpu_steal, scc->cpu_steal, g_itv));

				if (DISPLAY_CPU_ALL(a->opt_flags)) {
					render(isdb, pre, PT_NOFLAG,
					       "cpu%d\t%%irq", NULL, cons(iv, i - 1, NOVAL),
					       NOVAL,
					       !g_itv ?
					       0.0 :
					       ll_sp_value(scp->cpu_hardirq, scc->cpu_hardirq, g_itv));

					render(isdb, pre, PT_NOFLAG,
					       "cpu%d\t%%soft", NULL, cons(iv, i - 1, NOVAL),
					       NOVAL,
					       !g_itv ?
					       0.0 :
					       ll_sp_value(scp->cpu_softirq, scc->cpu_softirq, g_itv));
					
					render(isdb, pre, PT_NOFLAG,
					       "cpu%d\t%%guest", NULL, cons(iv, i - 1, NOVAL),
					       NOVAL,
					       !g_itv ?
					       0.0 :
					       ll_sp_value(scp->cpu_guest, scc->cpu_guest, g_itv));
				}
				
				if (!g_itv) {
					/* CPU is offline */
					render(isdb, pre, pt_newlin,
					       "cpu%d\t%%idle", NULL, cons(iv, i - 1, NOVAL),
					       NOVAL,
					       0.0);
				}
				else {
					render(isdb, pre, pt_newlin,
					       "cpu%d\t%%idle", NULL, cons(iv, i - 1, NOVAL),
					       NOVAL,
					       (scc->cpu_idle < scp->cpu_idle) ?
					       0.0 :
					       ll_sp_value(scp->cpu_idle, scc->cpu_idle, g_itv));
				}
			}
		}
	}
}

/*
 ***************************************************************************
 * Display task creation and context switch statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_pcsw_stats(struct activity *a, int isdb, char *pre,
				  int curr, unsigned long long itv)
{
	struct stats_pcsw
		*spc = (struct stats_pcsw *) a->buf[curr],
		*spp = (struct stats_pcsw *) a->buf[!curr];
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);
	
	/* The first one as an example */
	render(isdb,		/* db/ppc flag */
	       pre,		/* the preformatted line leader */
	       PT_NOFLAG,	/* is this the end of a db line? */
	       "-\tproc/s",	/* ppc text */
	       NULL,		/* db text */
	       NULL,		/* db/ppc text format args (Cons *) */
	       NOVAL,		/* %lu value (unused unless PT_USEINT) */
	       /* and %.2f value, used unless PT_USEINT */
	       S_VALUE(spp->processes, spc->processes, itv));

	render(isdb, pre, pt_newlin,
	       "-\tcswch/s", NULL, NULL,
	       NOVAL,
	       ll_s_value(spp->context_switch, spc->context_switch, itv));
}

/*
 ***************************************************************************
 * Display interrupts statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_irq_stats(struct activity *a, int isdb, char *pre,
				 int curr, unsigned long long itv)
{
	int i;
	struct stats_irq *sic, *sip;
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);
	
	for (i = 0; (i < a->nr) && (i < a->bitmap_size + 1); i++) {

		sic = (struct stats_irq *) ((char *) a->buf[curr]  + i * a->msize);
		sip = (struct stats_irq *) ((char *) a->buf[!curr] + i * a->msize);
		
		/* Should current interrupt (including int "sum") be displayed? */
		if (a->bitmap[i >> 3] & (1 << (i & 0x07))) {
			
			/* Yes: Display it */
			if (!i) {
				/* This is interrupt "sum" */
				render(isdb, pre, pt_newlin,
				       "sum\tintr/s", "-1", NULL,
				       NOVAL,
				       ll_s_value(sip->irq_nr, sic->irq_nr, itv));
			}
			else {
				render(isdb, pre, pt_newlin,
				       "i%03d\tintr/s", "%d", cons(iv, i - 1, NOVAL),
				       NOVAL,
				       ll_s_value(sip->irq_nr, sic->irq_nr, itv));
			}
		}
	}
}

/*
 ***************************************************************************
 * Display swapping statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_swap_stats(struct activity *a, int isdb, char *pre,
				  int curr, unsigned long long itv)
{
	struct stats_swap
		*ssc = (struct stats_swap *) a->buf[curr],
		*ssp = (struct stats_swap *) a->buf[!curr];
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);
	
	render(isdb, pre, PT_NOFLAG,
	       "-\tpswpin/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(ssp->pswpin, ssc->pswpin, itv));
	render(isdb, pre, pt_newlin,
	       "-\tpswpout/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(ssp->pswpout, ssc->pswpout, itv));
}

/*
 ***************************************************************************
 * Display paging statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_paging_stats(struct activity *a, int isdb, char *pre,
				    int curr, unsigned long long itv)
{
	struct stats_paging
		*spc = (struct stats_paging *) a->buf[curr],
		*spp = (struct stats_paging *) a->buf[!curr];
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);

	render(isdb, pre, PT_NOFLAG,
	       "-\tpgpgin/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(spp->pgpgin, spc->pgpgin, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tpgpgout/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(spp->pgpgout, spc->pgpgout, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tfault/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(spp->pgfault, spc->pgfault, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tmajflt/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(spp->pgmajfault, spc->pgmajfault, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tpgfree/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(spp->pgfree, spc->pgfree, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tpgscank/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(spp->pgscan_kswapd, spc->pgscan_kswapd, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tpgscand/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(spp->pgscan_direct, spc->pgscan_direct, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tpgsteal/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(spp->pgsteal, spc->pgsteal, itv));

	render(isdb, pre, pt_newlin,
	       "-\t%%vmeff", NULL, NULL,
	       NOVAL,
	       (spc->pgscan_kswapd + spc->pgscan_direct -
		spp->pgscan_kswapd - spp->pgscan_direct) ?
	       SP_VALUE(spp->pgsteal, spc->pgsteal,
			spc->pgscan_kswapd + spc->pgscan_direct -
			spp->pgscan_kswapd - spp->pgscan_direct) : 0.0);
}

/*
 ***************************************************************************
 * Display I/O and transfer rate statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_io_stats(struct activity *a, int isdb, char *pre,
				int curr, unsigned long long itv)
{
	struct stats_io
		*sic = (struct stats_io *) a->buf[curr],
		*sip = (struct stats_io *) a->buf[!curr];
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);

	render(isdb, pre, PT_NOFLAG,
	       "-\ttps", NULL, NULL,
	       NOVAL,
	       S_VALUE(sip->dk_drive, sic->dk_drive, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\trtps", NULL, NULL,
	       NOVAL,
	       S_VALUE(sip->dk_drive_rio, sic->dk_drive_rio, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\twtps", NULL, NULL,
	       NOVAL,
	       S_VALUE(sip->dk_drive_wio, sic->dk_drive_wio, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tbread/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(sip->dk_drive_rblk, sic->dk_drive_rblk, itv));

	render(isdb, pre, pt_newlin,
	       "-\tbwrtn/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(sip->dk_drive_wblk, sic->dk_drive_wblk, itv));
}

/*
 ***************************************************************************
 * Display memory and swap statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_memory_stats(struct activity *a, int isdb, char *pre,
				    int curr, unsigned long long itv)
{
	struct stats_memory
		*smc = (struct stats_memory *) a->buf[curr],
		*smp = (struct stats_memory *) a->buf[!curr];
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);
		
	if (DISPLAY_MEMORY(a->opt_flags)) {

		render(isdb, pre, PT_NOFLAG,
		       "-\tfrmpg/s", NULL, NULL,
		       NOVAL,
		       S_VALUE((double) KB_TO_PG(smp->frmkb),
			       (double) KB_TO_PG(smc->frmkb), itv));

		render(isdb, pre, PT_NOFLAG,
		       "-\tbufpg/s", NULL, NULL,
		       NOVAL,
		       S_VALUE((double) KB_TO_PG(smp->bufkb),
			       (double) KB_TO_PG(smc->bufkb), itv));

		render(isdb, pre, pt_newlin,
		       "-\tcampg/s", NULL, NULL,
		       NOVAL,
		       S_VALUE((double) KB_TO_PG(smp->camkb),
			       (double) KB_TO_PG(smc->camkb), itv));
	}

	if (DISPLAY_MEM_AMT(a->opt_flags)) {

		render(isdb, pre, PT_USEINT,
		       "-\tkbmemfree", NULL, NULL,
		       smc->frmkb, DNOVAL);

		render(isdb, pre, PT_USEINT,
		       "-\tkbmemused", NULL, NULL,
		       smc->tlmkb - smc->frmkb, DNOVAL);

		render(isdb, pre, PT_NOFLAG,
		       "-\t%%memused", NULL, NULL, NOVAL,
		       smc->tlmkb ?
		       SP_VALUE(smc->frmkb, smc->tlmkb, smc->tlmkb) :
		       0.0);

		render(isdb, pre, PT_USEINT,
		       "-\tkbbuffers", NULL, NULL,
		       smc->bufkb, DNOVAL);

		render(isdb, pre, PT_USEINT,
		       "-\tkbcached", NULL, NULL,
		       smc->camkb, DNOVAL);

		render(isdb, pre, PT_USEINT,
		       "-\tkbcommit", NULL, NULL,
		       smc->comkb, DNOVAL);

		render(isdb, pre, pt_newlin,
		       "-\t%%commit", NULL, NULL, NOVAL,
		       (smc->tlmkb + smc->tlskb) ?
		       SP_VALUE(0, smc->comkb, smc->tlmkb + smc->tlskb) :
		       0.0);
	}
	
	if (DISPLAY_SWAP(a->opt_flags)) {

		render(isdb, pre, PT_USEINT,
		       "-\tkbswpfree", NULL, NULL,
		       smc->frskb, DNOVAL);

		render(isdb, pre, PT_USEINT,
		       "-\tkbswpused", NULL, NULL,
		       smc->tlskb - smc->frskb, DNOVAL);

		render(isdb, pre, PT_NOFLAG,
		       "-\t%%swpused", NULL, NULL, NOVAL,
		       smc->tlskb ?
		       SP_VALUE(smc->frskb, smc->tlskb, smc->tlskb) :
		       0.0);

		render(isdb, pre, PT_USEINT,
		       "-\tkbswpcad", NULL, NULL,
		       smc->caskb, DNOVAL);

		render(isdb, pre, pt_newlin,
		       "-\t%%swpcad", NULL, NULL, NOVAL,
		       (smc->tlskb - smc->frskb) ?
		       SP_VALUE(0, smc->caskb, smc->tlskb - smc->frskb) :
		       0.0);
	}
}

/*
 ***************************************************************************
 * Display kernel tables statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_ktables_stats(struct activity *a, int isdb, char *pre,
				     int curr, unsigned long long itv)
{
	struct stats_ktables
		*skc = (struct stats_ktables *) a->buf[curr];
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);
	
	render(isdb, pre, PT_USEINT,
	       "-\tdentunusd", NULL, NULL,
	       skc->dentry_stat, DNOVAL);

	render(isdb, pre, PT_USEINT,
	       "-\tfile-nr", NULL, NULL,
	       skc->file_used, DNOVAL);

	render(isdb, pre, PT_USEINT,
	       "-\tinode-nr", NULL, NULL,
	       skc->inode_used, DNOVAL);

	render(isdb, pre, PT_USEINT | pt_newlin,
	       "-\tpty-nr", NULL, NULL,
	       skc->pty_nr, DNOVAL);
}

/*
 ***************************************************************************
 * Display queue and load statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_queue_stats(struct activity *a, int isdb, char *pre,
				   int curr, unsigned long long itv)
{
	struct stats_queue
		*sqc = (struct stats_queue *) a->buf[curr];
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);
	
	render(isdb, pre, PT_USEINT,
	       "-\trunq-sz", NULL, NULL,
	       sqc->nr_running, DNOVAL);

	render(isdb, pre, PT_USEINT,
	       "-\tplist-sz", NULL, NULL,
	       sqc->nr_threads, DNOVAL);

	render(isdb, pre, PT_NOFLAG,
	       "-\tldavg-1", NULL, NULL,
	       NOVAL,
	       (double) sqc->load_avg_1 / 100);

	render(isdb, pre, PT_NOFLAG,
	       "-\tldavg-5", NULL, NULL,
	       NOVAL,
	       (double) sqc->load_avg_5 / 100);

	render(isdb, pre, pt_newlin,
	       "-\tldavg-15", NULL, NULL,
	       NOVAL,
	       (double) sqc->load_avg_15 / 100);
}

/*
 ***************************************************************************
 * Display serial lines statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_serial_stats(struct activity *a, int isdb, char *pre,
				    int curr, unsigned long long itv)
{
	int i;
	struct stats_serial *ssc, *ssp;
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);

	for (i = 0; i < a->nr; i++) {

		ssc = (struct stats_serial *) ((char *) a->buf[curr]  + i * a->msize);
		ssp = (struct stats_serial *) ((char *) a->buf[!curr] + i * a->msize);

		if (ssc->line == 0)
			continue;

		if (ssc->line == ssp->line) {
			render(isdb, pre, PT_NOFLAG,
			       "ttyS%d\trcvin/s", "%d",
			       cons(iv, ssc->line - 1, NOVAL),
			       NOVAL,
			       S_VALUE(ssp->rx, ssc->rx, itv));

			render(isdb, pre, PT_NOFLAG,
			       "ttyS%d\txmtin/s", "%d",
			       cons(iv, ssc->line - 1, NOVAL),
			       NOVAL,
			       S_VALUE(ssp->tx, ssc->tx, itv));

			render(isdb, pre, PT_NOFLAG,
			       "ttyS%d\tframerr/s", "%d",
			       cons(iv, ssc->line - 1, NOVAL),
			       NOVAL,
			       S_VALUE(ssp->frame, ssc->frame, itv));

			render(isdb, pre, PT_NOFLAG,
			       "ttyS%d\tprtyerr/s", "%d",
			       cons(iv, ssc->line - 1, NOVAL),
			       NOVAL,
			       S_VALUE(ssp->parity, ssc->parity, itv));

			render(isdb, pre, PT_NOFLAG,
			       "ttyS%d\tbrk/s", "%d",
			       cons(iv, ssc->line - 1, NOVAL),
			       NOVAL,
			       S_VALUE(ssp->brk, ssc->brk, itv));

			render(isdb, pre, pt_newlin,
			       "ttyS%d\tovrun/s", "%d",
			       cons(iv, ssc->line - 1, NOVAL),
			       NOVAL,
			       S_VALUE(ssp->overrun, ssc->overrun, itv));
		}
	}

}

/*
 ***************************************************************************
 * Display disks statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_disk_stats(struct activity *a, int isdb, char *pre,
				  int curr, unsigned long long itv)
{
	int i, j;
	struct stats_disk *sdc,	*sdp;
	struct ext_disk_stats xds;
	char *dev_name;
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);

	for (i = 0; i < a->nr; i++) {

		sdc = (struct stats_disk *) ((char *) a->buf[curr] + i * a->msize);

		if (!(sdc->major + sdc->minor))
			continue;

		j = check_disk_reg(a, curr, !curr, i);
		sdp = (struct stats_disk *) ((char *) a->buf[!curr] + j * a->msize);
		
		/* Compute extended stats (service time, etc.) */
		compute_ext_disk_stats(sdc, sdp, itv, &xds);

		dev_name = NULL;

		if ((USE_PRETTY_OPTION(flags)) && (sdc->major == DEVMAP_MAJOR)) {
			dev_name = transform_devmapname(sdc->major, sdc->minor);
		}

		if (!dev_name) {
			dev_name = get_devname(sdc->major, sdc->minor,
					       USE_PRETTY_OPTION(flags));
		}

		render(isdb, pre, PT_NOFLAG,
		       "%s\ttps", "%s",
		       cons(sv, dev_name, NULL),
		       NOVAL,
		       S_VALUE(sdp->nr_ios, sdc->nr_ios, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\trd_sec/s", NULL,
		       cons(sv, dev_name, NULL),
		       NOVAL,
		       ll_s_value(sdp->rd_sect, sdc->rd_sect, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\twr_sec/s", NULL,
		       cons(sv, dev_name, NULL),
		       NOVAL,
		       ll_s_value(sdp->wr_sect, sdc->wr_sect, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\tavgrq-sz", NULL,
		       cons(sv, dev_name, NULL),
		       NOVAL,
		       xds.arqsz);

		render(isdb, pre, PT_NOFLAG,
		       "%s\tavgqu-sz", NULL,
		       cons(sv, dev_name, NULL),
		       NOVAL,
		       S_VALUE(sdp->rq_ticks, sdc->rq_ticks, itv) / 1000.0);

		render(isdb, pre, PT_NOFLAG,
		       "%s\tawait", NULL,
		       cons(sv, dev_name, NULL),
		       NOVAL,
		       xds.await);

		render(isdb, pre, PT_NOFLAG,
		       "%s\tsvctm", NULL,
		       cons(sv, dev_name, NULL),
		       NOVAL,
		       xds.svctm);

		render(isdb, pre, pt_newlin,
		       "%s\t%%util", NULL,
		       cons(sv, dev_name, NULL),
		       NOVAL,
		       xds.util / 10.0);
	}
}

/*
 ***************************************************************************
 * Display network interfaces statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_net_dev_stats(struct activity *a, int isdb, char *pre,
				     int curr, unsigned long long itv)
{
	int i, j;
	struct stats_net_dev *sndc, *sndp;
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);

	for (i = 0; i < a->nr; i++) {

		sndc = (struct stats_net_dev *) ((char *) a->buf[curr] + i * a->msize);

		if (!strcmp(sndc->interface, ""))
			continue;
		
		j = check_net_dev_reg(a, curr, !curr, i);
		sndp = (struct stats_net_dev *) ((char *) a->buf[!curr] + j * a->msize);

		render(isdb, pre, PT_NOFLAG,
		       "%s\trxpck/s", "%s",
		       cons(sv, sndc->interface, NULL), /* What if the format args are strings? */
		       NOVAL,
		       S_VALUE(sndp->rx_packets, sndc->rx_packets, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\ttxpck/s", NULL,
		       cons(sv, sndc->interface, NULL),
		       NOVAL,
		       S_VALUE(sndp->tx_packets, sndc->tx_packets, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\trxkB/s", NULL,
		       cons(sv, sndc->interface, NULL),
		       NOVAL,
		       S_VALUE(sndp->rx_bytes, sndc->rx_bytes, itv) / 1024);

		render(isdb, pre, PT_NOFLAG,
		       "%s\ttxkB/s", NULL,
		       cons(sv, sndc->interface, NULL),
		       NOVAL,
		       S_VALUE(sndp->tx_bytes, sndc->tx_bytes, itv) / 1024);

		render(isdb, pre, PT_NOFLAG,
		       "%s\trxcmp/s", NULL,
		       cons(sv, sndc->interface, NULL),
		       NOVAL,
		       S_VALUE(sndp->rx_compressed, sndc->rx_compressed, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\ttxcmp/s", NULL,
		       cons(sv, sndc->interface, NULL),
		       NOVAL,
		       S_VALUE(sndp->tx_compressed, sndc->tx_compressed, itv));

		render(isdb, pre, pt_newlin,
		       "%s\trxmcst/s", NULL,
		       cons(sv, sndc->interface, NULL),
		       NOVAL,
		       S_VALUE(sndp->multicast, sndc->multicast, itv));
	}
}

/*
 ***************************************************************************
 * Display network interface errors statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_net_edev_stats(struct activity *a, int isdb, char *pre,
				      int curr, unsigned long long itv)
{
	int i, j;
	struct stats_net_edev *snedc, *snedp;
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);

	for (i = 0; i < a->nr; i++) {

		snedc = (struct stats_net_edev *) ((char *) a->buf[curr] + i * a->msize);

		if (!strcmp(snedc->interface, ""))
			continue;
		
		j = check_net_edev_reg(a, curr, !curr, i);
		snedp = (struct stats_net_edev *) ((char *) a->buf[!curr] + j * a->msize);

		render(isdb, pre, PT_NOFLAG,
		       "%s\trxerr/s", "%s",
		       cons(sv, snedc->interface, NULL),
		       NOVAL,
		       S_VALUE(snedp->rx_errors, snedc->rx_errors, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\ttxerr/s", NULL,
		       cons(sv, snedc->interface, NULL),
		       NOVAL,
		       S_VALUE(snedp->tx_errors, snedc->tx_errors, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\tcoll/s", NULL,
		       cons(sv, snedc->interface, NULL),
		       NOVAL,
		       S_VALUE(snedp->collisions, snedc->collisions, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\trxdrop/s", NULL,
		       cons(sv, snedc->interface, NULL),
		       NOVAL,
		       S_VALUE(snedp->rx_dropped, snedc->rx_dropped, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\ttxdrop/s", NULL,
		       cons(sv, snedc->interface, NULL),
		       NOVAL,
		       S_VALUE(snedp->tx_dropped, snedc->tx_dropped, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\ttxcarr/s", NULL,
		       cons(sv, snedc->interface, NULL),
		       NOVAL,
		       S_VALUE(snedp->tx_carrier_errors, snedc->tx_carrier_errors, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\trxfram/s", NULL,
		       cons(sv, snedc->interface, NULL),
		       NOVAL,
		       S_VALUE(snedp->rx_frame_errors, snedc->rx_frame_errors, itv));

		render(isdb, pre, PT_NOFLAG,
		       "%s\trxfifo/s", NULL,
		       cons(sv, snedc->interface, NULL),
		       NOVAL,
		       S_VALUE(snedp->rx_fifo_errors, snedc->rx_fifo_errors, itv));

		render(isdb, pre, pt_newlin,
		       "%s\ttxfifo/s", NULL,
		       cons(sv, snedc->interface, NULL),
		       NOVAL,
		       S_VALUE(snedp->tx_fifo_errors, snedc->tx_fifo_errors, itv));
	}
}

/*
 ***************************************************************************
 * Display NFS client statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_net_nfs_stats(struct activity *a, int isdb, char *pre,
				     int curr, unsigned long long itv)
{
	struct stats_net_nfs
		*snnc = (struct stats_net_nfs *) a->buf[curr],
		*snnp = (struct stats_net_nfs *) a->buf[!curr];
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);
	
	render(isdb, pre, PT_NOFLAG,
	       "-\tcall/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snnp->nfs_rpccnt, snnc->nfs_rpccnt, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tretrans/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snnp->nfs_rpcretrans, snnc->nfs_rpcretrans, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tread/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snnp->nfs_readcnt, snnc->nfs_readcnt, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\twrite/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snnp->nfs_writecnt, snnc->nfs_writecnt, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\taccess/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snnp->nfs_accesscnt, snnc->nfs_accesscnt, itv));

	render(isdb, pre, pt_newlin,
	       "-\tgetatt/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snnp->nfs_getattcnt, snnc->nfs_getattcnt, itv));
}

/*
 ***************************************************************************
 * Display NFS server statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_net_nfsd_stats(struct activity *a, int isdb, char *pre,
				      int curr, unsigned long long itv)
{
	struct stats_net_nfsd
		*snndc = (struct stats_net_nfsd *) a->buf[curr],
		*snndp = (struct stats_net_nfsd *) a->buf[!curr];
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);

	render(isdb, pre, PT_NOFLAG,
	       "-\tscall/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snndp->nfsd_rpccnt, snndc->nfsd_rpccnt, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tbadcall/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snndp->nfsd_rpcbad, snndc->nfsd_rpcbad, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tpacket/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snndp->nfsd_netcnt, snndc->nfsd_netcnt, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tudp/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snndp->nfsd_netudpcnt, snndc->nfsd_netudpcnt, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\ttcp/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snndp->nfsd_nettcpcnt, snndc->nfsd_nettcpcnt, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\thit/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snndp->nfsd_rchits, snndc->nfsd_rchits, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tmiss/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snndp->nfsd_rcmisses, snndc->nfsd_rcmisses, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tsread/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snndp->nfsd_readcnt, snndc->nfsd_readcnt, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tswrite/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snndp->nfsd_writecnt, snndc->nfsd_writecnt, itv));

	render(isdb, pre, PT_NOFLAG,
	       "-\tsaccess/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snndp->nfsd_accesscnt, snndc->nfsd_accesscnt, itv));

	render(isdb, pre, pt_newlin,
	       "-\tsgetatt/s", NULL, NULL,
	       NOVAL,
	       S_VALUE(snndp->nfsd_getattcnt, snndc->nfsd_getattcnt, itv));
}

/*
 ***************************************************************************
 * Display network sockets statistics in selected format.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @isdb	Flag, true if db printing, false if ppc printing.
 * @pre		Prefix string for output entries
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t render_net_sock_stats(struct activity *a, int isdb, char *pre,
				      int curr, unsigned long long itv)
{
	struct stats_net_sock
		*snsc = (struct stats_net_sock *) a->buf[curr];
	int pt_newlin
		= (DISPLAY_HORIZONTALLY(flags) ? PT_NOFLAG : PT_NEWLIN);

	render(isdb, pre, PT_USEINT,
	       "-\ttotsck", NULL, NULL,
	       snsc->sock_inuse, DNOVAL);

	render(isdb, pre, PT_USEINT,
	       "-\ttcpsck", NULL, NULL,
	       snsc->tcp_inuse, DNOVAL);

	render(isdb, pre, PT_USEINT,
	       "-\tudpsck",  NULL, NULL,
	       snsc->udp_inuse, DNOVAL);

	render(isdb, pre, PT_USEINT,
	       "-\trawsck", NULL, NULL,
	       snsc->raw_inuse, DNOVAL);

	render(isdb, pre, PT_USEINT,
	       "-\tip-frag", NULL, NULL,
	       snsc->frag_inuse, DNOVAL);

	render(isdb, pre, PT_USEINT | pt_newlin,
	       "-\ttcp-tw", NULL, NULL,
	       snsc->tcp_tw, DNOVAL);
}

/*
 ***************************************************************************
 * Print tabulations
 *
 * IN:
 * @nr_tab      Number of tabs to print.
 ***************************************************************************
 */
void prtab(int nr_tab)
{
	int i;

	for (i = 0; i < nr_tab; i++) {
		printf("\t");
	}
}

/*
 ***************************************************************************
 * printf() function modified for XML display
 *
 * IN:
 * @nr_tab      Number of tabs to print.
 * @fmt         printf() format.
 ***************************************************************************
 */
void xprintf(int nr_tab, const char *fmt, ...)
{
	static char buf[1024];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	prtab(nr_tab);
	printf("%s\n", buf);
}

/*
 ***************************************************************************
 * Open or close <network> markup.
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Open or close action.
 ***************************************************************************
 */
void xml_markup_network(int tab, int action)
{
	static int markup_state = CLOSE_XML_MARKUP;

	if (action == markup_state)
		return;
	markup_state = action;

	if (action == OPEN_XML_MARKUP) {
		/* Open markup */
		xprintf(tab, "<network per=\"second\">");
	}
	else {
		/* Close markup */
		xprintf(--tab, "</network>");
	}
}

/*
 ***************************************************************************
 * Display CPU statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @g_itv	Interval of time in jiffies mutliplied by the number of
 * 		processors.
 ***************************************************************************
 */
__print_funct_t xml_print_cpu_stats(struct activity *a, int curr, int tab,
				    unsigned long long g_itv)
{
	int i;
	struct stats_cpu *scc, *scp;
	char cpuno[8];

	if (DISPLAY_CPU_DEF(a->opt_flags)) {
		xprintf(tab++, "<cpu-load>");
	}
	else if (DISPLAY_CPU_ALL(a->opt_flags)) {
		xprintf(tab++, "<cpu-load-all>");
	}

	for (i = 0; (i < a->nr) && (i < a->bitmap_size + 1); i++) {
		
		scc = (struct stats_cpu *) ((char *) a->buf[curr]  + i * a->msize);
		scp = (struct stats_cpu *) ((char *) a->buf[!curr] + i * a->msize);

		/* Should current CPU (including CPU "all") be displayed? */
		if (a->bitmap[i >> 3] & (1 << (i & 0x07))) {
			
			/* Yes: Display it */
			if (!i) {
				/* This is CPU "all" */
				strcpy(cpuno, "all");
			}
			else {
				sprintf(cpuno, "%d", i - 1);

				/* Recalculate interval for current proc */
				g_itv = get_per_cpu_interval(scc, scp);
				
				if (!g_itv) {
					/* Current CPU is offline */
					if (DISPLAY_CPU_DEF(a->opt_flags)) {
						xprintf(tab, "<cpu number=\"%d\" "
							"user=\"0.00\" "
							"nice=\"0.00\" "
							"system=\"0.00\" "
							"iowait=\"0.00\" "
							"steal=\"0.00\" "
							"idle=\"0.00\"/>",
							i - 1);
					}
					else if (DISPLAY_CPU_ALL(a->opt_flags)) {
						xprintf(tab, "<cpu number=\"%d\" "
							"usr=\"0.00\" "
							"nice=\"0.00\" "
							"sys=\"0.00\" "
							"iowait=\"0.00\" "
							"steal=\"0.00\" "
							"irq=\"0.00\" "
							"soft=\"0.00\" "
							"guest=\"0.00\" "
							"idle=\"0.00\"/>",
							i - 1);
					}
					continue;
				}
			}

			if (DISPLAY_CPU_DEF(a->opt_flags)) {
				xprintf(tab, "<cpu number=\"%s\" "
					"user=\"%.2f\" "
					"nice=\"%.2f\" "
					"system=\"%.2f\" "
					"iowait=\"%.2f\" "
					"steal=\"%.2f\" "
					"idle=\"%.2f\"/>",
					cpuno,
					ll_sp_value(scp->cpu_user,   scc->cpu_user,   g_itv),
					ll_sp_value(scp->cpu_nice,   scc->cpu_nice,   g_itv),
					ll_sp_value(scp->cpu_sys + scp->cpu_hardirq + scp->cpu_softirq,
						    scc->cpu_sys + scc->cpu_hardirq + scc->cpu_softirq,
						    g_itv),
					ll_sp_value(scp->cpu_iowait, scc->cpu_iowait, g_itv),
					ll_sp_value(scp->cpu_steal,  scc->cpu_steal,  g_itv),
					scc->cpu_idle < scp->cpu_idle ?
					0.0 :
					ll_sp_value(scp->cpu_idle,   scc->cpu_idle,   g_itv));
			}
			else if (DISPLAY_CPU_ALL(a->opt_flags)) {
				xprintf(tab, "<cpu number=\"%s\" "
					"usr=\"%.2f\" "
					"nice=\"%.2f\" "
					"sys=\"%.2f\" "
					"iowait=\"%.2f\" "
					"steal=\"%.2f\" "
					"irq=\"%.2f\" "
					"soft=\"%.2f\" "
					"guest=\"%.2f\" "
					"idle=\"%.2f\"/>",
					cpuno,
					ll_sp_value(scp->cpu_user - scp->cpu_guest,
						    scc->cpu_user - scc->cpu_guest,     g_itv),
					ll_sp_value(scp->cpu_nice,    scc->cpu_nice,    g_itv),
					ll_sp_value(scp->cpu_sys,     scc->cpu_sys,     g_itv),
					ll_sp_value(scp->cpu_iowait,  scc->cpu_iowait,  g_itv),
					ll_sp_value(scp->cpu_steal,   scc->cpu_steal,   g_itv),
					ll_sp_value(scp->cpu_hardirq, scc->cpu_hardirq, g_itv),
					ll_sp_value(scp->cpu_softirq, scc->cpu_softirq, g_itv),
					ll_sp_value(scp->cpu_guest,   scc->cpu_guest,   g_itv),
					scc->cpu_idle < scp->cpu_idle ?
					0.0 :
					ll_sp_value(scp->cpu_idle,   scc->cpu_idle,   g_itv));
			}
		}
	}

	if (DISPLAY_CPU_DEF(a->opt_flags)) {
		xprintf(--tab, "</cpu-load>");
	}
	else if (DISPLAY_CPU_ALL(a->opt_flags)) {
		xprintf(--tab, "</cpu-load-all>");
	}
}

/*
 ***************************************************************************
 * Display task creation and context switch statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_pcsw_stats(struct activity *a, int curr, int tab,
				     unsigned long long itv)
{
	struct stats_pcsw
		*spc = (struct stats_pcsw *) a->buf[curr],
		*spp = (struct stats_pcsw *) a->buf[!curr];

	/* proc/s and cswch/s */
	xprintf(tab, "<process-and-context-switch per=\"second\" "
		"proc=\"%.2f\" "
		"cswch=\"%.2f\"/>",
		S_VALUE(spp->processes, spc->processes, itv),
		ll_s_value(spp->context_switch, spc->context_switch, itv));
}

/*
 ***************************************************************************
 * Display interrupts statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_irq_stats(struct activity *a, int curr, int tab,
				    unsigned long long itv)
{
	int i;
	struct stats_irq *sic, *sip;
	char irqno[8];
	
	xprintf(tab++, "<interrupts>");
	xprintf(tab++, "<int-global per=\"second\">");

	for (i = 0; (i < a->nr) && (i < a->bitmap_size + 1); i++) {

		sic = (struct stats_irq *) ((char *) a->buf[curr]  + i * a->msize);
		sip = (struct stats_irq *) ((char *) a->buf[!curr] + i * a->msize);
		
		/* Should current interrupt (including int "sum") be displayed? */
		if (a->bitmap[i >> 3] & (1 << (i & 0x07))) {
			
			/* Yes: Display it */
			if (!i) {
				/* This is interrupt "sum" */
				strcpy(irqno, "sum");
			}
			else {
				sprintf(irqno, "%d", i - 1);
			}

			xprintf(tab, "<irq intr=\"%s\" value=\"%.2f\"/>", irqno,
				ll_s_value(sip->irq_nr, sic->irq_nr, itv));
		}
	}

	xprintf(--tab, "</int-global>");
	xprintf(--tab, "</interrupts>");
}

/*
 ***************************************************************************
 * Display swapping statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_swap_stats(struct activity *a, int curr, int tab,
				     unsigned long long itv)
{
	struct stats_swap
		*ssc = (struct stats_swap *) a->buf[curr],
		*ssp = (struct stats_swap *) a->buf[!curr];
	
	xprintf(tab, "<swap-pages per=\"second\" "
		"pswpin=\"%.2f\" "
		"pswpout=\"%.2f\"/>",
		S_VALUE(ssp->pswpin,  ssc->pswpin,  itv),
		S_VALUE(ssp->pswpout, ssc->pswpout, itv));
}

/*
 ***************************************************************************
 * Display paging statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_paging_stats(struct activity *a, int curr, int tab,
				       unsigned long long itv)
{
	struct stats_paging
		*spc = (struct stats_paging *) a->buf[curr],
		*spp = (struct stats_paging *) a->buf[!curr];

	xprintf(tab, "<paging per=\"second\" "
		"pgpgin=\"%.2f\" "
		"pgpgout=\"%.2f\" "
		"fault=\"%.2f\" "
		"majflt=\"%.2f\" "
		"pgfree=\"%.2f\" "
		"pgscank=\"%.2f\" "
		"pgscand=\"%.2f\" "
		"pgsteal=\"%.2f\" "
		"vmeff-percent=\"%.2f\"/>",
	       S_VALUE(spp->pgpgin,        spc->pgpgin,        itv),
	       S_VALUE(spp->pgpgout,       spc->pgpgout,       itv),
	       S_VALUE(spp->pgfault,       spc->pgfault,       itv),
	       S_VALUE(spp->pgmajfault,    spc->pgmajfault,    itv),
	       S_VALUE(spp->pgfree,        spc->pgfree,        itv),
	       S_VALUE(spp->pgscan_kswapd, spc->pgscan_kswapd, itv),
	       S_VALUE(spp->pgscan_direct, spc->pgscan_direct, itv),
	       S_VALUE(spp->pgsteal,       spc->pgsteal,       itv),
	       (spc->pgscan_kswapd + spc->pgscan_direct -
		spp->pgscan_kswapd - spp->pgscan_direct) ?
	       SP_VALUE(spp->pgsteal, spc->pgsteal,
			spc->pgscan_kswapd + spc->pgscan_direct -
			spp->pgscan_kswapd - spp->pgscan_direct) : 0.0);
}

/*
 ***************************************************************************
 * Display I/O and transfer rate statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_io_stats(struct activity *a, int curr, int tab,
				   unsigned long long itv)
{
	struct stats_io
		*sic = (struct stats_io *) a->buf[curr],
		*sip = (struct stats_io *) a->buf[!curr];

	xprintf(tab, "<io per=\"second\">");

	xprintf(++tab, "<tps>%.2f</tps>",
		S_VALUE(sip->dk_drive, sic->dk_drive, itv));
	
	xprintf(tab, "<io-reads rtps=\"%.2f\" bread=\"%.2f\"/>",
		S_VALUE(sip->dk_drive_rio,  sic->dk_drive_rio,  itv),
		S_VALUE(sip->dk_drive_rblk, sic->dk_drive_rblk, itv));
	
	xprintf(tab, "<io-writes wtps=\"%.2f\" bwrtn=\"%.2f\"/>",
		S_VALUE(sip->dk_drive_wio,  sic->dk_drive_wio,  itv),
		S_VALUE(sip->dk_drive_wblk, sic->dk_drive_wblk, itv));
	
	xprintf(--tab, "</io>");
}

/*
 ***************************************************************************
 * Display memory statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_memory_stats(struct activity *a, int curr, int tab,
				       unsigned long long itv)
{
	struct stats_memory
		*smc = (struct stats_memory *) a->buf[curr],
		*smp = (struct stats_memory *) a->buf[!curr];

	xprintf(tab, "<memory per=\"second\" unit=\"kB\">");
	
	if (DISPLAY_MEM_AMT(a->opt_flags)) {

		xprintf(++tab, "<memfree>%lu</memfree>",
			smc->frmkb);
	
		xprintf(tab, "<memused>%lu</memused>",
			smc->tlmkb - smc->frmkb);

		xprintf(tab, "<memused-percent>%.2f</memused-percent>",
			smc->tlmkb ?
			SP_VALUE(smc->frmkb, smc->tlmkb, smc->tlmkb) :
			0.0);
	
		xprintf(tab, "<buffers>%lu</buffers>",
			smc->bufkb);
	
		xprintf(tab, "<cached>%lu</cached>",
			smc->camkb);

		xprintf(tab, "<commit>%lu</commit>",
			smc->comkb);

		xprintf(tab--, "<commit-percent>%.2f</commit-percent>",
			(smc->tlmkb + smc->tlskb) ?
			SP_VALUE(0, smc->comkb, smc->tlmkb + smc->tlskb) :
			0.0);
	}
		
	if (DISPLAY_SWAP(a->opt_flags)) {

		xprintf(++tab, "<swpfree>%lu</swpfree>",
			smc->frskb);
	
		xprintf(tab, "<swpused>%lu</swpused>",
			smc->tlskb - smc->frskb);
	
		xprintf(tab, "<swpused-percent>%.2f</swpused-percent>",
			smc->tlskb ?
			SP_VALUE(smc->frskb, smc->tlskb, smc->tlskb) :
			0.0);

		xprintf(tab, "<swpcad>%lu</swpcad>",
			smc->caskb);

		xprintf(tab--, "<swpcad-percent>%.2f</swpcad-percent>",
			(smc->tlskb - smc->frskb) ?
			SP_VALUE(0, smc->caskb, smc->tlskb - smc->frskb) :
			0.0);
	}

	if (DISPLAY_MEMORY(a->opt_flags)) {

		xprintf(++tab, "<frmpg>%.2f</frmpg>",
			S_VALUE((double) KB_TO_PG(smp->frmkb),
				(double) KB_TO_PG(smc->frmkb), itv));
	
		xprintf(tab, "<bufpg>%.2f</bufpg>",
			S_VALUE((double) KB_TO_PG(smp->bufkb),
				(double) KB_TO_PG(smc->bufkb), itv));
	
		xprintf(tab--, "<campg>%.2f</campg>",
			S_VALUE((double) KB_TO_PG(smp->camkb),
				(double) KB_TO_PG(smc->camkb), itv));
	}

	xprintf(tab, "</memory>");
}

/*
 ***************************************************************************
 * Display kernel tables statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_ktables_stats(struct activity *a, int curr, int tab,
					unsigned long long itv)
{
	struct stats_ktables
		*skc = (struct stats_ktables *) a->buf[curr];
	
	xprintf(tab, "<kernel per=\"second\">");

	xprintf(++tab, "<dentunusd>%u</dentunusd>",
		skc->dentry_stat);
	
	xprintf(tab, "<file-nr>%u</file-nr>",
		skc->file_used);
	
	xprintf(tab, "<inode-nr>%u</inode-nr>",
		skc->inode_used);
	
	xprintf(tab, "<pty-nr>%u</pty-nr>",
		skc->pty_nr);

	xprintf(--tab, "</kernel>");
}

/*
 ***************************************************************************
 * Display queue and load statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_queue_stats(struct activity *a, int curr, int tab,
				      unsigned long long itv)
{
	struct stats_queue
		*sqc = (struct stats_queue *) a->buf[curr];
	
	xprintf(tab, "<queue "
		"runq-sz=\"%lu\" "
		"plist-sz=\"%u\" "
		"ldavg-1=\"%.2f\" "
		"ldavg-5=\"%.2f\" "
		"ldavg-15=\"%.2f\"/>",
		sqc->nr_running,
		sqc->nr_threads,
		(double) sqc->load_avg_1 / 100,
		(double) sqc->load_avg_5 / 100,
		(double) sqc->load_avg_15 / 100);
}

/*
 ***************************************************************************
 * Display serial lines statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_serial_stats(struct activity *a, int curr, int tab,
				       unsigned long long itv)
{
	int i;
	struct stats_serial *ssc, *ssp;

	xprintf(tab, "<serial per=\"second\">");
	tab++;

	for (i = 0; i < a->nr; i++) {

		ssc = (struct stats_serial *) ((char *) a->buf[curr]  + i * a->msize);
		ssp = (struct stats_serial *) ((char *) a->buf[!curr] + i * a->msize);

		if (ssc->line == 0)
			continue;

		if (ssc->line == ssp->line) {

			xprintf(tab, "<tty line=\"%d\" "
				"rcvin=\"%.2f\" "
				"xmtin=\"%.2f\" "
				"framerr=\"%.2f\" "
				"prtyerr=\"%.2f\" "
				"brk=\"%.2f\" "
				"ovrun=\"%.2f\"/>",
				ssc->line - 1,
				S_VALUE(ssp->rx,      ssc->rx,      itv),
				S_VALUE(ssp->tx,      ssc->tx,      itv),
				S_VALUE(ssp->frame,   ssc->frame,   itv),
				S_VALUE(ssp->parity,  ssc->parity,  itv),
				S_VALUE(ssp->brk,     ssc->brk,     itv),
				S_VALUE(ssp->overrun, ssc->overrun, itv));
		}
	}

	xprintf(--tab, "</serial>");
}

/*
 ***************************************************************************
 * Display disks statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_disk_stats(struct activity *a, int curr, int tab,
				     unsigned long long itv)
{
	int i, j;
	struct stats_disk *sdc,	*sdp;
	struct ext_disk_stats xds;
	char *dev_name;

	xprintf(tab, "<disk per=\"second\">");
	tab++;

	for (i = 0; i < a->nr; i++) {

		sdc = (struct stats_disk *) ((char *) a->buf[curr] + i * a->msize);

		if (!(sdc->major + sdc->minor))
			continue;

		j = check_disk_reg(a, curr, !curr, i);
		sdp = (struct stats_disk *) ((char *) a->buf[!curr] + j * a->msize);

		/* Compute extended statistics values */
		compute_ext_disk_stats(sdc, sdp, itv, &xds);
		
		dev_name = NULL;

		if ((USE_PRETTY_OPTION(flags)) && (sdc->major == DEVMAP_MAJOR)) {
			dev_name = transform_devmapname(sdc->major, sdc->minor);
		}

		if (!dev_name) {
			dev_name = get_devname(sdc->major, sdc->minor,
					       USE_PRETTY_OPTION(flags));
		}

		xprintf(tab, "<disk-device dev=\"%s\" "
			"tps=\"%.2f\" "
			"rd_sec=\"%.2f\" "
			"wr_sec=\"%.2f\" "
			"avgrq-sz=\"%.2f\" "
			"avgqu-sz=\"%.2f\" "
			"await=\"%.2f\" "
			"svctm=\"%.2f\" "
			"util-percent=\"%.2f\"/>",
			/* Confusion possible here between index and minor numbers */
			dev_name,
			S_VALUE(sdp->nr_ios, sdc->nr_ios, itv),
			ll_s_value(sdp->rd_sect, sdc->rd_sect, itv),
			ll_s_value(sdp->wr_sect, sdc->wr_sect, itv),
			/* See iostat for explanations */
			xds.arqsz,
			S_VALUE(sdp->rq_ticks, sdc->rq_ticks, itv) / 1000.0,
			xds.await,
			xds.svctm,
			xds.util / 10.0);
	}

	xprintf(--tab, "</disk>");
}

/*
 ***************************************************************************
 * Display network interfaces statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_net_dev_stats(struct activity *a, int curr, int tab,
					unsigned long long itv)
{
	int i, j;
	struct stats_net_dev *sndc, *sndp;

	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_xml_markup;

	xml_markup_network(tab, OPEN_XML_MARKUP);
	tab++;

	for (i = 0; i < a->nr; i++) {

		sndc = (struct stats_net_dev *) ((char *) a->buf[curr] + i * a->msize);

		if (!strcmp(sndc->interface, ""))
			continue;
		
		j = check_net_dev_reg(a, curr, !curr, i);
		sndp = (struct stats_net_dev *) ((char *) a->buf[!curr] + j * a->msize);

		xprintf(tab, "<net-dev iface=\"%s\" "
			"rxpck=\"%.2f\" "
			"txpck=\"%.2f\" "
			"rxkB=\"%.2f\" "
			"txkB=\"%.2f\" "
			"rxcmp=\"%.2f\" "
			"txcmp=\"%.2f\" "
			"rxmcst=\"%.2f\"/>",
			sndc->interface,
			S_VALUE(sndp->rx_packets,    sndc->rx_packets,    itv),
			S_VALUE(sndp->tx_packets,    sndc->tx_packets,    itv),
			S_VALUE(sndp->rx_bytes,      sndc->rx_bytes,      itv) / 1024,
			S_VALUE(sndp->tx_bytes,      sndc->tx_bytes,      itv) / 1024,
			S_VALUE(sndp->rx_compressed, sndc->rx_compressed, itv),
			S_VALUE(sndp->tx_compressed, sndc->tx_compressed, itv),
			S_VALUE(sndp->multicast,     sndc->multicast,     itv));
	}

close_xml_markup:
	if (CLOSE_MARKUP(a->options)) {
		xml_markup_network(tab, CLOSE_XML_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display network interfaces error statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_net_edev_stats(struct activity *a, int curr, int tab,
					 unsigned long long itv)
{
	int i, j;
	struct stats_net_edev *snedc, *snedp;

	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_xml_markup;

	xml_markup_network(tab, OPEN_XML_MARKUP);
	tab++;

	for (i = 0; i < a->nr; i++) {

		snedc = (struct stats_net_edev *) ((char *) a->buf[curr] + i * a->msize);

		if (!strcmp(snedc->interface, ""))
			continue;
		
		j = check_net_edev_reg(a, curr, !curr, i);
		snedp = (struct stats_net_edev *) ((char *) a->buf[!curr] + j * a->msize);

		xprintf(tab, "<net-edev iface=\"%s\" "
			"rxerr=\"%.2f\" "
			"txerr=\"%.2f\" "
			"coll=\"%.2f\" "
			"rxdrop=\"%.2f\" "
			"txdrop=\"%.2f\" "
			"txcarr=\"%.2f\" "
			"rxfram=\"%.2f\" "
			"rxfifo=\"%.2f\" "
			"txfifo=\"%.2f\"/>",
			snedc->interface,
			S_VALUE(snedp->rx_errors,         snedc->rx_errors,         itv),
			S_VALUE(snedp->tx_errors,         snedc->tx_errors,         itv),
			S_VALUE(snedp->collisions,        snedc->collisions,        itv),
			S_VALUE(snedp->rx_dropped,        snedc->rx_dropped,        itv),
			S_VALUE(snedp->tx_dropped,        snedc->tx_dropped,        itv),
			S_VALUE(snedp->tx_carrier_errors, snedc->tx_carrier_errors, itv),
			S_VALUE(snedp->rx_frame_errors,   snedc->rx_frame_errors,   itv),
			S_VALUE(snedp->rx_fifo_errors,    snedc->rx_fifo_errors,    itv),
			S_VALUE(snedp->tx_fifo_errors,    snedc->tx_fifo_errors,    itv));
	}

close_xml_markup:
	if (CLOSE_MARKUP(a->options)) {
		xml_markup_network(tab, CLOSE_XML_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display NFS client statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_net_nfs_stats(struct activity *a, int curr, int tab,
					unsigned long long itv)
{
	struct stats_net_nfs
		*snnc = (struct stats_net_nfs *) a->buf[curr],
		*snnp = (struct stats_net_nfs *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_xml_markup;

	xml_markup_network(tab, OPEN_XML_MARKUP);
	tab++;

	xprintf(tab, "<net-nfs "
		"call=\"%.2f\" "
		"retrans=\"%.2f\" "
		"read=\"%.2f\" "
		"write=\"%.2f\" "
		"access=\"%.2f\" "
		"getatt=\"%.2f\"/>",
		S_VALUE(snnp->nfs_rpccnt,     snnc->nfs_rpccnt,     itv),
		S_VALUE(snnp->nfs_rpcretrans, snnc->nfs_rpcretrans, itv),
		S_VALUE(snnp->nfs_readcnt,    snnc->nfs_readcnt,    itv),
		S_VALUE(snnp->nfs_writecnt,   snnc->nfs_writecnt,   itv),
		S_VALUE(snnp->nfs_accesscnt,  snnc->nfs_accesscnt,  itv),
		S_VALUE(snnp->nfs_getattcnt,  snnc->nfs_getattcnt,  itv));

close_xml_markup:
	if (CLOSE_MARKUP(a->options)) {
		xml_markup_network(tab, CLOSE_XML_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display NFS server statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_net_nfsd_stats(struct activity *a, int curr, int tab,
       				   	 unsigned long long itv)
{
	struct stats_net_nfsd
		*snndc = (struct stats_net_nfsd *) a->buf[curr],
		*snndp = (struct stats_net_nfsd *) a->buf[!curr];

	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_xml_markup;

	xml_markup_network(tab, OPEN_XML_MARKUP);
	tab++;

	xprintf(tab, "<net-nfsd "
		"scall=\"%.2f\" "
		"badcall=\"%.2f\" "
		"packet=\"%.2f\" "
		"udp=\"%.2f\" "
		"tcp=\"%.2f\" "
		"hit=\"%.2f\" "
		"miss=\"%.2f\" "
		"sread=\"%.2f\" "
		"swrite=\"%.2f\" "
		"saccess=\"%.2f\" "
		"sgetatt=\"%.2f\"/>",
		S_VALUE(snndp->nfsd_rpccnt,    snndc->nfsd_rpccnt,    itv),
		S_VALUE(snndp->nfsd_rpcbad,    snndc->nfsd_rpcbad,    itv),
		S_VALUE(snndp->nfsd_netcnt,    snndc->nfsd_netcnt,    itv),
		S_VALUE(snndp->nfsd_netudpcnt, snndc->nfsd_netudpcnt, itv),
		S_VALUE(snndp->nfsd_nettcpcnt, snndc->nfsd_nettcpcnt, itv),
		S_VALUE(snndp->nfsd_rchits,    snndc->nfsd_rchits,    itv),
		S_VALUE(snndp->nfsd_rcmisses,  snndc->nfsd_rcmisses,  itv),
		S_VALUE(snndp->nfsd_readcnt,   snndc->nfsd_readcnt,   itv),
		S_VALUE(snndp->nfsd_writecnt,  snndc->nfsd_writecnt,  itv),
		S_VALUE(snndp->nfsd_accesscnt, snndc->nfsd_accesscnt, itv),
		S_VALUE(snndp->nfsd_getattcnt, snndc->nfsd_getattcnt, itv));

close_xml_markup:
	if (CLOSE_MARKUP(a->options)) {
		xml_markup_network(tab, CLOSE_XML_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display network socket statistics in XML.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t xml_print_net_sock_stats(struct activity *a, int curr, int tab,
       				   	 unsigned long long itv)
{
	struct stats_net_sock
		*snsc = (struct stats_net_sock *) a->buf[curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_xml_markup;

	xml_markup_network(tab, OPEN_XML_MARKUP);
	tab++;

	xprintf(tab, "<net-sock "
		"totsck=\"%u\" "
		"tcpsck=\"%u\" "
		"udpsck=\"%u\" "
		"rawsck=\"%u\" "
		"ip-frag=\"%u\" "
		"tcp-tw=\"%u\"/>",
		snsc->sock_inuse,
	       	snsc->tcp_inuse,
	       	snsc->udp_inuse,
       		snsc->raw_inuse,
	       	snsc->frag_inuse,
	       	snsc->tcp_tw);

close_xml_markup:
	if (CLOSE_MARKUP(a->options)) {
		xml_markup_network(tab, CLOSE_XML_MARKUP);
	}
}

