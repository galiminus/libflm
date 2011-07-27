/*
 * Copyright (c) 2010-2011, Victor Goya <phorque@libflm.me>
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
	      void *            state,
	      uint32_t		delay)
{
    flm_Timer * timer;

    timer = flm__Alloc (sizeof (flm_Timer));
    if (timer == NULL) {

        return (NULL);
    }
    flm__TimerInit (timer, monitor, handler, state, delay);
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

flm_Timer *
flm_TimerRetain (flm_Timer *    timer)
{
    return (flm__Retain (&timer->obj));
}

void
flm_TimerRelease (flm_Timer *    timer)
{
    flm__Release (&timer->obj);
    return ;
}

void
flm__TimerInit (flm_Timer *		timer,
		flm_Monitor *		monitor,
		flm_TimerHandler	handler,
		void *                  state,
		uint32_t		delay)
{
    flm__ObjInit (&timer->obj);

    timer->obj.type = FLM__TYPE_TIMER;

    timer->handler = handler;
    timer->state = state;
    timer->monitor = monitor;
    timer->set = false;

    flm_TimerReset (timer, delay);

    return ;
}
