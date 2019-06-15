/*
 * simunvacation.c - clean up database files for people who are no longer
 * "on vacation".
 *
 * Copyright (c) 2004-2016 Regents of The University of Michigan.
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

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/time.h>
#include <syslog.h>
#include <unistd.h>

#include "simvacation.h"
#include "vdb.h"
#include "vlu.h"
#include "vutil.h"

void usage(void);

int
main(int argc, char **argv) {
    int          debug = 0;
    extern int   optind, opterr;
    extern char *optarg;
    char         ch;

    struct vlu *vlu;
    struct vdb *vdb;

    char *        config_file = CONFFILE;
    ucl_object_t *config;

    struct name_list *uniqnames;
    struct name_list *u;

    while ((ch = getopt(argc, argv, "c:d")) != EOF) {
        switch ((char)ch) {

        case 'c':
            config_file = optarg;
            break;

        case 'd':
            debug = 1;
            break;

        case '?':
        default:
            usage();
        }
    }

    if (debug) {
        openlog("simunvacation", LOG_NOWAIT | LOG_PERROR | LOG_PID,
                LOG_VACATION);
    } else {
        openlog("simunvacation", LOG_PID, LOG_VACATION);
    }

    if ((config = vacation_config(config_file)) == NULL) {
        exit(1);
    }

    vdb = vdb_init(config, "simunvacation");
    uniqnames = vdb_get_names(vdb);

    vlu = vlu_init(config);
    if (vlu_connect(vlu) != 0) {
        vdb_close(vdb);
        exit(1);
    }

    /*
     * For each candidate uniqname, check to see if user is
     * still on vacation.  If not, tell the database to clean them up.
     */
    for (u = uniqnames; u; u = u->next) {
        switch (vlu_search(vlu, u->name)) {
        case VLU_RESULT_PERMFAIL:
            syslog(LOG_INFO, "cleaning up %s", u->name);
            vdb_clean(vdb, u->name);
            break;
        case VLU_RESULT_TEMPFAIL:
            syslog(LOG_ERR, "lookup error processing %s", u->name);
            break;
        default:
            syslog(LOG_DEBUG, "leaving %s alone", u->name);
            break;
        }
    }

    /* Vacuum the database. */
    vdb_gc(vdb);

    vdb_close(vdb);
    vlu_close(vlu);
    exit(0);
}

void
usage(void) {
    fprintf(stderr, "usage: simunvacation [-v vbdir] [-s searchbase]\n");
    fprintf(stderr, "                [-h ldap_host] [-p ldap_port]\n");
    exit(1);
}
