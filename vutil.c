/*
 * Copyright (c) Regents of The University of Michigan
 * See COPYING.
 */

#include <syslog.h>
#include <unistd.h>

#include "embedded_config.h"
#include "simvacation.h"
#include "vutil.h"

ucl_object_t *
vacation_config(const char *config_file) {
    struct ucl_parser *parser;
    ucl_object_t *     config;

    parser = ucl_parser_new(
            UCL_PARSER_KEY_LOWERCASE | UCL_PARSER_NO_IMPLICIT_ARRAYS);

    if (!ucl_parser_add_string(parser, SIMVACATION_CONFIG_BASE, 0)) {
        syslog(LOG_ERR, "vacation_config: base UCL parsing failed");
        return NULL;
    }

    config = ucl_parser_get_object(parser);
    ucl_parser_free(parser);

    if (config_file == NULL) {
        config_file = "/etc/mail/simvacation.conf";
        if (access(config_file, F_OK) != 0) {
            syslog(LOG_INFO, "skipping config file loading, %s doesn't exist",
                    config_file);
            return config;
        }
    }

    parser = ucl_parser_new(
            UCL_PARSER_KEY_LOWERCASE | UCL_PARSER_NO_IMPLICIT_ARRAYS);
    ucl_parser_set_filevars(parser, config_file, false);
    if (!ucl_parser_add_file(parser, config_file)) {
        syslog(LOG_ERR, "vacation_config: UCL parsing failed");
        return NULL;
    }

    ucl_object_merge(config, ucl_parser_get_object(parser), false);
    ucl_parser_free(parser);

    return config;
}
