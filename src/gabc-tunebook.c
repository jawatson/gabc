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

// Define the structure here
//
G_DEFINE_TYPE (GabcTunebook, gabc_tunebook, GTK_SOURCE_TYPE_BUFFER)


static void
gabc_tunebook_class_init (GabcTunebookClass *klass)
{
  g_print ("In the class init\n");
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkTextBufferClass *buffer_class = GTK_TEXT_BUFFER_CLASS (klass);

}


static void
gabc_tunebook_init (GabcTunebook *self)
{
  g_print ("in the init\n");
}


void
gabc_tunebook_open_file (GabcTunebook      *self,
                         GFile           *file)
{

  GtkSourceFileLoader *loader;
  gtk_source_file_set_location(GTK_SOURCE_FILE (self->abc_source_file), file);
  loader = gtk_source_file_loader_new (GTK_SOURCE_BUFFER (self),
                                       GTK_SOURCE_FILE (self->abc_source_file));

  gtk_source_file_loader_load_async (loader,
                                     G_PRIORITY_DEFAULT,
                                     NULL, NULL, NULL, NULL,
                                     (GAsyncReadyCallback) gabc_tunebook_open_file_cb,
                                     self);
}

void
gabc_tunebook_open_file_cb (GtkSourceFileLoader *loader,
                          GAsyncResult        *result,
                          GabcTunebook          *self)
{
  GtkTextIter start;
  GError *error = NULL;

  if (!gtk_source_file_loader_load_finish (loader, result, &error))
  {
    g_printerr ("Error loading file: %s\n", error->message);
    g_clear_error (&error);
  }
  else
  {
    gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (self), &start);
    gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (self), &start);
    self->tunebook_is_modified = FALSE;
  }
  g_object_unref (loader);
}

void
gabc_tunebook_save_file (GabcTunebook *self)
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
gabc_tunebook_is_empty (GabcTunebook *self)
{
  GtkTextIter start;
  GtkTextIter end;
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (self), &start, &end);
  return gtk_text_iter_equal (&start, &end);
}

gboolean
gabc_tunebook_is_modified (GabcTunebook *self)
{
 return self->tunebook_is_modified;
}
