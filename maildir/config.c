/**
 * @file
 * Config used by libmaildir
 *
 * @authors
 * Copyright (C) 2020 Richard Russon <rich@flatcap.org>
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @page maildir_config Config used by Maildir
 *
 * Config used by libmaildir
 */

#include "config.h"
#include <stddef.h>
#include <config/lib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "mutt/lib.h"

/**
 * maildir_field_delimiter_validator - Validate the "maildir_field_delimiter" config variable - Implements ConfigDef::validator() - @ingroup cfg_def_validator
 *
 * Ensure maildir_field_delimiter is a single non-alphanumeric non-(-.\/) character.
 */
static int maildir_field_delimiter_validator(const struct ConfigSet *cs,
                                             const struct ConfigDef *cdef,
                                             intptr_t value, struct Buffer *err)
{
  const char *delim = (const char *) value;

  if (strlen(delim) != 1)
  {
    // L10N: maildir_field_delimiter is a config variable and shouldn't be translated
    buf_printf(err, _("maildir_field_delimiter must be exactly one character long"));
    return CSR_ERR_INVALID;
  }

  if (isalnum(*delim) || strchr("-.\\/", *delim))
  {
    // L10N: maildir_field_delimiter is a config variable and shouldn't be translated
    buf_printf(err, _("maildir_field_delimiter cannot be alphanumeric or '-.\\/'"));
    return CSR_ERR_INVALID;
  }

  static bool maildir_field_delimiter_changed = false;
  if (maildir_field_delimiter_changed)
  {
    // L10N: maildir_field_delimiter is a config variable and shouldn't be translated
    buf_printf(err, _("maildir_field_delimiter can only be set once"));
    return CSR_ERR_INVALID;
  }

  if (*delim != *(const char *) cdef->initial)
  {
    maildir_field_delimiter_changed = true;
  }

  return CSR_SUCCESS;
}

/**
 * MaildirVars - Config definitions for the Maildir library
 */
static struct ConfigDef MaildirVars[] = {
  // clang-format off
  { "check_new", DT_BOOL, true, 0, NULL,
    "(maildir,mh) Check for new mail while the mailbox is open"
  },
  { "maildir_check_cur", DT_BOOL, false, 0, NULL,
    "Check both 'new' and 'cur' directories for new mail"
  },
  { "maildir_field_delimiter", DT_STRING|DT_NOT_EMPTY, IP ":", 0, maildir_field_delimiter_validator,
    "Field delimiter to be used for maildir email files (default is colon, recommended alternative is semi-colon)"
  },
  { "maildir_trash", DT_BOOL, false, 0, NULL,
    "Use the maildir 'trashed' flag, rather than deleting"
  },
  { "mh_purge", DT_BOOL, false, 0, NULL,
    "Really delete files in MH mailboxes"
  },
  { "mh_seq_flagged", DT_STRING, IP "flagged", 0, NULL,
    "MH sequence for flagged message"
  },
  { "mh_seq_replied", DT_STRING, IP "replied", 0, NULL,
    "MH sequence to tag replied messages"
  },
  { "mh_seq_unseen", DT_STRING, IP "unseen", 0, NULL,
    "MH sequence for unseen messages"
  },
  { NULL },
  // clang-format on
};

#if defined(USE_HCACHE)
/**
 * MaildirVarsHcache - Config definitions for the Maildir header cache
 */
static struct ConfigDef MaildirVarsHcache[] = {
  // clang-format off
  { "maildir_header_cache_verify", DT_BOOL, true, 0, NULL,
    "Check for maildir changes when opening mailbox"
  },
  { NULL },
  // clang-format on
};
#endif

/**
 * config_init_maildir - Register maildir config variables - Implements ::module_init_config_t - @ingroup cfg_module_api
 */
bool config_init_maildir(struct ConfigSet *cs)
{
  bool rc = cs_register_variables(cs, MaildirVars, DT_NO_FLAGS);

#if defined(USE_HCACHE)
  rc |= cs_register_variables(cs, MaildirVarsHcache, DT_NO_FLAGS);
#endif

  return rc;
}
