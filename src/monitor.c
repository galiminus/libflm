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

#include <stdlib.h>
#include <string.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/monitor.h"
#include "flm/core/private/epoll.h"
#include "flm/core/private/error.h"
#include "flm/core/private/obj.h"
#include "flm/core/private/select.h"
#include "flm/core/private/timer.h"

#include "config.h"

int     (*clockGettimeHandler)(clockid_t, struct timespec *);

size_t   _flm_MonitorDefaultTmSize = FLM__MONITOR_TM_WHEEL_SIZE;
uint32_t _flm_MonitorDefaultTmRes  = FLM__MONITOR_TM_RES;

enum flm__MonitorBackend _flm__MonitorBackend;

void
flm__setMonitorDefaultTmSize (size_t tm_size)
{
    _flm_MonitorDefaultTmSize = tm_size;
}

void
flm__setMonitorDefaultTmRes (uint32_t tm_res)
{
    _flm_MonitorDefaultTmRes = tm_res;
}

void
flm__setMonitorClockGettime (int (*handler)(clockid_t, struct timespec *))
{
    clockGettimeHandler = handler;
}

void
flm__setMonitorBackend (enum flm__MonitorBackend backend)
{
    _flm__MonitorBackend = backend;
}

flm_Monitor *
flm_MonitorNew ()
{
    flm_Monitor * monitor;

    if (_flm__MonitorBackend == FLM__MONITOR_BACKEND_AUTO) {
        if (HAVE_EPOLL_CTL) {
            _flm__MonitorBackend = FLM__MONITOR_BACKEND_EPOLL;
        }
        else if (HAVE_SELECT) {
            _flm__MonitorBackend = FLM__MONITOR_BACKEND_SELECT;
        }
        else {
            _flm__MonitorBackend = FLM__MONITOR_BACKEND_NONE;
        }
    }

    switch (_flm__MonitorBackend) {
    case FLM__MONITOR_BACKEND_EPOLL:
        monitor = &(flm__EpollNew ()->monitor);
        break ;
    case FLM__MONITOR_BACKEND_SELECT:
        monitor = &(flm__SelectNew ()->monitor);
        break ;
    default:
        flm__Error = FLM_ERR_NOSYS;
        return (NULL);
    }

    return (monitor);
}

int
flm_MonitorWait (flm_Monitor * monitor)
{
    /**
     * The monitor will wait until a fatal error occurs or there
     * is no more IO to wait for.
     */
    while ((monitor->tm.count + monitor->io.count) > 0) {
        if (monitor->wait && monitor->wait (monitor)) {
            return (-1);
        }

        if (flm__MonitorTimerTick (monitor) == -1) {
            return (-1);
        }
    }
    return (0);
}

flm_Monitor *
flm_MonitorRetain (flm_Monitor * monitor)
{
    return (flm__Retain (&monitor->obj));
}

void
flm_MonitorRelease (flm_Monitor * monitor)
{
    flm__Release (&monitor->obj);
    return ;
}

int
flm__MonitorInit (flm_Monitor * monitor)
{
    size_t count;

    flm__ObjInit (&monitor->obj);

    monitor->obj.type = FLM__TYPE_MONITOR;

    monitor->obj.perf.destruct =                     \
        (flm__ObjPerfDestruct_f) flm__MonitorPerfDestruct;

    /**
     * Action callbacks
     */
    monitor->add            =       NULL;
    monitor->del            =       NULL;
    monitor->reset          =       NULL;
    monitor->wait           =       NULL;

    /**
     * Number of monitored IO
     */
    monitor->io.count       =   0;

    /**
     * Number of monitored timers
     */
    monitor->tm.count       =   0;

    /**
     * Set default clock_gettime()
     */
    if (clockGettimeHandler == NULL) {
        flm__setMonitorClockGettime (clock_gettime);
    }

    /**
     * Set the current time for the timer wheel
     */
    if (clockGettimeHandler (CLOCK_MONOTONIC, &(monitor->tm.current)) == -1) {
        flm__Error = FLM_ERR_ERRNO;
        return (-1);
    }

    /**
     * How much milliseconds until the next timer has to be triggered
     */
    monitor->tm.next    =       -1;

    /**
     * Size of the timer wheel
     */
    monitor->tm.size    =       _flm_MonitorDefaultTmSize;

    /**
     * Resolution of the timer wheel
     */
    monitor->tm.res     =       _flm_MonitorDefaultTmRes;

    /**
     * Current tick
     */
    monitor->tm.pos     =       0;

    /**
     * Allocate the wheel itself
     */
    monitor->tm.wheel   =       flm__Alloc (sizeof (*monitor->tm.wheel) *
                                            monitor->tm.size);
    if (monitor->tm.wheel == NULL) {
        flm__Error = FLM_ERR_NOMEM;
        return (-1);
    }

    /**
     * Initialize IO list
     */
    TAILQ_INIT (&(monitor->io.list));

    /**
     * Create all the timer wheel slots
     */
    for (count = 0; count < monitor->tm.size; count++) {
        TAILQ_INIT (&(monitor->tm.wheel[count]));
    }
    return (0);
}

void
flm__MonitorPerfDestruct (flm_Monitor * monitor)
{
    size_t      pos;
    flm_Timer * timer;
    flm_IO *    io;

    for (pos = 0; pos < monitor->tm.size; pos++) {
        TAILQ_FOREACH (timer, &(monitor->tm.wheel[pos]), wh.entries) {
            flm__MonitorTimerDelete (monitor, timer);
        }
    }
    flm__Free (monitor->tm.wheel);

    TAILQ_FOREACH (io, &(monitor->io.list), entries) {
        flm__MonitorIODelete (monitor, io);
    }
    return ;
}

int
flm__MonitorIOAdd (flm_Monitor * monitor,
                   flm_IO * io)
{
    if (monitor->add && monitor->add (monitor, io) == -1) {
        return (-1);
    }

    TAILQ_INSERT_TAIL (&(monitor->io.list), io, entries);
    flm_IORetain (io);

    monitor->io.count++;
    return (0);
}

void
flm__MonitorIODelete (flm_Monitor * monitor,
                      flm_IO * io)
{
    if (monitor->del) {
        monitor->del (monitor, io);
    }

    TAILQ_REMOVE (&(monitor->io.list), io, entries);
    flm_IORelease (io);

    monitor->io.count--;
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
    struct timespec     current;
    uint32_t            diff;
    uint32_t            curpos;
    uint32_t            pos;

    flm_Timer *         timer;
    struct flm_Timer    temp;

    if (clockGettimeHandler (CLOCK_MONOTONIC, &current) == -1) {
        flm__Error = FLM_ERR_ERRNO;
        return (-1);
    }

    diff = (((current.tv_sec * 1000) +
             (current.tv_nsec / 1000000)) -
            ((monitor->tm.current.tv_sec * 1000) +
             (monitor->tm.current.tv_nsec / 1000000))) / monitor->tm.res;

    curpos = monitor->tm.pos;

    monitor->tm.current = current;
    monitor->tm.pos = (curpos + diff) % monitor->tm.size;

    for (pos = 0; pos <= diff; pos++) {
        TAILQ_FOREACH (timer,
                       &(monitor->tm.wheel[(curpos + pos) %
                                           monitor->tm.size]),
                       wh.entries) {
            if (timer->wh.rounds > 0) {
                timer->wh.rounds--;
                continue ;

            }
            temp.wh.entries = timer->wh.entries;

            flm_TimerRetain (timer);
            flm_TimerCancel (timer);
            if (timer->handler) {
                timer->handler (timer, timer->state);
            }
            flm_TimerRelease (timer);
            timer = &temp;
        }
    }
    flm__MonitorTimerRearm (monitor);
    return (0);
}

void
flm__MonitorTimerAdd (flm_Monitor *     monitor,
                      flm_Timer *       timer,
                      uint32_t          msdelay)
{
    uint32_t    delay;

    /**
     * Round the delay to match the timer wheel resolution
     */
    delay = msdelay / monitor->tm.res;

    /**
     * Get the number of rounds needed
     */
    timer->wh.rounds = delay / monitor->tm.size;

    /**
     * Get the position in the wheel
     */
    timer->wh.pos = (monitor->tm.pos + delay) %      \
        monitor->tm.size;

    /**
     * And insert it
     */
    TAILQ_INSERT_TAIL (&(monitor->tm.wheel[timer->wh.pos]), timer, wh.entries);
    flm_TimerRetain (timer);

    monitor->tm.count++;

    flm__MonitorTimerRearm (monitor);
    return ;
}

void
flm__MonitorTimerDelete (flm_Monitor *  monitor,
                         flm_Timer *    timer)
{
    /**
     * Remove from the wheel
     */

    TAILQ_REMOVE (&(monitor->tm.wheel[timer->wh.pos]), timer, wh.entries);
    flm_TimerRelease (timer);

    monitor->tm.count--;

    flm__MonitorTimerRearm (monitor);
    return ;
}

void
flm__MonitorTimerReset (flm_Monitor *   monitor,
                        flm_Timer *     timer,
                        uint32_t        delay)
{
    /**
     * Retain the timer to avoid its freeing by flm__MonitorTimerDelete()
     */
    flm_TimerRetain (timer);

    /**
     * Delete then add to move the timer to its new position
     */
    flm__MonitorTimerDelete (monitor, timer);
    flm__MonitorTimerAdd (monitor, timer, delay);

    /**
     * Release the timer since it was retained by the monitor
     */
    flm_TimerRelease (timer);
    return ;
}

void
flm__MonitorTimerRearm (flm_Monitor * monitor)
{
    uint32_t            diff;
    bool                empty;
    flm_Timer *         timer;

    /**
     * Read the wheel from now to find the next tick.
     * XXX: optimise it
     */
    empty = true;
    for (diff = 0; diff < monitor->tm.size; diff++) {
        timer = TAILQ_FIRST(&(monitor->tm.wheel[(monitor->tm.pos + diff) %
                                                monitor->tm.size]));
        if (timer) {
            empty = false;
            if (timer->wh.rounds == 0) {
                break ;
            }
        }
    }
    if (empty) {
        /**
         * Wait forever
         */
        monitor->tm.next = -1;
    }
    else {
        monitor->tm.next = diff * monitor->tm.res;
    }
    return ;
}
