/* gabc-tunebook.c
 *
 * Copyright 2025 James Watson
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

#include "gabc-tunebook.h"

struct _GabcTunebook
{
  GtkSourceBuffer               parent_instance;

  /*
  EditorBufferMonitor          *monitor;
  GtkSourceFile                *file;
  gchar                        *draft_id;
  const GtkSourceEncoding      *encoding;
  GError                       *last_error;
  */
};


gboolean
gabc_document_get_busy (GabcTunebook  *self)
{
  return true;
}
