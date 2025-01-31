/**
 * @file
 * Data shared between Index, Pager and Sidebar
 *
 * @authors
 * Copyright (C) 2021 Richard Russon <rich@flatcap.org>
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
 * @page index_shared_data Shared data
 *
 * Data shared between Index, Pager and Sidebar
 */

#include "config.h"
#include <stdbool.h>
#include "mutt/lib.h"
#include "email/lib.h"
#include "core/lib.h"
#include "shared_data.h"
#include "lib.h"
#include "mview.h"

/**
 * index_shared_data_set_mview - Set the MailboxView for the Index and friends
 * @param shared Shared Index data
 * @param mv     Mailbox View, may be NULL
 */
void index_shared_data_set_mview(struct IndexSharedData *shared, struct MailboxView *mv)
{
  if (!shared)
    return;

  NotifyIndex subtype = NT_INDEX_NO_FLAGS;

  if (shared->mailbox_view != mv)
  {
    shared->mailbox_view = mv;
    subtype |= NT_INDEX_MVIEW;
  }

  struct Mailbox *m = mview_mailbox(mv);
  if (shared->mailbox != m)
  {
    shared->mailbox = m;
    shared->email = NULL;
    shared->email_seq = 0;
    subtype |= NT_INDEX_MAILBOX | NT_INDEX_EMAIL;
  }

  struct Account *a = m ? m->account : NULL;
  if (shared->account != a)
  {
    shared->account = a;
    subtype |= NT_INDEX_ACCOUNT;
  }

  struct ConfigSubset *sub = NeoMutt->sub;
#if 0
  if (m)
    sub = m->sub;
  else if (a)
    sub = a->sub;
#endif
  if (shared->sub != sub)
  {
    shared->sub = sub;
    subtype |= NT_INDEX_SUBSET;
  }

  if (subtype != NT_INDEX_NO_FLAGS)
  {
    mutt_debug(LL_NOTIFY, "NT_INDEX: %p\n", (void *) shared);
    notify_send(shared->notify, NT_INDEX, subtype, shared);
  }
}

/**
 * index_shared_data_set_email - Set the current Email for the Index and friends
 * @param shared Shared Index data
 * @param e      Current Email, may be NULL
 */
void index_shared_data_set_email(struct IndexSharedData *shared, struct Email *e)
{
  if (!shared)
    return;

  size_t seq = e ? e->sequence : 0;
  if ((shared->email != e) || (shared->email_seq != seq))
  {
    shared->email = e;
    shared->email_seq = seq;

    mutt_debug(LL_NOTIFY, "NT_INDEX_EMAIL: %p\n", (void *) shared->email);
    notify_send(shared->notify, NT_INDEX, NT_INDEX_EMAIL, shared);
  }
}

/**
 * index_shared_data_is_cur_email - Check whether an email is the currently selected Email
 * @param shared Shared Index data
 * @param e      Email to check
 * @retval true  e is current
 * @retval false e is not current
 */
bool index_shared_data_is_cur_email(const struct IndexSharedData *shared,
                                    const struct Email *e)
{
  if (!shared)
    return false;

  return shared->email_seq == e->sequence;
}

/**
 * index_shared_data_free - Free Shared Index Data - Implements MuttWindow::wdata_free() - @ingroup window_wdata_free
 *
 * Only `notify` is owned by IndexSharedData and should be freed.
 */
void index_shared_data_free(struct MuttWindow *win, void **ptr)
{
  if (!ptr || !*ptr)
    return;

  struct IndexSharedData *shared = *ptr;

  mutt_debug(LL_NOTIFY, "NT_INDEX_DELETE: %p\n", (void *) shared);
  notify_send(shared->notify, NT_INDEX, NT_INDEX_DELETE, shared);
  notify_free(&shared->notify);

  FREE(ptr);
}

/**
 * index_shared_data_new - Create new Index Data
 * @retval ptr New IndexSharedData
 */
struct IndexSharedData *index_shared_data_new(void)
{
  struct IndexSharedData *shared = mutt_mem_calloc(1, sizeof(struct IndexSharedData));

  shared->notify = notify_new();
  shared->sub = NeoMutt->sub;

  mutt_debug(LL_NOTIFY, "NT_INDEX_ADD: %p\n", (void *) shared);
  notify_send(shared->notify, NT_INDEX, NT_INDEX_ADD, shared);

  return shared;
}
