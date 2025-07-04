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
#include "gabc-application.h"
#include "gabc-log-window.h"
#include "gabc-tunebook.h"


G_BEGIN_DECLS

#define GABC_TYPE_WINDOW (gabc_window_get_type())

G_DECLARE_FINAL_TYPE (GabcWindow, gabc_window, GABC, WINDOW, AdwApplicationWindow)

GabcWindow *
gabc_window_new (GabcApplication *app);

GabcTunebook *
gabc_window_get_current_tunebook (GabcWindow *self);

/*void
gabc_window_open_file (GabcWindow       *self,
                       GFile            *file);
*/

void
gabc_window_append_file_content_to_buffer (GabcWindow       *self,
                                           GFile            *file);



G_END_DECLS

