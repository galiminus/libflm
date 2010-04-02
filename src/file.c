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

#include <fcntl.h>
#include <string.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/filter.h"
#include "flm/core/private/file.h"
#include "flm/core/private/io.h"

flm_File *
flm_FileNew (int fd)
{
	flm_File * file;

	file = flm_SlabAlloc (sizeof (flm_File));
	if (file == NULL) {
		return (NULL);
	}
	if (flm__FileInit (file, fd) == -1) {
		flm_SlabFree (file);
		return (NULL);
	}
	return (file);
}

flm_File *
flm_FileOpen (const char * root,
	      const char * path,
	      const char * mode)
{
	flm_File * file;

	file = flm_SlabAlloc (sizeof (flm_File));
	if (file == NULL) {
		return (NULL);
	}
	if (flm__FileInitOpen (file, root, path, mode) == -1) {
		flm_SlabFree (file);
		return (NULL);
	}
	return (file);
}

int
flm__FileInit (flm_File * file,
	       int fd)
{
	/* a file is an IO for practical reason but doesn't share the
	   asynchronous aspect of the other kind of IO, here */
	if (flm__IOInit (FLM_IO (file),		\
			 NULL,			\
			 NULL,			\
			 NULL,			\
			 NULL,			\
			 NULL,			\
			 fd,			\
			 0) == -1) {
		return (-1);
	}
	FLM_OBJ (file)->type = FLM__TYPE_FILE;

	return (0);
}

int
flm__FileInitOpen (flm_File * file,
		   const char * root,
		   const char * path,
		   const char * mode)
{
	int fd;
	int flags;

	(void) root;

	if (!strcmp (mode, "r")) {
		flags = O_RDONLY;
	}
	else if (!strcmp (mode, "r+")) {
		flags = O_RDWR;
	}
	else if (!strcmp (mode, "w")) {
		flags = O_WRONLY | O_TRUNC | O_CREAT;
	}
	else if (!strcmp (mode, "w+")) {
		flags = O_RDWR | O_TRUNC | O_CREAT;
	}
	else if (!strcmp (mode, "a")) {
		flags = O_WRONLY | O_APPEND | O_CREAT;
	}
	else if (!strcmp (mode, "a+")) {
		flags = O_RDWR | O_APPEND | O_CREAT;
	}
	else {
		goto error;
	}

#ifdef O_CLOEXEC
	flags |= O_CLOEXEC;
#endif

	if (flags & O_CREAT) {
		fd = open (path, flags,	       \
			   S_IRUSR | S_IWUSR | \
			   S_IRGRP | S_IWGRP | \
			   S_IROTH | S_IWOTH);
	}
	else {
		fd = open (path, flags);
	}
	if (fd == -1) {
		goto error;
	}
	if (flm__FileInit (file, fd) == -1) {
		goto close_fd;
	}
	return (0);

close_fd:
	close (fd);
error:
	return (-1);
}
