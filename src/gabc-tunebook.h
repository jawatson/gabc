/* gabc-tunebook.h
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
#pragma once

#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

#define GABC_TYPE_TUNEBOOK (gabc_tunebook_get_type())

G_DECLARE_FINAL_TYPE (GabcTunebook, gabc_tunebook, GABC, TUNEBOOK, GtkSourceBuffer)

struct _GabcTunebook
{
  GtkSourceBuffer               parent_instance;
  GtkSourceFile                *abc_source_file;
  gboolean                      tunebook_is_modified;
  /*

   *   EditorBufferMonitor          *monitor;
  GtkSourceFile                *file;
  gchar                        *draft_id;
  const GtkSourceEncoding      *encoding;
  GError                       *last_error;
  */
};


//GFile                    *gabc_tunebook_get_file                (GabcTunebook  *self);

gboolean                  gabc_tunebook_is_empty                  (GabcTunebook *self);
void                      gabc_tunebook_save_file                 (GabcTunebook *self);

void                      gabc_tunebook_save_file_async_cb        (GtkSourceFileSaver *saver,
                                                                   GAsyncResult       *result,
                                                                   GabcTunebook       *self);


G_END_DECLS
