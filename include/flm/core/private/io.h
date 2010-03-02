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

#include "flm/core/private/filter.h"

#define FLM__TYPE_IO	0x000A0000

typedef void	(*flm__IOSysRead_f)			\
(flm_IO * io, flm_Monitor * monitor, uint8_t count);

typedef void	(*flm__IOSysWrite_f)			\
(flm_IO * io, flm_Monitor * monitor, uint8_t count);

typedef void	(*flm__IOSysClose_f)			\
(flm_IO * io, flm_Monitor * monitor);

typedef void (*flm__IOCloseHandler)			\
(flm_IO * io, flm_Monitor * monitor, void * data);

typedef void (*flm__IOErrorHandler)			\
(flm_IO * io, flm_Monitor * monitor, void * data);

struct flm_IO
{
	/* inheritance */
	struct flm_Filter			filter;

	struct {
		int				fd;
	} sys;

	struct {
		bool				can;
		bool				want;
		uint8_t				limit;
	} rd;
	struct {
		bool				can;
		bool				want;
		uint8_t				limit;
	} wr;
	struct {
		bool				shutdown;
		flm__IOCloseHandler		handler;
	} cl;
	struct {
		flm__IOErrorHandler		handler;
	} er;
	struct {
		void *				data;
	} user;

	/* IO methods */
	struct {
		flm__IOSysRead_f		read;
		flm__IOSysWrite_f		write;
		flm__IOSysClose_f		close;
	} perf;
};

#define FLM_IO_EVENT(io, type, monitor)					\
	if (io->type.handler) {						\
		io->type.handler (io, monitor, FLM_IO (io)->user.data);	\
	}

#define FLM_IO_EVENT_WITH(io, type, monitor, ...)			\
	if (io->type.handler) {						\
		io->type.handler (io,					\
				  monitor,				\
				  FLM_IO (io)->user.data,		\
				  __VA_ARGS__);				\
	}

int
flm__IOInit (flm_IO *			io,
	     flm_Monitor *		monitor,
	     flm__IOCloseHandler	cl_handler,
	     flm__IOErrorHandler	er_handler,
	     void *			data,
	     int			fd);

int
flm__IOInitSocket (flm_IO *		io,
		   flm_Monitor *	monitor,
		   flm__IOCloseHandler	cl_handler,
		   flm__IOErrorHandler	er_handler,
		   void *		data,
		   int			domain,
		   int			type);

void
flm__IOPerfDestruct (flm_IO * io);

int
flm__IOAccept (flm_IO * io);

int
flm__IOBind (flm_IO * io,
	     const char * interface,
	     int port,
	     int type);

int
flm__IOListen (flm_IO * io);

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
flm__IOPerfClose (flm_IO *	io,
		  flm_Monitor *	monitor);

#endif /* !_FLM_CORE_PRIVATE_IO_H_ */
