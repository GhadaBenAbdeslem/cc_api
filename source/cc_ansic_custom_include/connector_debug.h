/*
* Copyright (c) 2017 Digi International Inc.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
* REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
* INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
* LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
* OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
* PERFORMANCE OF THIS SOFTWARE.
*
* Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#ifndef CONNECTOR_DEBUG_H_
#define CONNECTOR_DEBUG_H_

#include "connector_config.h"

#if (defined CONNECTOR_DEBUG)

#include <assert.h>
#include <stdarg.h>

typedef enum {
    debug_beg,
    debug_mid,
    debug_end,
    debug_all
} debug_t;

/**
 * Debug output from Cloud Connector, Writes a formatted string to stdout, expanding the format
 * tags with the value of the argument list arg.  This function behaves exactly as
 * vprintf() except an additional argument is passed indicating which part of the line is represented.
 *
 * debug_beg: Start of line. A fixed string could be added to each line to distinguish differing
 * types of output or to enable locking of the output until debug_eol is sent (too keep content from
 * breaking up in the middle). If format is NULL, there is no content.
 *
 * debug_mid: Middle of line. No header or trailer should be added.
 *
 * debug_end: End of line. A trailer may be added, but a newline MUST be added. At the point the message
 * is complete and if locking is enabled the unlock should be here. If format is NULL, there is no content.
 *
 * debug_all: The full message is contained in this one call. Equivalent to calling:
 *
 *      connector_debug_vprintf(debug_beg, format, args);
 *      connector_debug_vprintf(debug_end, "", args);
 *
 */

void connector_debug_vprintf(debug_t const debug, char const * const format, va_list args);

/**
* @defgroup DEBUG_MACROS User Defined Debug Macros
* @{
*/
/**
 *  Verify that the condition is true, otherwise halt the program.
 */
#define ASSERT(cond)        assert(cond)
/**
* @}
*/

#else

#define ASSERT(cond)
#endif

/**
 * Compile time assertion of functional state (limits, range checking, etc.)
 *
 *   Failure will emit a compiler-specific error
 *           gcc: 'duplicate case value'
 *   Example:
 *           CONFIRM(sizeof (int) == 4);
 *           CONFIRM(CHAR_BIT == 8);
 *           CONFIRM(ElementCount(array) == array_item_count);
 */
#define CONFIRM(cond)           do { switch(0) {case 0: case (cond):;} } while (0)


#endif
