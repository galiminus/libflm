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
#include <stdbool.h>

#include "flm/core/public/timer.h"

#include "flm/core/private/alloc.h"
#include "flm/core/private/monitor.h"

#include "flm/core/private/timer.h"

flm_Timer *
flm_TimerNew (flm_Monitor *	monitor,
	      flm_TimerHandler	handler,
	      void *		data,
	      uint32_t		delay)
{
	flm_Timer * timer;

	timer = flm__Alloc (sizeof (flm_Timer));
	if (timer == NULL) {
		return (NULL);
	}
	if (flm__TimerInit (timer, monitor, handler, data, delay) == -1) {
		flm__Free (timer);
		return (NULL);
	}
	return (timer);
}

void
flm_TimerReset (flm_Timer *	timer,
		uint32_t	delay)
{
	if (timer->set) {
		flm__MonitorTimerReset (timer->monitor, timer, delay);
	}
	else {
		timer->set = true;
		flm__MonitorTimerAdd (timer->monitor, timer, delay);
	}
	return ;
}

void
flm_TimerCancel (flm_Timer *	timer)
{
	if (timer->set) {
		timer->set = false;
		flm__MonitorTimerDelete (timer->monitor, timer);
	}
	return ;
}

int
flm__TimerInit (flm_Timer *		timer,
		flm_Monitor *		monitor,
		flm_TimerHandler	handler,
		void *			data,
		uint32_t		delay)
{
	if (flm__ObjInit (FLM_OBJ (timer)) == -1) {
		return (-1);
	}
	FLM_OBJ (timer)->type = FLM__TYPE_TIMER;

	timer->handler = handler;
	timer->data = data;
	timer->monitor = monitor;
	timer->set = false;

	printf ("CREATE\n");

	/* round delay to the upper second */
	delay = (delay + 1000) / 1000;

	flm_TimerReset (timer, delay);

	return (0);
}
