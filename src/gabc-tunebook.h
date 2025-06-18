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
  gboolean                      is_modified;
};


void                      gabc_tunebook_open_file                 (GabcTunebook  *self,
                                                                   GFile         *file);

void                      gabc_tunebook_open_file_cb              (GtkSourceFileLoader *loader,
                                                                   GAsyncResult        *result,
                                                                   GabcTunebook        *self);

void                      gabc_tunebook_append_file               (GabcTunebook        *self,
                                                                   GFile               *file);

void                      gabc_tunebook_append_file_cb            (GObject       *source_object,
                                                                   GAsyncResult  *result,
                                                                   GabcTunebook    *self);

gboolean                  gabc_tunebook_is_empty                  (GabcTunebook *self);

gboolean                  gabc_tunebook_is_modified               (GabcTunebook *self);

void                      gabc_tunebook_save_file                 (GabcTunebook *self);

void                      gabc_tunebook_save_file_async_cb        (GtkSourceFileSaver *saver,
                                                                   GAsyncResult       *result,
                                                                   GabcTunebook       *self);

gchar *                   gabc_tunebook_write_to_scratch_file     (GabcTunebook  *self,
                                                                   GSettings *settings);

GtkSourceFile *           gabc_tunebook_get_abc_source_file       (GabcTunebook *self);

void                      gabc_tunebook_set_abc_source_file       (GabcTunebook *self, GFile *f);

void                      gabc_tunebook_clear                     (GabcTunebook *self);


G_END_DECLS
