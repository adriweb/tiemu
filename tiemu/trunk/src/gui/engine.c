/* Hey EMACS -*- linux-c -*- */
/* $Id$ */

/*  TiEmu - an TI emulator
 *
 *  Copyright (c) 2000-2001, Thomas Corvazier, Romain Lievin
 *  Copyright (c) 2001-2003, Romain Lievin
 *  Copyright (c) 2003, Julien Blache
 *  Copyright (c) 2004, Romain Li�vin
 *  Copyright (c) 2005, Romain Li�vin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
    Run engine from GTK main loop at regular interval.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>

#include "intl.h"
#include "ti68k_int.h"
#include "m68k.h"
#include "engine.h"
#include "dbg_all.h"

/* 
   The TI92/89 should approximately execute NB_CYCLES_PER_LOOP_HW[12] in 
   TIME_LIMIT milliseconds (10.000.000 or 12.000.000 cycles/s).
   If you think this values are a bit too big, you can slow down 
   the emulator by changing them 
*/
#define NB_CYCLES_PER_LOOP_HW1 300000	// 300000 cycles
#define NB_CYCLES_PER_LOOP_HW2 360000	// 360000 cycles
#define TIME_LIMIT               30	    // 30 ms
#define MIN_INSTRUCTIONS_PER_CYCLE 4 	// instructions take at least 4 cycles

static int cpu_cycles = NB_CYCLES_PER_LOOP_HW2;

static guint tid = 0;
static gint  res = 0;
static guint cal = 0;

// function called when the function below is exited
static void engine_notify(gint *data)
{
	// call debugger here
	if(*data)
		gtk_debugger_enter(GPOINTER_TO_INT(*data));

	// and reset source id
	tid = 0;
}

// function called by g_timeout_add_full/g_idle_add_full
static gboolean engine_func(gint *data)
{
	GTimer *tmr = g_timer_new();
	gdouble ms;

	// set instruction rate
    if(params.cpu_rate != -1)
        cpu_cycles = params.cpu_rate;
    else
        cpu_cycles = tihw.hw_type == HW1 ? NB_CYCLES_PER_LOOP_HW1 : NB_CYCLES_PER_LOOP_HW2;

	// run emulation core
	g_timer_start(tmr);
	*data = hw_m68k_run(cpu_cycles / MIN_INSTRUCTIONS_PER_CYCLE, cpu_cycles);
	g_timer_stop(tmr);

	// a bkpt has been encountered ? If yes, stop engine
	if(*data)
		return FALSE;

	// get time needed to execute 'cpu_cycles' cycles
	ms = 1000 * g_timer_elapsed(tmr, NULL); // return 0, why ?
	g_timer_destroy(tmr);
	//cal = (guint)ms;

	return TRUE;
}

// start emulation engine
void engine_start(void) 
{
	if (cal >= TIME_LIMIT)
		fprintf(stderr, "warning: emulation slower than TI (cal = %d, TIME_LIMIT = %d)\n", cal, TIME_LIMIT);
	else if (cal >= TIME_LIMIT-5)
		fprintf(stderr, "warning: emulation may be slower than TI (cal = %d, TIME_LIMIT = %d)\n", cal, TIME_LIMIT);

	if(params.restricted && cal < TIME_LIMIT)
		tid = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, TIME_LIMIT-cal, 
					 (GSourceFunc)engine_func, &res, 
					 (GDestroyNotify)engine_notify);
	else
		tid = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, 
				      (GSourceFunc)engine_func, &res, 
				      (GDestroyNotify)engine_notify);
}

// stop it
void engine_stop(void) 
{
	if(tid)
		g_source_remove(tid);
}

// state of engine
int engine_is_stopped() 
{
	return !tid;
}

int engine_is_running(void)
{
	return tid;
}

/*
	Called at startup to know needed time for exec'ing hw_m68k_run.
	This allow to make exec'ing more precise.
*/
#define NLOOPS	20
void engine_calibrate(void)
{
	int i;
	gdouble ms;
	GTimer *tmr = g_timer_new();
	int cycles = tihw.hw_type == HW1 ? NB_CYCLES_PER_LOOP_HW1 : NB_CYCLES_PER_LOOP_HW2;

	fprintf(stdout, "Calibrating engine: ");
	g_timer_start(tmr);

	for(i = 0; i < NLOOPS; i++)
	{
		hw_m68k_run(cycles / MIN_INSTRUCTIONS_PER_CYCLE, cycles);
	}

	g_timer_stop(tmr);
	ms = 1000 * g_timer_elapsed(tmr, NULL);
	g_timer_destroy(tmr);
	
	cal = (guint)(ms / NLOOPS);
	fprintf(stdout, "%i loops in %.1f ms => %i ms\n", i, ms, cal);
}