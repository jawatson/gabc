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
        GtkButton           *play_button;
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
gabc_window_play_file  (GAction    *action G_GNUC_UNUSED,
                          GVariant    *parameter G_GNUC_UNUSED,
                          GabcWindow  *self);

gchar *
gabc_window_write_buffer_to_file (GabcWindow  *self);

gchar *
gabc_window_write_ps_file (gchar *file_path, GabcWindow *self);

static gchar *
gabc_window_write_midi_file (gchar *file_path, GabcWindow *self);

static void
gabc_window_play_media_file (gchar *file_path, GabcWindow *self);

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
        gtk_widget_class_bind_template_child (widget_class,
                                              GabcWindow,
                                              play_button);
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

  g_autoptr (GSimpleAction) play_action = g_simple_action_new ("play", NULL);
  g_signal_connect (play_action,
                    "activate",
                    G_CALLBACK (gabc_window_play_file),
                    self);
  g_action_map_add_action (G_ACTION_MAP (self),
                         G_ACTION (play_action));

  self->buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer (GTK_TEXT_VIEW(self->main_text_view)));
  g_object_ref (self->buffer);

  gtk_source_buffer_set_highlight_syntax (self->buffer, true);
  //gtk_source_buffer_set_highlight_matching_brackets (self->buffer, true);
  gtk_source_view_set_show_line_numbers (GTK_SOURCE_VIEW(self->main_text_view), true);

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

  g_object_unref (abc_filter);
  g_object_unref (filter_list);
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
  gchar *abc_file_path, *ps_file_path;
  abc_file_path = gabc_window_write_buffer_to_file (self);
  ps_file_path = gabc_window_write_ps_file (abc_file_path, self);
  gabc_window_play_media_file (ps_file_path, self);
  g_free (abc_file_path);
  g_free (ps_file_path);
}

static void
gabc_window_play_file  (GAction    *action G_GNUC_UNUSED,
                          GVariant    *parameter G_GNUC_UNUSED,
                          GabcWindow  *self)
{
  gchar *abc_file_path, *midi_file_path;
  abc_file_path = gabc_window_write_buffer_to_file (self);
  midi_file_path = gabc_window_write_midi_file (abc_file_path, self);
  gabc_window_play_media_file (midi_file_path, self);
  g_free (abc_file_path);
  g_free (midi_file_path);
}


gchar *
gabc_window_write_buffer_to_file (GabcWindow *self)
{

  GtkTextIter start;
  GtkTextIter end;
  char *text;
  gchar *file_path;

  gtk_widget_set_sensitive (GTK_WIDGET (self->main_text_view), FALSE);
  gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (self->buffer), &start);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (self->buffer), &end);
  text = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (self->buffer), &start, &end, FALSE);
  gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (self->buffer), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (self->main_text_view), TRUE);

  //g_print("%s",g_uuid_string_random());
  file_path = g_build_filename (g_getenv("XDG_CACHE_HOME"), "gabc.abc", NULL);
  g_file_set_contents(file_path, text, -1, NULL);

  g_free (text);
  return file_path;
}

gchar *
set_file_extension (gchar *file_path, gchar *extension)
{
  gchar *parent_dir;
  gchar *original_basename;
  gchar *new_file_path;
  gchar *new_basename;
  gchar **tokens;
  GFile *file;

  file = g_file_new_for_path(file_path);
  original_basename = g_file_get_basename(file);
  parent_dir = g_file_get_path(g_file_get_parent(file));

  tokens = g_strsplit(original_basename, ".", 0);

  new_basename = g_strconcat(tokens[0], ".", extension, NULL);
  new_file_path = g_build_filename (parent_dir, new_basename, NULL);

  g_free (parent_dir);
  g_free (original_basename);
  g_free (new_basename);
  g_strfreev(tokens);
  g_free (file);

  return new_file_path;
}

gchar *
gabc_window_write_ps_file (gchar *file_path, GabcWindow *self)
{
  // For spawn
  gchar *standard_output;
  gchar *standard_error;

  GError *error = NULL;
  gint exit_status;
  gboolean result;

  gchar *ps_file_path;
  gchar *abc_basename;

  ps_file_path = set_file_extension (file_path, "ps");
  abc_basename = g_file_get_basename (g_file_new_for_path(file_path));

  gchar *cmd[] = { "abcm2ps", "-O=", abc_basename, NULL };

  result = g_spawn_sync (g_getenv("XDG_CACHE_HOME"), (gchar **)cmd, NULL,
                      G_SPAWN_SEARCH_PATH, NULL, NULL,
                      &standard_output, &standard_error,
                      &exit_status, &error);

  if (result != TRUE )
  {
    g_print ("abcm2ps failed: %s \n", error->message);
    g_clear_error (&error);
  }

  g_print ("%s\n",standard_output);

  g_free (standard_output);
  g_free (standard_error);
  g_free (abc_basename);

  return ps_file_path;
}


static gchar *
gabc_window_write_midi_file (gchar *file_path, GabcWindow *self)
{
  gchar *standard_output;
  gchar *standard_error;
  GError *error = NULL;
  gint exit_status;
  gboolean result;

  gchar *abc_basename;
  gchar *midi_basename;
  gchar *midi_file_path;

  abc_basename = g_file_get_basename (g_file_new_for_path(file_path));
  midi_file_path = set_file_extension (file_path, "mid");
  midi_basename = g_file_get_basename (g_file_new_for_path(midi_file_path));

  const gchar *cmd[5] = { "abc2midi", abc_basename, "-o", midi_basename, NULL };

  result = g_spawn_sync (g_getenv("XDG_CACHE_HOME"), (gchar **)cmd, NULL,
                      G_SPAWN_SEARCH_PATH, NULL, NULL,
                      &standard_output, &standard_error,
                      &exit_status, &error);

  if (result != TRUE ) {
    g_print ("abc2midi failed: %s \n", error->message);
    g_clear_error (&error);
  }

  g_print ("%s\n",standard_output);

  g_free (standard_output);
  g_free (standard_error);
  g_free (abc_basename);
  g_free (midi_basename);

  return midi_file_path;
}


static void
gabc_window_play_media_file (gchar *file_path, GabcWindow *self)
{
  GFile *media_file = g_file_new_for_path ((char *)file_path);
  GtkFileLauncher *launcher = gtk_file_launcher_new (media_file);
  gtk_file_launcher_launch (launcher, GTK_WINDOW (self), NULL, NULL, NULL);
}



