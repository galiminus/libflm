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

#include <sys/epoll.h>

#include <errno.h>
#include <unistd.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/io.h"
#include "flm/core/private/epoll.h"
#include "flm/core/private/error.h"

static int
flm__EpollCtl (flm__Epoll * epoll, flm_IO * io, int op);

static int
flm__EpollPerfAdd (flm__Epoll * epoll, flm_IO * io);

static int
flm__EpollPerfDel (flm__Epoll * epoll, flm_IO * io);

static int
flm__EpollPerfReset (flm__Epoll * epoll, flm_IO * io);

static int
flm__EpollPerfWait (flm__Epoll * epoll);

int (*epollCreateHandler) (int);
int (*epollCtlHandler) (int, int, int, struct epoll_event *);
int (*epollWaitHandler) (int, struct epoll_event *, int, int);

void
flm__setEpollCreateHandler (int (*handler)(int))
{
    epollCreateHandler = handler;
}

void
flm__setEpollCtlHandler (int (*handler)(int, int, int, struct epoll_event *))
{
    epollCtlHandler = handler;
}

void
flm__setEpollWaitHandler (int (*handler) (int, struct epoll_event *, int, int))
{
    epollWaitHandler = handler;
}


flm__Epoll *
flm__EpollNew ()
{
    flm__Epoll * epoll;

    epoll = flm__Alloc (sizeof (flm__Epoll));
    if (epoll == NULL) {
        flm__Error = FLM_ERR_NOMEM;
        return (NULL);
    }
    if (flm__EpollInit (epoll) == -1) {
        flm__Free (epoll);
        return (NULL);
    }
    return (epoll);
}

int
flm__EpollInit (flm__Epoll * epoll)
{
    if (flm__MonitorInit ((flm_Monitor *) epoll) == -1) {
        goto error;
    }

    ((flm_Obj *)(epoll))->perf.destruct =               \
        (flm__ObjPerfDestruct_f) flm__EpollPerfDestruct;
    
    ((flm_Monitor *)(epoll))->add =                     \
        (flm__MonitorAdd_f) flm__EpollPerfAdd;

    ((flm_Monitor *)(epoll))->del =                     \
        (flm__MonitorDel_f) flm__EpollPerfDel;

    ((flm_Monitor *)(epoll))->reset =                   \
        (flm__MonitorDel_f) flm__EpollPerfReset;

    ((flm_Monitor *)(epoll))->wait =                    \
        (flm__MonitorWait_f) flm__EpollPerfWait;

    epoll->size = FLM__EPOLL_MAXEVENTS_DEFAULT;

    if (epollCreateHandler == NULL) {
        flm__setEpollCreateHandler (epoll_create);
    }

    if (epollCtlHandler == NULL) {
        flm__setEpollCtlHandler (epoll_ctl);
    }

    if (epollWaitHandler == NULL) {
        flm__setEpollWaitHandler (epoll_wait);
    }

    if ((epoll->epfd = epollCreateHandler (epoll->size)) == -1) {
        flm__Error = FLM_ERR_ERRNO;
        goto error;
    }

    epoll->events = flm__Alloc (epoll->size * sizeof (struct epoll_event));
    if (epoll->events == NULL) {
        flm__Error = FLM_ERR_NOMEM;
        goto close_epfd;
    }

    return (0);

  close_epfd:
    close (epoll->epfd);
  error:
    return (-1);
}

void
flm__EpollPerfDestruct (flm__Epoll * epoll)
{
    close (epoll->epfd);
    flm__Free (epoll->events);
    flm__MonitorPerfDestruct ((flm_Monitor *) epoll);
    return ;
}

int
flm__EpollCtl (flm__Epoll * epoll,
               flm_IO * io,
               int op)
{
    struct epoll_event event;

    event.data.u64 = 0; /* makes valgrind happy */
    event.data.ptr = io;

    event.events = EPOLLET | EPOLLIN | EPOLLRDHUP | EPOLLOUT;
    if (epollCtlHandler (epoll->epfd, op, io->sys.fd, &event) == -1) {
        flm__Error = FLM_ERR_ERRNO;
        return (-1);
    }
    return (0);
}

int
flm__EpollPerfAdd (flm__Epoll * epoll,
                   flm_IO * io)
{
    if (flm__EpollCtl (epoll, io, EPOLL_CTL_ADD) == -1) {
        flm__Error = FLM_ERR_ERRNO;
        return (-1);
    }
    return (0);
}

int
flm__EpollPerfDel (flm__Epoll * epoll,
                   flm_IO * io)
{
    if (flm__EpollCtl (epoll, io, EPOLL_CTL_DEL) == -1) {
        flm__Error = FLM_ERR_ERRNO;
        return (-1);
    }
    return (0);
}

int
flm__EpollPerfReset (flm__Epoll * epoll,
                     flm_IO * io)
{
    if (flm__EpollCtl (epoll, io, EPOLL_CTL_MOD) == -1) {
        flm__Error = FLM_ERR_ERRNO;
        return (-1);
    }
    return (0);
}

int
flm__EpollPerfWait (flm__Epoll * epoll)
{
    int ev_max;
    int ev_count;
    struct epoll_event * event;
    flm_IO * io;

    int ret;

    for (;;) {
        ev_max = epollWaitHandler (epoll->epfd,                       \
                                   epoll->events,                     \
                                   epoll->size,                       \
                                   ((flm_Monitor *)(epoll))->tm.next);
        if (ev_max >= 0) {
            break ;
        }
        if (errno == EINTR  ||          \
            errno == EAGAIN ||          \
            errno == EWOULDBLOCK) {
            continue ;
        }
        flm__Error = FLM_ERR_ERRNO;
        return (-1);
    }

    for (ev_count = 0; ev_count < ev_max; ev_count++) {
        event = &epoll->events[ev_count];
        if ((io = ((flm_IO *) (event->data.ptr))) == NULL) {
            continue ;
        }
        ret = 0;
        if ((event->events & (EPOLLIN | EPOLLRDHUP)) &&                    \
            flm__IORead (io, ((flm_Monitor *) epoll)) == io->rd.limit &&   \
            io->rd.can &&                                                  \
            flm__EpollPerfReset (epoll, io) == -1) {
            ret = -1;
        }
        if ((event->events & EPOLLOUT) &&                                  \
            flm__IOWrite (io, ((flm_Monitor *) epoll)) == io->wr.limit &&  \
            io->wr.can &&                                                  \
            flm__EpollPerfReset (epoll, io) == -1) {
            ret = -1;
        }

        if (io->cl.shutdown && !io->wr.want && !io->cl.closed) {
            flm__IOClose (io, ((flm_Monitor *) epoll));
        }
    }
    return (0);
}
