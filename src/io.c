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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/error.h"
#include "flm/core/private/io.h"
#include "flm/core/private/obj.h"
#include "flm/core/private/monitor.h"

flm_IO *
flm_IONew (flm_Monitor *        monitor,
           int                  fd,
           void *               state)
{
    flm_IO * io;
    
    if ((io = flm__Alloc (sizeof (flm_IO))) == NULL) {
        return (NULL);
    }
    if (flm__IOInit (io, monitor, fd, state) == -1) {
        flm__Free (io);
        return (NULL);
    }
    return (io);
}

void
flm_IOShutdown (flm_IO *        io)
{
    io->rd.want = false;
    io->cl.shutdown = true;

    if (io->wr.want == false) {
        flm__IOClose (io, io->monitor);
    }
    return ;
}

void
flm_IOClose (flm_IO *           io)
{
    io->rd.can = false;
    io->rd.want = false;
    
    io->wr.can = false;
    io->wr.want = false;
    
    io->cl.shutdown = true;

    flm__IOClose (io, io->monitor);
    return ;
}

void
flm_IOOnRead (flm_IO *                  io,
              flm_IOReadHandler         handler)
{
    io->rd.handler = handler;
    return ;
}

void
flm_IOOnWrite (flm_IO *                 io,
               flm_IOWriteHandler       handler)
{
    io->wr.handler = handler;
    return ;
}

void
flm_IOOnClose (flm_IO *                 io,
               flm_IOCloseHandler       handler)
{
    io->cl.handler = handler;
    return ;
}

void
flm_IOOnError (flm_IO *                 io,
               flm_IOErrorHandler       handler)
{
    io->er.handler = handler;
    return ;
}

flm_IO *
flm_IORetain (flm_IO *                  io)
{
    return (flm__Retain ((flm_Obj *) io));
}

void
flm_IORelease (flm_IO *                 io)
{
    flm__Release ((flm_Obj *) io);
    return ;
}

int
flm__IOInit (flm_IO *                   io,
             flm_Monitor *              monitor,
             int                        fd,
             void *                     state)
{
    flm__ObjInit ((flm_Obj *) io);

    ((flm_Obj *)(io))->perf.destruct =                  \
        (flm__ObjPerfDestruct_f) flm__IOPerfDestruct;
    
    io->state                   =       state;
    
    io->sys.fd                  =       fd;
    
    io->rd.can                  =       false;
    io->rd.want                 =       true;
    io->rd.limit                =       4;
    
    io->wr.can                  =       false;
    io->wr.want                 =       false;
    io->wr.limit                =       4;
    
    io->cl.shutdown             =       false;
    io->cl.closed               =       false;

    flm_IOOnRead (io, NULL);
    flm_IOOnWrite (io, NULL);
    flm_IOOnClose (io, NULL);
    flm_IOOnError (io, NULL);
    
    io->perf.read               =       flm__IOPerfRead;
    io->perf.write              =       flm__IOPerfWrite;
    io->perf.close              =       flm__IOPerfClose;
    
    io->monitor         =       monitor;
    if (io->monitor && flm__MonitorIOAdd (io->monitor, io) == -1) {
        return (-1);
    }
    
    return (0);
}

void
flm__IOPerfDestruct (flm_IO * io)
{
    int retry;

    /* ensure that a fd is really closed */
    retry = 0;
    while (close (io->sys.fd) == -1) {
        if (errno == EINTR) {
            continue ;
        }
        if (errno == EBADF) {
            break ;
        }
        if (retry == 3) { /* retry 2 times then give up */
            break ;
        }
        retry++;
    }
    return ;
}

uint8_t
flm__IORead (flm_IO *           io,
             flm_Monitor *      monitor)
{
    uint8_t count;

    for (count = 0; true; count++) {
        if (!io->rd.want) {
            break ;
        }
        if (io->perf.read) {
            io->perf.read (io, monitor, count + 1);
        }
        if (!io->rd.can) {
            break ;
        }
    }
    return (count);
}

uint8_t
flm__IOWrite (flm_IO *          io,
              flm_Monitor *     monitor)
{
    uint8_t count;

    for (count = 0; true; count++) {
        if (!io->wr.want) {
            break ;
        }
        if (io->perf.write) {
            io->perf.write (io, monitor, count + 1);
        }
        if (!io->wr.can) {
            break ;
        }
    }
    return (count);
}

void
flm__IOClose (flm_IO *          io,
              flm_Monitor *     monitor)
{
    if (io->perf.close) {
        io->perf.close (io, monitor);
        io->cl.closed = true;
    }
    return ;
}

void
flm__IOPerfRead (flm_IO *       io,
                 flm_Monitor *  _monitor,
                 uint8_t        _count)
{
    (void) _monitor;
    (void) _count;

    if (io->rd.handler) {
        io->rd.handler (io, io->state);
    }
    return ;
}

void
flm__IOPerfWrite (flm_IO *      io,
                  flm_Monitor *  _monitor,
                  uint8_t        _count)
{
    (void) _monitor;
    (void) _count;

    if (io->wr.handler) {
        io->wr.handler (io, io->state);
    }
    return ;
}

void
flm__IOPerfClose (flm_IO *      io,
                  flm_Monitor * monitor)
{
    if (io->cl.handler) {
        io->cl.handler (io, io->state);
    }
    flm__MonitorIODelete (monitor, io);
    return ;
}
