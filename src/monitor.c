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
#include "flm/core/private/obj.h"
#include "flm/core/private/select.h"
#include "flm/core/private/timer.h"

#include "config.h"

int     (*clockGettimeHandler)(clockid_t, struct timespec *);

void
flm__setClockGettime (int (*handler)(clockid_t, struct timespec *))
{
    clockGettimeHandler = handler;
}

flm_Monitor *
flm_MonitorNew ()
{
    flm_Monitor * monitor;

    if (HAVE_EPOLL_CTL) {
        monitor = FLM_MONITOR (flm__EpollNew ());
    }
    else if (HAVE_SELECT) {
        monitor = FLM_MONITOR (flm__SelectNew ());
    }
    else {
        monitor = NULL;
    }

    if (monitor == NULL) {
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
    while (monitor->count > 0) {
        if (monitor->wait && monitor->wait (monitor)) {
            return (-1);
        }

        if (flm__MonitorTimerTick (monitor) == -1) {
            return (-1);
        }
    }
    return (0);
}

int
flm__MonitorInit (flm_Monitor * monitor)
{
    size_t count;

    flm__ObjInit (FLM_OBJ (monitor));

    FLM_OBJ (monitor)->type = FLM__TYPE_MONITOR;

    FLM_OBJ (monitor)->perf.destruct =                  \
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
    monitor->count          =       0;

    /**
     * Set default clock_gettime()
     */
    if (clockGettimeHandler == NULL) {
        flm__setClockGettime (clock_gettime);
    }

    /**
     * Set the current time for the timer wheel
     */
    if (clockGettimeHandler (CLOCK_MONOTONIC, &(monitor->tm.current)) == -1) {
        return (-1);
    }

    /**
     * How much milliseconds until the next timer has to be triggered
     */
    monitor->tm.next        =       -1;

    /**
     * Current tick
     */
    monitor->tm.pos         =       0;

    /**
     * Create all the timer wheel slots
     */
    for (count = 0; count < FLM__MONITOR_TM_WHEEL_SIZE; count++) {
        TAILQ_INIT (&(monitor->tm.wheel[count]));
    }
    return (0);
}

void
flm__MonitorPerfDestruct (flm_Monitor * monitor)
{
    size_t      pos;
    flm_Timer * timer;

    for (pos = 0; pos < FLM__MONITOR_TM_WHEEL_SIZE; pos++) {
        TAILQ_FOREACH (timer, &(monitor->tm.wheel[pos]), wh.entries) {
            flm__MonitorTimerDelete (monitor, timer);
        }
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
    flm_Retain (FLM_OBJ (io));
    monitor->count++;
    return (0);
}

void
flm__MonitorIODelete (flm_Monitor * monitor,
                      flm_IO * io)
{
    if (monitor->del) {
        monitor->del (monitor, io);
    }
    flm_Release (FLM_OBJ (io));
    monitor->count--;
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
        return (-1);
    }

    diff = (((current.tv_sec * 1000) +
             (current.tv_nsec / 1000000)) -
            ((monitor->tm.current.tv_sec * 1000) +
             (monitor->tm.current.tv_nsec / 1000000))) / FLM__MONITOR_TM_RES;

    curpos = monitor->tm.pos;

    monitor->tm.current = current;
    monitor->tm.pos = (curpos + diff) % FLM__MONITOR_TM_WHEEL_SIZE;

    for (pos = 0; pos <= diff; pos++) {
        TAILQ_FOREACH (timer,
                       &(monitor->tm.wheel[(curpos + pos) %
                                           FLM__MONITOR_TM_WHEEL_SIZE]),
                       wh.entries) {
            if (timer->wh.rounds > 0) {
                timer->wh.rounds--;
                continue ;
                
            }
            temp.wh.entries = timer->wh.entries;
            
            flm_Retain (FLM_OBJ (timer));
            flm_TimerCancel (timer);
            if (timer->handler) {
                timer->handler (timer, timer->state);
            }
            flm_Release (FLM_OBJ (timer));
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
     * Round the delay to match FLM__MONITOR_TM_RES
     */
    delay = msdelay / FLM__MONITOR_TM_RES;

    /**
     * Get the number of rounds needed
     */
    timer->wh.rounds = delay / FLM__MONITOR_TM_WHEEL_SIZE;

    /**
     * Get the position in the wheel
     */
    timer->wh.pos = (monitor->tm.pos + delay) %      \
        FLM__MONITOR_TM_WHEEL_SIZE;

    /**
     * And insert it
     */
    TAILQ_INSERT_TAIL (&(monitor->tm.wheel[timer->wh.pos]), timer, wh.entries);
    flm_Retain (FLM_OBJ (timer));

    monitor->count++;

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
    flm_Release (FLM_OBJ (timer));

    monitor->count--;

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
    flm_Retain (FLM_OBJ (timer));

    /**
     * Delete then add to move the timer to its new position
     */
    flm__MonitorTimerDelete (monitor, timer);
    flm__MonitorTimerAdd (monitor, timer, delay);

    /**
     * Release the timer since it is retained by the monitor
     */
    flm_Release (FLM_OBJ (timer));
    return ;
}

void
flm__MonitorTimerRearm (flm_Monitor * monitor)
{
    uint32_t            diff;

    /**
     * Read the wheel from now to find the next tick.
     * XXX: optimise it
     */
    for (diff = 0; diff < FLM__MONITOR_TM_WHEEL_SIZE; diff++) {
        if (TAILQ_FIRST(&(monitor->tm.wheel[(monitor->tm.pos + diff) %
                                            FLM__MONITOR_TM_WHEEL_SIZE]))) {
            break ;
        }
    }
    if (diff == FLM__MONITOR_TM_WHEEL_SIZE) {
        /**
         * The wheel is empty, wait indefinitly
         */
        monitor->tm.next = -1;
    }
    else {
        monitor->tm.next = diff * FLM__MONITOR_TM_RES;
    }
    return ;
}

