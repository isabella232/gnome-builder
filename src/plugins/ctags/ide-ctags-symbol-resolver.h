/* ide-ctags-symbol-resolver.h
 *
 * Copyright 2015-2019 Christian Hergert <christian@hergert.me>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <libide-core.h>
#include <libide-code.h>

G_BEGIN_DECLS

#define IDE_TYPE_CTAGS_SYMBOL_RESOLVER (ide_ctags_symbol_resolver_get_type())

G_DECLARE_FINAL_TYPE (IdeCtagsSymbolResolver, ide_ctags_symbol_resolver, IDE, CTAGS_SYMBOL_RESOLVER, IdeObject)

void               ide_ctags_symbol_resolver_get_location_async  (IdeCtagsSymbolResolver   *self,
                                                                  IdeCtagsIndex            *index,
                                                                  const IdeCtagsIndexEntry *entry,
                                                                  GCancellable             *cancellable,
                                                                  GAsyncReadyCallback       callback,
                                                                  gpointer                  user_data);
IdeLocation *ide_ctags_symbol_resolver_get_location_finish (IdeCtagsSymbolResolver   *self,
                                                                  GAsyncResult             *result,
                                                                  GError                  **error);

G_END_DECLS
