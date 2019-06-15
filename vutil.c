/*
 * Copyright (c) 2016 Regents of The University of Michigan
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

#include <syslog.h>

#include "simvacation.h"
#include "vutil.h"

ucl_object_t *
vacation_config(char *config_file) {
    struct ucl_parser *parser;
    const char *       err;

    parser = ucl_parser_new(UCL_PARSER_KEY_LOWERCASE);
    if (!ucl_parser_add_file(parser, config_file)) {
        syslog(LOG_ERR, "vacation_config: UCL parsing failed");
        return (NULL);
    }
    if ((err = ucl_parser_get_error(parser)) != NULL) {
        syslog(LOG_ERR, "vacation_config: libucl error: %s", err);
        return (NULL);
    }

    return (ucl_parser_get_object(parser));
}
