/* main.c
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

#include "config.h"

#include <glib/gi18n.h>

#include "gabc-application.h"
#include "gabc-window.h"


static void
open (GApplication  *application,
      GFile        **files,
      gint           n_files,
      const gchar   *hint)
{
  GabcWindow *window;

  window = GABC_WINDOW (gabc_window_new (GABC_APPLICATION (application)));

  //TODO
  // gabc_tunebook_open_file(window->tunebook, files[0]);

  gtk_window_present (GTK_WINDOW (window));
}


int
main (int   argc,
      char *argv[])
{
	g_autoptr(GabcApplication) app = NULL;
	int ret;

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	//app = gabc_application_new ("me.pm.m0dns.gabc", G_APPLICATION_DEFAULT_FLAGS);
  	app = gabc_application_new ("me.pm.m0dns.gabc", G_APPLICATION_HANDLES_OPEN);
        g_signal_connect (app, "open", G_CALLBACK (open), NULL);
	ret = g_application_run (G_APPLICATION (app), argc, argv);

	return ret;
}
