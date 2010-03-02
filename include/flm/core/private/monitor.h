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

#include "flm/core/private/obj.h"
#include "flm/core/private/io.h"
#include "flm/core/private/map.h"

#define FLM__TYPE_MONITOR	0x000D0000

typedef int (*flm__MonitorAdd_f) (flm_Monitor * monitor, flm_IO * io);
typedef int (*flm__MonitorDel_f) (flm_Monitor * monitor, flm_IO * io);
typedef int (*flm__MonitorWait_f) (flm_Monitor * monitor);

struct flm__MonitorTo
{
	flm_IO *			io;
	unsigned int			count;
	LIST_ENTRY (flm__MonitorTo)	entries;
};

struct flm__MonitorElem
{
	flm_IO *			io;
	void *				data;
	struct flm__MonitorTo *		to;
};

struct flm_Monitor
{
	struct flm_Obj				obj;

	flm__MonitorAdd_f			add;
	flm__MonitorDel_f			del;
	flm__MonitorWait_f			wait;

	struct {
		flm__Map *			map;
	} io;

	struct {
		/* current time */
		struct timespec			current;
		/* milliseconds before next timeout */
		int				next;
	} tm;

	struct {
		/* current position in the timeout array */
		size_t				current;
		/* array of timeouts, each filterent is a second */
		flm__Map *			wheel;
		/* number of timeouts set */
		size_t				count;
	} to;
};

#define FLM_MONITOR__TIMER_WHEEL_SIZE	256

int
flm__MonitorInit (flm_Monitor * monitor);

void
flm__MonitorPerfDestruct (flm_Monitor * monitor);

int
flm__MonitorAdd (flm_Monitor * monitor, flm_IO * io);

int
flm__MonitorDel (flm_Monitor * monitor, flm_IO * io);

#endif /* !_FLM_CORE_PRIVATE_MONITOR_H_ */
