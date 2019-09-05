/*
 * Copyright (c) Regents of The University of Michigan
 * See COPYING.
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

    struct vlu_backend *vlu;
    VLU *               vluh = NULL;
    struct vdb_backend *vdb;
    VDB *               vdbh = NULL;

    char *        config_file = NULL;
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

    if ((vdb = vdb_backend(ucl_object_tostring(
                 ucl_object_lookup_path(config, "core.vdb")))) == NULL) {
        exit(1);
    }

    if ((vdbh = vdb->init(config, "simunvacation")) == NULL) {
        exit(1);
    }

    uniqnames = vdb->get_names(vdbh);

    if ((vlu = vlu_backend(ucl_object_tostring(
                 ucl_object_lookup_path(config, "core.vlu")))) == NULL) {
        vdb->close(vdbh);
        exit(1);
    }

    if ((vluh = vlu->init(config)) == NULL) {
        vdb->close(vdbh);
        exit(1);
    }

    /*
     * For each candidate uniqname, check to see if user is
     * still on vacation.  If not, tell the database to clean them up.
     */
    for (u = uniqnames; u; u = u->next) {
        switch (vlu->search(vluh, u->name)) {
        case VLU_RESULT_PERMFAIL:
            syslog(LOG_INFO, "cleaning up %s", u->name);
            vdb->clean(vdbh, u->name);
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
    vdb->gc(vdbh);

    vdb->close(vdbh);
    vlu->close(vluh);
    exit(0);
}

void
usage(void) {
    fprintf(stderr, "usage: simunvacation [-c config_file] [-d]\n");
    exit(1);
}
