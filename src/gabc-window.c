/* gabc-window.c
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

#include "gabc-window.h"

struct _GabcWindow
{
	AdwApplicationWindow  parent_instance;

	/* Template widgets */
	GtkHeaderBar        *header_bar;
	GtkSourceView       *main_text_view;
        GtkSourceBuffer     *buffer;
        GtkButton           *open_button;
        GtkButton           *engrave_button;
};

G_DEFINE_FINAL_TYPE (GabcWindow, gabc_window, ADW_TYPE_APPLICATION_WINDOW)

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

static void
gabc_window_engrave_file (GAction    *action G_GNUC_UNUSED,
                          GVariant    *parameter G_GNUC_UNUSED,
                          GabcWindow  *self);

static void
gabc_window_write_buffer_to_file (GabcWindow  *self);




static void
gabc_window_class_init (GabcWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        g_type_ensure (GTK_SOURCE_TYPE_VIEW);

	gtk_widget_class_set_template_from_resource (widget_class, "/me/pm/m0dns/gabc/gabc-window.ui");
	gtk_widget_class_bind_template_child (widget_class,
                                              GabcWindow,
                                              header_bar);
        gtk_widget_class_bind_template_child (widget_class,
                                              GabcWindow,
                                              main_text_view);
        gtk_widget_class_bind_template_child (widget_class,
                                              GabcWindow,
                                              open_button);
        gtk_widget_class_bind_template_child (widget_class,
                                              GabcWindow,
                                              engrave_button);
}


static void
gabc_window_init (GabcWindow *self)
{
  GtkSourceLanguageManager *lm;
  GtkSourceLanguage *language = NULL;

  gtk_widget_init_template (GTK_WIDGET (self));
  g_autoptr (GSimpleAction) open_action = g_simple_action_new ("open", NULL);
  g_signal_connect (open_action,
                    "activate",
                    G_CALLBACK (gabc_window__open_file_dialog),
                    self);
  g_action_map_add_action (G_ACTION_MAP (self),
                         G_ACTION (open_action));

  g_autoptr (GSimpleAction) engrave_action = g_simple_action_new ("engrave", NULL);
  g_signal_connect (engrave_action,
                    "activate",
                    G_CALLBACK (gabc_window_engrave_file),
                    self);
  g_action_map_add_action (G_ACTION_MAP (self),
                         G_ACTION (engrave_action));

  // https://www.mail-archive.com/gnome-devtools@gnome.org/msg00448.html
  self->buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer (GTK_TEXT_VIEW(self->main_text_view)));
  g_object_ref (self->buffer);

  gtk_source_buffer_set_highlight_syntax (self->buffer, true);
  gtk_source_buffer_set_highlight_matching_brackets (self->buffer, true);

  lm = gtk_source_language_manager_get_default();
  language = gtk_source_language_manager_get_language (lm,"abc");
  if (language == NULL)
  {
    g_print ("No language found for mime type '%s'\n", "abc");
  }
  else
  {
    gtk_source_buffer_set_language (self->buffer, language);
  }
}


static void
gabc_window__open_file_dialog (GAction    *action G_GNUC_UNUSED,
                              GVariant    *parameter G_GNUC_UNUSED,
                              GabcWindow  *self)
{
  GtkFileDialog *gfd;
  GtkFileFilter *abc_filter;
  GListStore *filter_list;

  gfd = gtk_file_dialog_new ();
  gtk_file_dialog_set_title ( gfd, "Open abc File");

  abc_filter = gtk_file_filter_new();
  gtk_file_filter_add_pattern(abc_filter, "*.abc");

  filter_list = g_list_store_new( G_TYPE_OBJECT );
  g_list_store_append (filter_list, G_OBJECT (abc_filter));

  gtk_file_dialog_set_filters (gfd, G_LIST_MODEL (filter_list));
  gtk_file_dialog_set_default_filter (gfd, abc_filter);

  gtk_file_dialog_open (gfd,
                        GTK_WINDOW (self),
                        NULL,
                        file_open_callback,
                        G_OBJECT (self));
}


static void
file_open_callback ( GObject* file_dialog,
                      GAsyncResult* res,
                      gpointer self)
{
  g_autoptr (GFile) file = gtk_file_dialog_open_finish (GTK_FILE_DIALOG (file_dialog),
                                                        res,
                                                        NULL);
  if (file) {
      open_file (self, file);
  }
  g_object_unref (file_dialog);
}


static void
open_file (GabcWindow       *self,
           GFile            *file)
{
  g_file_load_contents_async (file,
                              NULL,
                              (GAsyncReadyCallback) open_file_complete,
                              self);
}

static void
open_file_complete (GObject          *source_object,
                    GAsyncResult     *result,
                    GabcWindow       *self)
{
  GFile *file = G_FILE (source_object);
  GtkTextIter start;

  g_autofree char *contents = NULL;
  gsize length = 0;

  g_autoptr (GError) error = NULL;

  // Complete the asynchronous operation; this function will either
  // give you the contents of the file as a byte array, or will
  // set the error argument
  g_file_load_contents_finish (file,
                               result,
                               &contents,
                               &length,
                               NULL,
                               &error);

  // In case of error, print a warning to the standard error output
  if (error != NULL)
    {	g_object_ref (self->buffer);
      g_printerr ("Unable to open “%s”: %s\n",
                  g_file_peek_path (file),
                  error->message);
      return;
    }
  // Ensure that the file is encoded with UTF-8
  if (!g_utf8_validate (contents, length, NULL))
  {
    g_printerr ("Unable to load the contents of “%s”: "
                "the file is not encoded with UTF-8\n",
                g_file_peek_path (file));
    return;
  }
  // Set the text using the contents of the file
  gtk_text_buffer_set_text (GTK_TEXT_BUFFER (self->buffer), contents, length);

  // Reposition the cursor so it's at the start of the text
  gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (self->buffer), &start);
  gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (self->buffer), &start);
 }

static void
gabc_window_engrave_file (GAction    *action G_GNUC_UNUSED,
                          GVariant    *parameter G_GNUC_UNUSED,
                          GabcWindow  *self)
{
  g_print("engraving the file\n");
  gabc_window_write_buffer_to_file(self);
}

// https://gnome.pages.gitlab.gnome.org/gtksourceview/gtksourceview5/class.FileSaver.html

// https://github.com/GNOME/gtksourceview/blob/master/testsuite/test-file-saver.c
//
// https://developer.gnome.org/documentation/tutorials/beginners/getting_started/saving_files.html
//
//



static void
gabc_window_write_buffer_to_file (GabcWindow  *self)
{

  GtkTextIter start;
  GtkTextIter end;
  char *text;
  GFileIOStream *gios;
  GOutputStream *gos;
  GError *error = NULL;
  gchar *tmpname = NULL;
  gsize count;

  // For spawn
  gint exit_status = 0;
  gchar *p_stdout = NULL;
  gchar *p_stderr = NULL;
  GError *p_error = NULL;
  gboolean result;

  gtk_widget_set_sensitive (GTK_WIDGET (self->main_text_view), FALSE);
  gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (self->buffer), &start);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (self->buffer), &end);
  text = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (self->buffer), &start, &end, FALSE);
  gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (self->buffer), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (self->main_text_view), TRUE);

  g_file_open_tmp ( "gabc-XXXXXX.abc", &tmpname, &error );

  if ( error ) {
	  g_warning ( "%s", error->message );
	  g_error_free ( error );
	  //return NULL;
  }
  gios = g_file_open_readwrite ( g_file_new_for_path (tmpname), NULL, &error );
  if ( error ) {
	  g_warning ( "%s", error->message );
	  g_error_free ( error );
	  //return NULL;
  }

  count = strlen(text);
  g_print ("length is %lu \n",count);
  g_print("%s",text);
  gos = g_io_stream_get_output_stream ( G_IO_STREAM(gios) );
  if ( g_output_stream_write ( gos, text, count, NULL, &error ) < 0 ) {
	  g_critical ( "Couldn't write tmp %s file due to %s", tmpname, error->message );
	  g_free (tmpname);
	  tmpname = NULL;
  }

  g_output_stream_close ( gos, NULL, &error );
  g_object_unref ( gios );

  g_print("%s\n",tmpname);

  gchar cmd[64];
  //g_snprintf (cmd, 64, "/usr/local/bin/abcm2ps -i -O= %s", tmpname);
  g_snprintf (cmd, 64, "gedit %s", tmpname);

  g_print("%s \n", cmd);

  // use spawn sync as this lets us specify the working directory
  result = g_spawn_command_line_sync("evince", &p_stdout, &p_stderr, &exit_status, &p_error);

  if (result == TRUE ) {
    g_print ("well that went well \n");
  }
  else
  {
    g_print ("abc2ps failed");
  }
  //g_snprintf (cmd, 64, "evince %", tmpname);"

}

