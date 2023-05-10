/* gabc-window.h
 *
 * Copyright 2023 James Watson
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

#include <adwaita.h>
#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

#define GABC_TYPE_WINDOW (gabc_window_get_type())

G_DECLARE_FINAL_TYPE (GabcWindow, gabc_window, GABC, WINDOW, AdwApplicationWindow)

static void
gabc_window__open_file_dialog (GAction    *action G_GNUC_UNUSED,
                              GVariant    *parameter G_GNUC_UNUSED,
                              GabcWindow  *self);

static void
file_open_callback ( GObject* source_object,
                      GAsyncResult* res,
                      gpointer data);

static void
open_file_complete (GObject          *source_object,
                    GAsyncResult     *result,
                    GabcWindow       *self);

static void
open_file (GabcWindow       *self,
           GFile            *file);

G_END_DECLS
