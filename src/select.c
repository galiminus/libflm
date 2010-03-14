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

/* According to POSIX.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/io.h"
#include "flm/core/private/monitor.h"
#include "flm/core/private/select.h"

static int
flm__SelectPerfAdd (flm__Select * select, flm_IO * io);

static int
flm__SelectPerfDel (flm__Select * select, flm_IO * io);

static int
flm__SelectPerfWait (flm__Select * select);

flm__Select *
flm__SelectNew ()
{
	flm__Select * select;

	select = flm_SlabAlloc (sizeof (flm__Select));
	if (select == NULL) {
		return (NULL);
	}
	if (flm__SelectInit (select) == -1) {
		flm_SlabFree (select);
		return (NULL);
	}
	return (select);
}

int
flm__SelectInit (flm__Select * select)
{
	if (flm__MonitorInit (FLM_MONITOR (select)) == -1) {
		return (-1);
	}

	FLM_MONITOR (select)->add	=				\
		(flm__MonitorAdd_f) flm__SelectPerfAdd;

	FLM_MONITOR (select)->del	=				\
		(flm__MonitorDel_f) flm__SelectPerfDel;

	FLM_MONITOR (select)->wait	=				\
		(flm__MonitorWait_f) flm__SelectPerfWait;

	return (0);
}

int
flm__SelectPerfAdd (flm__Select * select,
		    flm_IO * io)
{
	(void) select;

	if (io->sys.fd >= FD_SETSIZE) {
		return (-1);
	}
	return (0);
}

int
flm__SelectPerfDel (flm__Select * select,
		    flm_IO * io)
{
	(void) select;

	if (io->sys.fd >= FD_SETSIZE) {
		return (-1);
	}
	return (0);
}

int
flm__SelectPerfWait (flm__Select * _select)
{
	flm_IO * io;

	size_t index;
	int max_fd;

	fd_set rset;
	fd_set wset;

	int ret;

	FD_ZERO (&rset);
	FD_ZERO (&wset);

	max_fd = -1;
	FLM_MAP_FOREACH (FLM_MONITOR (_select)->io.map, io, index) {

		if (io == NULL) {
			continue ;
		}

		if (index >= FD_SETSIZE) {
			break ;
		}

		if (io->rd.want) {
			FD_SET (index, &rset);
		}
		if (io->wr.want) {
			FD_SET (index, &wset);
		}
		if (io->sys.fd > max_fd) {
			max_fd = io->sys.fd;
		}
	}

	for (;;) {
		ret = select (max_fd + 1, &rset, &wset, NULL, NULL);
		if (ret >= 0) {
			break ;
		}
		if (errno == EINTR) {
			continue ;
		}
		return (-1); /* fatal error */
	}

	FLM_MAP_FOREACH (FLM_MONITOR (_select)->io.map, io, index) {

		if (io == NULL) {
			continue ;
		}

		if (index >= FD_SETSIZE) {
			break ;
		}

		if (FD_ISSET (index, &rset)) {
			flm__IORead (io, FLM_MONITOR (_select));
		}
		if (FD_ISSET (index, &wset)) {
			flm__IOWrite (io, FLM_MONITOR (_select));
		}
		if (io->cl.shutdown && !io->wr.want) {
			flm__IOClose (io, FLM_MONITOR (_select));
		}
	}
	return (0);
}
