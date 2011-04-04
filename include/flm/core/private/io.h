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

#ifndef _FLM_CORE_PRIVATE_IO_H_
# define _FLM_CORE_PRIVATE_IO_H_

#include <sys/queue.h>

#include <stdint.h>
#include <stdbool.h>

#include "flm/core/public/io.h"
#include "flm/core/public/monitor.h"

#include "flm/core/private/obj.h"

#define FLM__TYPE_IO	0x00050000

typedef void	(*flm__IOSysRead_f)			\
(flm_IO * io, flm_Monitor * monitor, uint8_t count);

typedef void	(*flm__IOSysWrite_f)			\
(flm_IO * io, flm_Monitor * monitor, uint8_t count);

typedef void	(*flm__IOSysClose_f)			\
(flm_IO * io, flm_Monitor * monitor);

struct flm_IO
{
    /* inheritance */
    struct flm_Obj              obj;

    void *  			state;

    flm_Monitor *		monitor;

    struct {
        int			fd;
    } sys;

    struct {
        bool			can;
        bool			want;
        uint8_t			limit;
        flm_IOReadHandler	handler;
    } rd;
    struct {
        bool			can;
        bool			want;
        uint8_t			limit;
        flm_IOWriteHandler	handler;
    } wr;
    struct {
        bool                    closed;
        bool			shutdown;
        flm_IOCloseHandler	handler;
    } cl;
    struct {
        flm_IOErrorHandler	handler;
    } er;

    /* IO methods */
    struct {
        flm__IOSysRead_f	read;
        flm__IOSysWrite_f	write;
        flm__IOSysClose_f	close;
    } perf;

    TAILQ_ENTRY (flm_IO)		entries;
};

int
flm__IOInit (flm_IO *			io,
	     flm_Monitor *		monitor,
	     int			fd,
	     void *		state);

void
flm__IOPerfDestruct (flm_IO *	io);


uint8_t
flm__IORead (flm_IO *		io,
	     flm_Monitor *	monitor);

uint8_t
flm__IOWrite (flm_IO *		io,
	      flm_Monitor *	monitor);

void
flm__IOClose (flm_IO *		io,
	      flm_Monitor *	monitor);

void
flm__IOPerfRead (flm_IO *	io,
                 flm_Monitor *	monitor,
                 uint8_t        count);

void
flm__IOPerfWrite (flm_IO *	io,
		  flm_Monitor *	monitor,
                  uint8_t       count);

void
flm__IOPerfClose (flm_IO *	io,
		  flm_Monitor *	monitor);

#endif /* !_FLM_CORE_PRIVATE_IO_H_ */
