/**
 * @file
 * GNU dbm backend for the key/value Store
 *
 * @authors
 * Copyright (C) 2004 Thomas Glanzmann <sithglan@stud.uni-erlangen.de>
 * Copyright (C) 2004 Tobias Werth <sitowert@stud.uni-erlangen.de>
 * Copyright (C) 2004 Brian Fundakowski Feldman <green@FreeBSD.org>
 * Copyright (C) 2016 Pietro Cerutti <gahr@gahr.ch>
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
 * @page store_gdbm GNU dbm (GDBM)
 *
 * GNU dbm backend for the key/value Store.
 * https://www.gnu.org.ua/software/gdbm/
 */

#include "config.h"
#include <stddef.h>
#include <gdbm.h>
#include <limits.h>
#include "mutt/lib.h"
#include "lib.h"

/**
 * store_gdbm_open - Implements StoreOps::open() - @ingroup store_open
 */
static StoreHandle *store_gdbm_open(const char *path)
{
  if (!path)
    return NULL;

  const int pagesize = 4096;

  GDBM_FILE db = gdbm_open((char *) path, pagesize, GDBM_WRCREAT, 00600, NULL);
  if (!db)
  {
    /* if rw failed try ro */
    db = gdbm_open((char *) path, pagesize, GDBM_READER, 00600, NULL);
  }

  // Return an opaque pointer
  return (StoreHandle *) db;
}

/**
 * store_gdbm_fetch - Implements StoreOps::fetch() - @ingroup store_fetch
 */
static void *store_gdbm_fetch(StoreHandle *store, const char *key, size_t klen, size_t *vlen)
{
  if (!store || (klen > INT_MAX))
    return NULL;

  datum dkey = { 0 };
  datum data = { 0 };

  // Decloak an opaque pointer
  GDBM_FILE db = store;

  dkey.dptr = (char *) key;
  dkey.dsize = klen;
  data = gdbm_fetch(db, dkey);

  *vlen = data.dsize;
  return data.dptr;
}

/**
 * store_gdbm_free - Implements StoreOps::free() - @ingroup store_free
 */
static void store_gdbm_free(StoreHandle *store, void **ptr)
{
  FREE(ptr);
}

/**
 * store_gdbm_store - Implements StoreOps::store() - @ingroup store_store
 */
static int store_gdbm_store(StoreHandle *store, const char *key, size_t klen,
                            void *value, size_t vlen)
{
  if (!store || (klen > INT_MAX) || (vlen > INT_MAX))
    return -1;

  datum dkey = { 0 };
  datum databuf = { 0 };

  // Decloak an opaque pointer
  GDBM_FILE db = store;

  dkey.dptr = (char *) key;
  dkey.dsize = klen;

  databuf.dsize = vlen;
  databuf.dptr = value;

  return gdbm_store(db, dkey, databuf, GDBM_REPLACE);
}

/**
 * store_gdbm_delete_record - Implements StoreOps::delete_record() - @ingroup store_delete_record
 */
static int store_gdbm_delete_record(StoreHandle *store, const char *key, size_t klen)
{
  if (!store || (klen > INT_MAX))
    return -1;

  datum dkey = { 0 };

  // Decloak an opaque pointer
  GDBM_FILE db = store;

  dkey.dptr = (char *) key;
  dkey.dsize = klen;

  return gdbm_delete(db, dkey);
}

/**
 * store_gdbm_close - Implements StoreOps::close() - @ingroup store_close
 */
static void store_gdbm_close(StoreHandle **ptr)
{
  if (!ptr || !*ptr)
    return;

  // Decloak an opaque pointer
  GDBM_FILE db = *ptr;
  gdbm_close(db);
  *ptr = NULL;
}

/**
 * store_gdbm_version - Implements StoreOps::version() - @ingroup store_version
 */
static const char *store_gdbm_version(void)
{
  return gdbm_version;
}

STORE_BACKEND_OPS(gdbm)
