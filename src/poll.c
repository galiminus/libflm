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

#include <poll.h>

#include <errno.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/io.h"
#include "flm/core/private/poll.h"

static int
flm__PollPerfWait (flm__Poll * poll);

flm__Poll *
flm__PollNew ()
{
	flm__Poll * poll;

	poll = flm_SlabAlloc (sizeof (flm__Poll));
	if (poll == NULL) {
		return (NULL);
	}
	if (flm__PollInit (poll) == -1) {
		flm_SlabFree (poll);
		return (NULL);
	}
	return (poll);
}

int
flm__PollInit (flm__Poll * poll)
{
	if (flm__MonitorInit (FLM_MONITOR (poll)) == -1) {
		return (-1);
	}

	FLM_MONITOR (poll)->wait =				\
		(flm__MonitorWait_f) flm__PollPerfWait;

	return (0);
}

int
flm__PollPerfWait (flm__Poll * _poll)
{
	flm_IO * io;

	size_t index;
	size_t fd;

	struct pollfd * fds;

	int ret;

	fds = flm__MemAlloc (flm__MapSize (FLM_MONITOR (_poll)->io.map) * \
			     sizeof (struct pollfd));
	if (fds == NULL) {
		goto error;
	}

	fd = 0;
	FLM_MAP_FOREACH (FLM_MONITOR (_poll)->io.map, io, index) {
		if (io == NULL) {
			continue ;
		}
		fds[fd].fd = io->sys.fd;
		fds[fd].events = 0;
		if (io->rd.want) {
			fds[fd].events = POLLIN | POLLHUP;
		}
		if (io->wr.want) {
			fds[fd].events |= POLLOUT;
		}
		fd++;
	}

	for (;;) {
		ret = poll (fds, fd, FLM__MONITOR_TM_RES);
		if (ret >= 0) {
			break ;
		}
		if (errno == EINTR  ||		\
		    errno == EAGAIN ||		\
		    errno == EWOULDBLOCK) {
			continue ;
		}
		goto free_fds; /* fatal error */
	}

	while (fd--) {

		io = flm__MapGet (FLM_MONITOR (_poll)->io.map, fds[fd].fd);
		if (io == NULL) {
			continue ;
		}

		if (fds[fd].revents & (POLLIN | POLLHUP)) {
			flm__IORead (io, FLM_MONITOR (_poll));
		}
		if (fds[fd].revents & POLLOUT) {
			flm__IOWrite (io, FLM_MONITOR (_poll));
		}
		if (io->cl.shutdown && !io->wr.want) {
			flm__IOClose (io, FLM_MONITOR (_poll));
		}
	}

	flm__Free (fds);
	return (0);

free_fds:
	flm__Free (fds);
error:
	return (-1);
}
