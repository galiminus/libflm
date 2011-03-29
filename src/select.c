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

/* According to POSIX.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <errno.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/error.h"
#include "flm/core/private/io.h"
#include "flm/core/private/monitor.h"
#include "flm/core/private/select.h"

static int
flm__SelectPerfAdd (flm__Select * select, flm_IO * io);

static int
flm__SelectPerfDel (flm__Select * select, flm_IO * io);

static int
flm__SelectPerfWait (flm__Select * select);

int (*selectHandler) (int, fd_set *, fd_set *, fd_set *, struct timeval *);

void
flm__setSelectHandler (int (*handler) (int, fd_set *, fd_set *, fd_set *, struct timeval *))
{
    selectHandler = handler;
}

flm__Select *
flm__SelectNew ()
{
    flm__Select * select;

    select = flm__Alloc (sizeof (flm__Select));
    if (select == NULL) {
        flm__Error = FLM_ERR_NOMEM;
        return (NULL);
    }
    if (flm__SelectInit (select) == -1) {
        flm__Free (select);
        return (NULL);
    }
    return (select);
}

int
flm__SelectInit (flm__Select * _select)
{
    if (flm__MonitorInit ((flm_Monitor *) _select) == -1) {
        return (-1);
    }

    ((flm_Monitor *)(_select))->add	=       \
        (flm__MonitorAdd_f) flm__SelectPerfAdd;

    ((flm_Monitor *)(_select))->del	=       \
        (flm__MonitorAdd_f) flm__SelectPerfDel;
    
    ((flm_Monitor *)(_select))->wait	=       \
        (flm__MonitorWait_f) flm__SelectPerfWait;
    
    memset (_select->ios, 0, sizeof (_select->ios));

    if (selectHandler == NULL) {
        flm__setSelectHandler (select);
    }
    
    return (0);
}

int
flm__SelectPerfAdd (flm__Select *       select,
		    flm_IO *            io)
{
    if (io->sys.fd >= FD_SETSIZE) {
        return (-1);
    }
    select->ios[io->sys.fd] = io;
    return (0);
}

int
flm__SelectPerfDel (flm__Select *       select,
                    flm_IO *            io)
{
    if (io->sys.fd >= FD_SETSIZE) {
        return (-1);
    }
    select->ios[io->sys.fd] = NULL;
    return (0);
}

int
flm__SelectPerfWait (flm__Select * _select)
{
    flm_IO *                io;
    size_t                  fd;
    int                     ret;
    struct timeval          delay;
    struct timeval *        delay_ptr;

    fd_set                  rset;
    fd_set                  wset;
    int                     max;

    if (((flm_Monitor *)(_select))->tm.next == -1) {
        delay_ptr = NULL;
    }
    else if (((flm_Monitor *)(_select))->tm.next >= 1000) {
        delay.tv_sec = ((flm_Monitor *)(_select))->tm.next / 1000;
        delay.tv_usec = (((flm_Monitor *)(_select))->tm.next * 1000) % 1000;
        delay_ptr = &delay;
    }
    else {
        delay.tv_sec = 0;
        delay.tv_usec = ((flm_Monitor *)(_select))->tm.next * 1000;
        delay_ptr = &delay;
    }

    max = -1;
    FD_ZERO (&rset);
    FD_ZERO (&wset);
    for (fd = 0; fd < FD_SETSIZE; fd++) {
        if ((io = _select->ios[fd]) == NULL) {
            continue ;
        }
        if (io->cl.shutdown && !io->wr.want && !io->cl.closed) {
            flm__IOClose (io, (flm_Monitor *) _select);
            continue ;
        }
        if (io->rd.want) {
            FD_SET (io->sys.fd, &rset);
        }
        if (io->wr.want) {
            FD_SET (io->sys.fd, &wset);
        }

        if (io->sys.fd > max) {
            max = io->sys.fd;
        }
    }

    if (max == -1 && delay_ptr == NULL) {
        return (0);
    }

    for (;;) {
        ret = selectHandler (max + 1, &rset, &wset, NULL, delay_ptr);
        if (ret >= 0) {
            break ;
        }
        if (errno == EINTR) {
            continue ;
        }
        flm__Error = FLM_ERR_ERRNO;
        return (-1); /* fatal error */
    }

    for (fd = 0; fd < FD_SETSIZE; fd++) {
        if ((io = _select->ios[fd]) == NULL) {
            continue ;
        }

        if ((int)fd > max) {
            break ;
        }

        if (fd >= FD_SETSIZE) {
            break ;
        }

        if (FD_ISSET (fd, &rset)) {
            flm__IORead (io, (flm_Monitor *) _select);
        }
        if (FD_ISSET (fd, &wset)) {
            flm__IOWrite (io, (flm_Monitor *) _select);
        }
    }
    return (0);
}
