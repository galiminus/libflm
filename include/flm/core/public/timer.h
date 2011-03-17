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

#ifndef _FLM_CORE_PUBLIC_TIMER_H_
# define _FLM_CORE_PUBLIC_TIMER_H_

#ifndef _FLM__SKIP

#include <stdint.h>

typedef struct flm_Timer flm_Timer;

#include "flm/core/public/monitor.h"

#endif /* !_FLM__SKIP */

/**
 * A timer handler is a callback function that take the timer itself
 * as its first argument, and an user-defined state set at the timer
 * creation (see the flm_TimerCreate() function).
 */
typedef void (*flm_TimerHandler) (flm_Timer * timer, void * state);

#include "flm/core/public/obj.h"

/**
 * \brief Create a new timer
 *
 * The timer handler will be called after delay (in milliseconds) is
 * elapsed. The handler will be called just once.
 *
 * \remark There is no guaranty that the handler will be called
 * exactly at the specified delay. In fact, libflm rounds the delay
 * to the neared 1/10 second.
 *
 * \param monitor A pointer to the monitor that will handle the timer.
 * \param handler The timer handler that will be called after the
 * delay is elapsed.
 * \param state An user-defined state that will be given back to the
 * timer handler.
 * \param delay A delay, in millisecond.
 *
 * \return A pointer to a new flm_Timer object.
 *
 * \code
 *  flm_Monitor * monitor;
 *  flm_Timer * timer;
 *
 *  monitor = flm_MonitorNew ();
 *  timer = flm_TimerNew (monitor, my_handler, my_state, 2000);
 *  flm_TimerRelease (timer);
 * \endcode
 */
flm_Timer *
flm_TimerNew (flm_Monitor *	monitor,
	      flm_TimerHandler	handler,
	      void *            state,
	      uint32_t		delay);

/**
 * \brief Reset the timer
 *
 * The scheduled delay will be canceled and replaced with the new
 * one. It is possible to reset a timer inside a timer handler, this
 * can be useful to repeat the timer indefinitly.
 *
 * \param timer A pointer to a flm_Timer object.
 * \param delay A delay, in millisecond.
 *
 * \return Nothing, this function cannot fail.
 *
 * \code
 *  flm_Monitor * monitor;
 *  flm_Timer * timer;
 *
 *  monitor = flm_MonitorNew ();
 *  timer = flm_TimerNew (monitor, my_handler, my_state, 2000);
 *  flm_TimerReset (timer, 4000);
 *  flm_TimerRelease (timer);
 * \endcode
 */
void
flm_TimerReset (flm_Timer *	timer,
		uint32_t	delay);

/**
 * \brief Cancel the timer
 *
 * The scheduled delay will be canceled. The monitor's reference
 * to the timer will be released and, if there is no more references,
 * the timer will be freed.
 *
 * \param timer A pointer to a flm_Timer object.
 *
 * \code
 *  flm_Monitor * monitor;
 *  flm_Timer * timer;
 *
 *  monitor = flm_MonitorNew ();
 *  timer = flm_TimerNew (monitor, my_handler, my_state, 2000);
 *  flm_TimerCancel (timer);
 *  flm_TimerRelease (timer);
 * \endcode
 */
void
flm_TimerCancel (flm_Timer *	timer);

/**
 * \brief Increment the reference counter.
 *
 * \param monitor A pointer to a flm_Timer object.
 * \return The same pointer, this function cannot fail.
 */
flm_Timer *
flm_TimerRetain (flm_Timer *    timer);

/**
 * \brief Decrement the reference counter.
 *
 * When the counter reaches zero the timer will be freed.
 * 
 * \remark the monitor used for the timer creation will keep a reference to
 * the timer until the timer is either elapsed or cancelled. It means
 * that you can release the timer immediatly after creating it, since it
 * will be freed automatically when the monitor will release its own
 * reference to the timer.
 *
 * \param monitor A pointer to a monitor object.
 */
void
flm_TimerRelease (flm_Timer *   timer);

/**
 * \example "Timer unit tests"
 * \include timer_test.c
 */

#endif /* !_FLM_CORE_PUBLIC_TIMER_H_ */
