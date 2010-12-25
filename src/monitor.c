/*
 * Copyright (c) 2008-2009, Victor Goya <phorque@libflm.me>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/monitor.h"
#include "flm/core/private/epoll.h"
#include "flm/core/private/select.h"
#include "flm/core/private/timer.h"

flm_Monitor *
flm_MonitorNew ()
{
	flm_Monitor * monitor;
	const char * backend;

	backend = getenv (FLM_MONITOR_BACKEND_ENV);

	if ((!backend || !strcmp (backend, "epoll")) &&
	    (monitor = FLM_MONITOR (flm__EpollNew ()))) {
		return (monitor);
	}

	if ((!backend || !strcmp (backend, "select")) &&
	    (monitor = FLM_MONITOR (flm__SelectNew ()))) {
		return (monitor);
	}

	return (NULL);
}

int
flm_MonitorWait (flm_Monitor * monitor)
{
	if (monitor->wait == NULL) {
		return (0);
	}
	while (monitor->wait (monitor) != -1 && monitor->count > 0) {
		flm__MonitorTimerTick (monitor);
	}
	return (0);
}

int
flm__MonitorInit (flm_Monitor * monitor)
{
	size_t count;

	if (flm__ObjInit (FLM_OBJ (monitor)) == -1) {
		return (-1);
	}
	FLM_OBJ (monitor)->type = FLM__TYPE_MONITOR;

	monitor->add		=	NULL;
	monitor->del		=	NULL;
	monitor->reset		=	NULL;
	monitor->wait		=	NULL;

	monitor->count		=	0;

	if (clock_gettime (CLOCK_MONOTONIC, &(monitor->tm.current)) == -1) {
		return (-1);
	}
	monitor->tm.next	=	0;
	monitor->tm.pos		=	0;
	for (count = 0; count < FLM__MONITOR_TM_WHEEL_SIZE; count++) {
		TAILQ_INIT (&(monitor->tm.wheel[count]));
	}
	return (0);
}

int
flm__MonitorIOAdd (flm_Monitor * monitor,
		   flm_IO * io)
{
	if (monitor->add && monitor->add (monitor, io) == -1) {
		return (-1);
	}
	flm_Retain (FLM_OBJ (io));
	monitor->count++;
	return (0);
}

void
flm__MonitorIODelete (flm_Monitor * monitor,
		      flm_IO * io)
{
	if (monitor->del) {
		monitor->del (monitor, io);
	}
	flm_Release (FLM_OBJ (io));
	monitor->count--;
	return ;
}

int
flm__MonitorIOReset (flm_Monitor * monitor,
		     flm_IO * io)
{
	if (monitor->reset && monitor->reset (monitor, io) == -1) {
		return (-1);
	}
	return (0);
}

int
flm__MonitorTimerTick (flm_Monitor * monitor)
{
	struct timespec current;
	uint32_t sec;
	uint32_t pos;

	flm_Timer * timer;
	struct flm_Timer temp;

	/* update time */
	if (clock_gettime (CLOCK_MONOTONIC, &current) == -1) {
		return (-1);
	}

	sec = current.tv_sec - monitor->tm.current.tv_sec;

	for (pos = 0; pos < sec; pos++) {
		TAILQ_FOREACH (timer,
			       &(monitor->tm.wheel[(monitor->tm.pos + pos) % \
						   FLM__MONITOR_TM_WHEEL_SIZE]),
			       wh.entries) {
			if (timer->wh.rounds > 0) {
				timer->wh.rounds--;
				continue ;

			}
			temp.wh.entries = timer->wh.entries;

			flm_Retain (FLM_OBJ (timer));
			flm_TimerCancel (timer);
			if (timer->handler) {
				timer->handler (timer->data);
			}
			flm_Release (FLM_OBJ (timer));
			timer = &temp;
		}
	}
	monitor->tm.current = current;
	monitor->tm.pos = (monitor->tm.pos + sec) %	\
		FLM__MONITOR_TM_WHEEL_SIZE;
	return (0);
}

void
flm__MonitorTimerAdd (flm_Monitor *	monitor,
		      flm_Timer *	timer,
		      uint32_t		delay)
{
	timer->wh.rounds = delay / FLM__MONITOR_TM_WHEEL_SIZE;
	timer->wh.pos = (timer->monitor->tm.pos + delay) %	\
		FLM__MONITOR_TM_WHEEL_SIZE;

	TAILQ_INSERT_TAIL (&(monitor->tm.wheel[timer->wh.pos]), timer, wh.entries);
	flm_Retain (FLM_OBJ (timer));

	monitor->count++;

	return ;
}

void
flm__MonitorTimerDelete (flm_Monitor *	monitor,
			 flm_Timer *	timer)
{
	TAILQ_REMOVE (&(monitor->tm.wheel[timer->wh.pos]), timer, wh.entries);
	flm_Release (FLM_OBJ (timer));

	monitor->count--;

	return ;
}

void
flm__MonitorTimerReset (flm_Monitor *	monitor,
			flm_Timer *	timer,
			uint32_t	delay)
{
	TAILQ_REMOVE (&(timer->monitor->tm.wheel[timer->wh.pos]), timer, wh.entries);

	timer->wh.rounds = delay / FLM__MONITOR_TM_WHEEL_SIZE;
	timer->wh.pos = (timer->monitor->tm.pos + delay) %	\
		FLM__MONITOR_TM_WHEEL_SIZE;

	TAILQ_INSERT_TAIL (&(monitor->tm.wheel[timer->wh.pos]), timer, wh.entries);
	return ;
}
