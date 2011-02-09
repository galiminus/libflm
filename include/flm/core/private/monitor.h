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

#ifndef _FLM_CORE_PRIVATE_MONITOR_H_
# define _FLM_CORE_PRIVATE_MONITOR_H_

#include <sys/queue.h>

#include <time.h>

#include "flm/core/public/monitor.h"
#include "flm/core/public/timer.h"

#include "flm/core/private/obj.h"
#include "flm/core/private/io.h"

#define FLM__TYPE_MONITOR	0x000D0000

typedef int (*flm__MonitorAdd_f) (flm_Monitor * monitor, flm_IO * io);
typedef int (*flm__MonitorDel_f) (flm_Monitor * monitor, flm_IO * io);
typedef int (*flm__MonitorReset_f) (flm_Monitor * monitor, flm_IO * io);
typedef int (*flm__MonitorWait_f) (flm_Monitor * monitor);

#define FLM__MONITOR_TM_WHEEL_SIZE	4096
#define FLM__MONITOR_TM_RES		100  /* milliseconds */

struct flm_Monitor
{
	struct flm_Obj				obj;

	flm__MonitorAdd_f			add;
	flm__MonitorDel_f			del;
	flm__MonitorReset_f			reset;
	flm__MonitorWait_f			wait;

	uint32_t				count;

	struct {
		/* current time */
		struct timespec			current;

		/* milliseconds before next timeout */
		int				next;

		/* current position in the timer wheel */
		size_t				pos;

		/* simple timer wheel */
		TAILQ_HEAD (tmwh, flm_Timer)	wheel[FLM__MONITOR_TM_WHEEL_SIZE];
	} tm;
};

int
flm__MonitorInit (flm_Monitor * monitor);

int
flm__MonitorIOAdd (flm_Monitor * monitor, flm_IO * io);

void
flm__MonitorIODelete (flm_Monitor * monitor, flm_IO * io);

int
flm__MonitorIOReset (flm_Monitor * monitor, flm_IO * io);

int
flm__MonitorTimerTick (flm_Monitor * monitor);

void
flm__MonitorTimerAdd (flm_Monitor *	monitor,
		      flm_Timer *	timer,
		      uint32_t		delay);

void
flm__MonitorTimerDelete (flm_Monitor *	monitor,
			 flm_Timer *	timer);

void
flm__MonitorTimerReset (flm_Monitor *	monitor,
			flm_Timer *	timer,
			uint32_t	delay);

#endif /* !_FLM_CORE_PRIVATE_MONITOR_H_ */
