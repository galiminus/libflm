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

#include <stdint.h>

#include "flm/core/public/timer.h"

#include "flm/core/private/alloc.h"
#include "flm/core/private/monitor.h"

#include "flm/core/private/timer.h"

flm_Timer *
flm_TimerNew (flm_Monitor *	monitor,
	      flm_TimerHandler	handler,
	      uint32_t		seconds)
{
	flm_Timer * timer;

	timer = flm_SlabAlloc (sizeof (flm_Timer));
	if (timer == NULL) {
		return (NULL);
	}
	if (flm__TimerInit (timer, monitor, handler, seconds) == -1) {
		flm_SlabFree (timer);
		return (NULL);
	}
	return (timer);
}

void
flm_TimerReset (flm_Timer *	timer,
		uint32_t	seconds)
{

}

void
flm_TimerCancel (flm_Timer *	timer)
{

}

int
flm__TimerInit (flm_Timer *		timer,
		flm_Monitor *		monitor,
		flm_TimerHandler	handler,
		uint32_t		seconds)
{
	uint32_t pos;

	if (flm__ObjInit (FLM_OBJ (timer)) == -1) {
		goto error;
	}
	FLM_OBJ (timer)->type = FLM__TYPE_TIMER;

	FLM_OBJ (timer)->perf.destruct =				\
		(flm__ObjPerfDestruct_f) flm__TimerPerfDestruct;

	timer->handler = handler;
	timer->rounds = seconds / FLM__MONITOR_TM_WHEEL_SIZE;

	pos = (monitor->tm.pos + seconds) % FLM__MONITOR_TM_WHEEL_SIZE;
	timer->next = monitor->tm.wheel[pos];
	monitor->tm.wheel[pos] = timer;

	return (0);

error:
	return (-1);
}

void
flm__TimerPerfDestruct (flm__Timer * timer)
{
	return ;
}
