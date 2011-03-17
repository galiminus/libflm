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

/**
 * \brief The main event loop object
 */

/**
 * \file monitor.h
 * \c A monitor handle Input/Ouput, threads and timer. It is basically
 * an event loop waiting from kernel notifications, and using the best
 * supported I/O notification framework available on the
 * system. Currently, only epoll(7) and select(2) are supported.
 */

#ifndef _FLM_CORE_PUBLIC_MONITOR_H_
# define _FLM_CORE_PUBLIC_MONITOR_H_

#ifndef _FLM__SKIP

typedef struct flm_Monitor flm_Monitor;

#include "flm/core/public/obj.h"

#endif /* !_FLM__SKIP */

/**
 * \brief Create a new flm_Monitor object.
 *
 * \remark Monitors are not thread-safe, using the same monitor in
 * multiple threads will probably lead to an application crash. But it is
 * possible to create as many monitor as needed, one for each thread.
 *
 * \return A pointer to a new flm_Monitor object.
 */
flm_Monitor *
flm_MonitorNew ();

/**
 * \brief Wait for events.
 *
 * \remark This function returns in case of error, when there is nothing to
 * monitor or when all the monitored objects are closed and all timers
 * elapsed.
 *
 * \param monitor A pointer to a flm_Monitor object.
 *
 * \par Example of IO monitoring
 * \code
 * flm_Monitor *        monitor;
 * flm_IO *             io;
 * int                  fd;
 *
 * monitor = flm_MonitorNew ();
 * 
 * fd = open ("/dev/zero", O_RDONLY);
 * io = flm_IONew (monitor, fd, NULL);
 * flm_IORead (io, my_read_handler);
 * flm_IORelease (io);
 *
 * flm_MonitorWait (monitor);
 * \endcode
 */
int
flm_MonitorWait (flm_Monitor * monitor);

/**
 * \brief Increment the reference counter.
 *
 * \param monitor A pointer to a flm_Monitor object.
 * \return The same pointer, this function cannot fail.
 */
flm_Monitor *
flm_MonitorRetain (flm_Monitor * monitor);

/**
 * \brief Decrement the reference counter.
 *
 * When the counter reaches zero all the internals references to monitored
 * IO objects, timers and threads will be released and those objects will
 * be freed if there is no more references to them.
 * 
 * \remark The underlying file descriptor of the IO objects will not be
 * automatically closed.
 *
 * \param monitor A pointer to a flm_Monitor object.
 */
void
flm_MonitorRelease (flm_Monitor * monitor);

/**
 * \example "Monitor unit tests"
 * \include monitor_test.c
 */

#endif /* !_FLM_CORE_PUBLIC_MONITOR_H_ */
