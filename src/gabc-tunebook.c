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




gboolean
gabc_document_get_busy (GabcTunebook  *self)
{
  return true;
}

gboolean
gabc_tunebook_is_empty (GabcTunebook *self) {
  GtkTextIter start;
  GtkTextIter end;
  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (self), &start, &end);
  return gtk_text_iter_equal (&start, &end);
}

