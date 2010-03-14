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
#include "flm/core/private/map.h"
#include "flm/core/private/monitor.h"
#include "flm/core/private/epoll.h"
#include "flm/core/private/poll.h"
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

	if ((!backend || !strcmp (backend, "poll")) &&
	    (monitor = FLM_MONITOR (flm__PollNew ()))) {
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
	while (monitor->wait (monitor) != -1) {
		flm__MonitorTick (monitor);
	}
	return (0);
}

int
flm__MonitorInit (flm_Monitor * monitor)
{
	if (flm__ObjInit (FLM_OBJ (monitor)) == -1) {
		goto error;
	}
	FLM_OBJ (monitor)->type = FLM__TYPE_MONITOR;

	FLM_OBJ (monitor)->perf.destruct =			\
		(flm__ObjPerfDestruct_f) flm__MonitorPerfDestruct;

	monitor->add		=	NULL;
	monitor->del		=	NULL;
	monitor->wait		=	NULL;

	if ((monitor->io.map = flm__MapNew (0)) == NULL) {
		goto error;
	}

	if (clock_gettime (CLOCK_MONOTONIC, &(monitor->tm.current)) == -1) {
		goto release_io_map;
	}
	monitor->tm.next	=	0;
	monitor->tm.pos		=	0;
	memset (monitor->tm.wheel,					\
		0,							\
		FLM__MONITOR_TM_WHEEL_SIZE * sizeof (flm_Timer *));
	return (0);

release_io_map:
	flm__Release (FLM_OBJ (monitor->io.map));
error:
	return (-1);
}

void
flm__MonitorPerfDestruct (flm_Monitor * monitor)
{
	flm_IO * io;
	size_t index;

	FLM_MAP_FOREACH (monitor->io.map, io, index) {
		if (io) {
			flm__MonitorDel (monitor, io);
		}
	}
	flm__Release (FLM_OBJ (monitor->io.map));
	return ;
}

int
flm__MonitorAdd (flm_Monitor * monitor,
		 flm_IO * io)
{
	if (flm__MapSet (monitor->io.map,			\
			 io->sys.fd,				\
			 flm__Retain (FLM_OBJ (io))) == -1) {
		goto release_io;
	}

	if (monitor->add && monitor->add (monitor, io) == -1) {
		goto unset_map;
	}
	return (0);

unset_map:
	flm__MapRemove (monitor->io.map, io->sys.fd);
release_io:
	flm__Release (FLM_OBJ (io));
	return (-1);
}

int
flm__MonitorDel (flm_Monitor * monitor,
		 flm_IO * io)
{
	if (monitor->del) {
		monitor->del (monitor, io);
	}
	if (flm__MapRemove (monitor->io.map, io->sys.fd)) {
		flm__Release (FLM_OBJ (io));
	}
	return (0);
}


int
flm__MonitorTick (flm_Monitor * monitor)
{
	struct timespec current;
	struct timespec diff;
	uint64_t msec;
	uint64_t pos;

	flm_Timer * timer;

	/* update time */
	if (clock_gettime (CLOCK_MONOTONIC, &current) == -1) {
		return (-1);
	}

	/* get the time difference from the last time we checked */
	if ((current.tv_nsec - monitor->tm.current.tv_nsec) < 0) {
		diff.tv_sec = current.tv_sec - monitor->tm.current.tv_sec - 1;
		diff.tv_nsec = 1000000000 + current.tv_nsec -	\
			monitor->tm.current.tv_nsec;
	}
	else {
		diff.tv_sec = current.tv_sec - monitor->tm.current.tv_sec;
		diff.tv_nsec = current.tv_nsec - monitor->tm.current.tv_nsec;
	}

	msec = diff.tv_sec * 10 + (diff.tv_nsec + 1000000) / 100000000;

	for (pos = 0; pos < msec; pos++) {
		for (timer = monitor->tm.wheel[(monitor->tm.pos + pos) % \
					       FLM__MONITOR_TM_WHEEL_SIZE];
		     timer;
		     timer = timer->next) {
			if (timer->rounds > 0) {
				timer->rounds--;
				continue ;
			}
			if (timer->handler) {
				timer->handler (monitor, timer, NULL);
			}
		}
	}
	monitor->tm.current = current;
	monitor->tm.pos = (monitor->tm.pos + msec) %	\
		FLM__MONITOR_TM_WHEEL_SIZE;
	return (0);
}
