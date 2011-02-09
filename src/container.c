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
#include "flm/core/private/container.h"

flm_Container *
flm_ContainerNew (void *                        content,
                  flm_ContainerFreeHandler      free_handler)
{
    flm_Container *         container;

    container = flm__Alloc (sizeof (flm_Container));
    if (container == NULL) {
        return (NULL);
    }
    if (flm__ContainerInit (container, content, free_handler) == -1) {
        flm__Free (container);
        return (NULL);
    }
    return (container);
}

void *
flm_ContainerContent (flm_Container *           container)
{
    return (container->content);
}

int
flm__ContainerInit (flm_Container *             container,
                    void *                      content,
                    flm_ContainerFreeHandler    free_handler)
{
    if (flm__ObjInit (FLM_OBJ (container)) == -1) {
        return (-1);
    }
    FLM_OBJ (container)->type = FLM__TYPE_CONTAINER;

    FLM_OBJ (container)->perf.destruct =                            \
        (flm__ObjPerfDestruct_f) flm__ContainerPerfDestruct;

    container->content = content;
    container->fr.handler = free_handler;

    return (0);
}

void
flm__ContainerPerfDestruct (flm_Container *     container)
{
    if (container->fr.handler) {
        container->fr.handler (flm_ContainerContent (container));
    }
    return ;
}
