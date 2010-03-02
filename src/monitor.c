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
	while (monitor->wait (monitor) != -1)
		;
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

	monitor->to.wheel = flm__MapNew (FLM_MONITOR__TIMER_WHEEL_SIZE);
	if (monitor->to.wheel == NULL) {
		goto release_io_map;
	}
	monitor->to.current	=	0;
	monitor->to.count	=	0;

	if (clock_gettime (CLOCK_MONOTONIC, &(monitor->tm.current)) == -1) {
		goto release_to_wheel;
	}
	monitor->tm.next = 0;
	return (0);

release_to_wheel:
	flm__Release (FLM_OBJ (monitor->to.wheel));
release_io_map:
	flm__Release (FLM_OBJ (monitor->io.map));
error:
	return (-1);
}

void
flm__MonitorPerfDestruct (flm_Monitor * monitor)
{
	struct flm__MonitorElem * filter;
	size_t index;

	FLM_MAP_FOREACH (monitor->io.map, filter, index) {
		if (filter) {
			flm__MonitorDel (monitor, filter->io);
		}
	}
	flm__Release (FLM_OBJ (monitor->io.map));

	/* free to */
	flm__Release (FLM_OBJ (monitor->to.wheel));
	return ;
}

int
flm__MonitorAdd (flm_Monitor * monitor,
		 flm_IO * io)
{
	struct flm__MonitorElem * filter;

	if ((filter = flm_SlabAlloc (sizeof (struct flm__MonitorElem))) == NULL) {
		goto error;
	}

	filter->io = flm__Retain (FLM_OBJ (io));
	filter->to = NULL;

	if (flm__MapSet (monitor->io.map, io->sys.fd, filter) == -1) {
		goto free_filter;
	}

	if (monitor->add && monitor->add (monitor, io) == -1) {
		goto unset_map;
	}
	return (0);

unset_map:
	flm__MapRemove (monitor->io.map, io->sys.fd);
free_filter:
	flm__Release (FLM_OBJ (filter->io));
	flm__Release (FLM_OBJ (filter->data));
	flm_SlabFree (filter);
error:
	return (-1);
}

int
flm__MonitorDel (flm_Monitor * monitor,
		 flm_IO * io)
{
	struct flm__MonitorElem * filter;

	if (monitor->del) {
		monitor->del (monitor, io);
	}
	if ((filter = flm__MapRemove (monitor->io.map, io->sys.fd))) {
		flm__Release (FLM_OBJ (filter->io));
		flm_SlabFree (filter);
	}
	return (0);
}
