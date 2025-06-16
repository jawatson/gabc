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

#include "gabc-window.h"
#include "gabc-tunebook.h"


void
gabc_tunebook_save_to_abc_file_location(GabcTunebook *self)
{
  GtkSourceFileSaver *saver = gtk_source_file_saver_new (
                                    (GtkSourceBuffer *) self,
                                    self->abc_source_file);
  gtk_source_file_saver_save_async (saver,
                                    G_PRIORITY_DEFAULT,
                                    NULL, NULL, NULL, NULL,
                                    (GAsyncReadyCallback) gabc_tunebook_save_file_async_cb,
                                    self);
}

void
gabc_tunebook_save_file_async_cb (GtkSourceFileSaver *saver,
                                GAsyncResult       *result,
                                GabcTunebook         *self)
{
  GError *error = NULL;

  if (!gtk_source_file_saver_save_finish (saver, result, &error))
  {
    g_printerr ("Error saving file: %s\n", error->message);
    g_clear_error (&error);
  }
  else
  {
    // ("gabc_window_save_file_async_cb: Setting self->buffer_is_modified = FALSE\n");
    self->tunebook_is_modified = FALSE;
  }
  g_object_unref (saver);
}


gboolean
gabc_tunebook_is_empty (GabcTunebook *self) {
  GtkTextIter start;
  GtkTextIter end;
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (self), &start, &end);
  return gtk_text_iter_equal (&start, &end);
}

