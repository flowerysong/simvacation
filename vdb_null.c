/*
 * Copyright (c) 2014-2016 Regents of The University of Michigan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <time.h>

#include "simvacation.h"
#include "vdb_null.h"

struct vdb *
vdb_init(ucl_object_t *config, char *rcpt) {
    return (calloc(1, 1));
}

void
vdb_close(struct vdb *vdb) {
    free(vdb);
    return;
}

int
vdb_recent(struct vdb *vdb, char *from) {
    return (0);
}

int
vdb_store_interval(struct vdb *vdb, time_t interval) {
    return (0);
}

int
vdb_store_reply(struct vdb *vdb, char *from) {
    return (0);
}

struct name_list *
vdb_get_names(struct vdb *vdb) {
    return (NULL);
}

void
vdb_clean(struct vdb *vdb, char *user) {
    return;
}

void
vdb_gc(struct vdb *vdb) {
    return;
}
