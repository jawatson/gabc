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
#include "gabc-log-window.h"

/*
 *
 * TODO add a dispose function
 */

struct _GabcWindow
{
	AdwApplicationWindow  parent_instance;

        GSettings           *settings;

	GtkHeaderBar        *header_bar;
	GtkSourceView       *main_text_view;
        GtkSourceBuffer     *buffer;
        GtkSourceFile       *abc_source_file;

        GabcLogWindow       *log_window;
};

G_DEFINE_FINAL_TYPE (GabcWindow, gabc_window, ADW_TYPE_APPLICATION_WINDOW)

static void
gabc_window_open_file_dialog (GSimpleAction *action G_GNUC_UNUSED,
                              GVariant      *parameter G_GNUC_UNUSED,
                              gpointer       user_data);

static void
gabc_window_save_file_handler (GSimpleAction *action G_GNUC_UNUSED,
                               GVariant      *parameter G_GNUC_UNUSED,
                               gpointer       user_data);

static void
gabc_window_save_file_dialog (GSimpleAction *action G_GNUC_UNUSED,
                              GVariant      *parameter G_GNUC_UNUSED,
                              gpointer       user_data);

static void
gabc_window_file_save_dialog_cb (GObject       *file_dialog,
                                 GAsyncResult  *res,
                                 gpointer       self);

static void
gabc_window_save_to_abc_file_location(GabcWindow *self);

static void
gabc_window_save_file_async_cb (GtkSourceFileSaver *saver,
                                GAsyncResult       *result,
                                GabcWindow         *self);

static void
file_open_cb (GObject      *source_object,
              GAsyncResult *res,
              gpointer      data);

static void
open_file_cb (GtkSourceFileLoader *loader,
              GAsyncResult        *result,
              GabcWindow          *self);

static void
gabc_window_clear_buffer (GSimpleAction *action G_GNUC_UNUSED,
                          GVariant      *parameter G_GNUC_UNUSED,
                          gpointer       user_data);

static void
open_file (GabcWindow       *self,
           GFile            *file);

static void
gabc_window_engrave_file (GSimpleAction *action G_GNUC_UNUSED,
                          GVariant      *parameter G_GNUC_UNUSED,
                          gpointer       user_data);

static void
gabc_window_play_file  (GSimpleAction *action G_GNUC_UNUSED,
                        GVariant      *parameter G_GNUC_UNUSED,
                        gpointer       user_data);

gchar *
gabc_window_write_buffer_to_file (GabcWindow  *self);

gchar *
gabc_window_write_ps_file (gchar *file_path, GabcWindow *self);

static gchar *
gabc_window_write_midi_file (gchar *file_path, GabcWindow *self);

static void
gabc_window_play_media_file (gchar *file_path, GabcWindow *self);

static void
gabc_window_open_log_dialog (GSimpleAction *action,
                             GVariant      *parameter,
                             gpointer       user_data);

// General Utilities
GtkFileFilter *
get_abc_file_filter (void);

GListStore *
get_abc_filter_list (GtkFileFilter *abc_filter);

gchar *
set_file_extension (gchar *file_path, gchar *extension);

/*
 * END OF DECLARATIONS
 */

static void
gabc_window_class_init (GabcWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        g_type_ensure (GTK_SOURCE_TYPE_VIEW);

	gtk_widget_class_set_template_from_resource (widget_class, "/me/pm/m0dns/gabc/gabc-window.ui");

        gtk_widget_class_bind_template_child (widget_class,
                                              GabcWindow,
                                              main_text_view);

        gtk_widget_class_bind_template_child (widget_class,
                                              GabcWindow,
                                              buffer);
}


static const GActionEntry win_actions[] = {
    { "open-log", gabc_window_open_log_dialog },
    { "play", gabc_window_play_file },
    { "engrave", gabc_window_engrave_file},
    { "save", gabc_window_save_file_handler},
    { "save_as", gabc_window_save_file_dialog},
    { "open", gabc_window_open_file_dialog},
    { "new", gabc_window_clear_buffer}
};


/*
 * The following article discusses the mapping;
 * https://discourse.gnome.org/t/about-g-settings-bind-with-mapping-usage/2399
 */
static gboolean
dark_mode_to_adw_color_scheme (GValue   *value,
                          GVariant *variant,
                          gpointer  user_data)
{
  gboolean dark_mode;
  AdwColorScheme color_scheme;

  dark_mode = g_variant_get_boolean (variant);
  color_scheme = (dark_mode) ? ADW_COLOR_SCHEME_PREFER_DARK : ADW_COLOR_SCHEME_PREFER_LIGHT;
  g_value_set_enum (value, color_scheme);

  return TRUE;
}


static void
gabc_window_init (GabcWindow *self)
{
  GtkSourceLanguageManager *lm;
  GtkSourceLanguage *language;
  const gchar *id;

  AdwStyleManager *sm;

  gtk_widget_init_template (GTK_WIDGET (self));

  self->settings = g_settings_new ("me.pm.m0dns.gabc");

  g_action_map_add_action_entries (G_ACTION_MAP (self),
	                           win_actions,
	                           G_N_ELEMENTS (win_actions),
	                           self);
  sm = adw_style_manager_get_default();

  g_settings_bind_with_mapping (self->settings, "dark-theme",
                              sm, "color-scheme",
                              G_SETTINGS_BIND_GET,
                              dark_mode_to_adw_color_scheme,
                              NULL,
                              NULL,
                              NULL);

  // todo free the sm
  self->abc_source_file = gtk_source_file_new();

  gtk_source_buffer_set_highlight_syntax (self->buffer, true);
  gtk_source_view_set_show_line_numbers (GTK_SOURCE_VIEW(self->main_text_view), true);

  self->log_window = gabc_log_window_new(self);

  lm = gtk_source_language_manager_get_default();
  id = "abc";
  language = gtk_source_language_manager_get_language (lm, id);
  if (language == NULL)
  {
    g_print ("No language found for language id '%s'\n", id);
  }
  else
  {
    gtk_source_buffer_set_language (self->buffer, language);
  }

}


static void
gabc_app_window_dispose (GObject *object)
{
  GabcWindow *win;

  win = GABC_WINDOW (object);

  g_clear_object (&win->settings);

  G_OBJECT_CLASS (gabc_window_parent_class)->dispose (object);
}



static void
gabc_window_clear_buffer (GSimpleAction *action G_GNUC_UNUSED,
                          GVariant      *parameter G_GNUC_UNUSED,
                          gpointer       user_data)
{
  GabcWindow *self = user_data;
  gtk_text_buffer_set_text (GTK_TEXT_BUFFER (self->buffer), "", -1);
  gtk_source_file_set_location (self->abc_source_file, NULL);
}


GtkFileFilter *
get_abc_file_filter (void)
{
  GtkFileFilter *abc_filter = gtk_file_filter_new();
  gtk_file_filter_add_pattern(abc_filter, "*.abc");
  return abc_filter;
}


GListStore *
get_abc_filter_list (GtkFileFilter *abc_filter)
{
  GListStore *filter_list = g_list_store_new( G_TYPE_OBJECT );
  g_list_store_append (filter_list, G_OBJECT (abc_filter));
  return filter_list;
}


static void
gabc_window_open_file_dialog (GSimpleAction *action G_GNUC_UNUSED,
                              GVariant      *parameter G_GNUC_UNUSED,
                              gpointer       user_data)
{
  GtkFileDialog *gfd;
  GtkFileFilter *abc_filter;
  GListStore *filter_list;

  GabcWindow *self = user_data;
  gfd = gtk_file_dialog_new ();
  gtk_file_dialog_set_title ( gfd, "Open abc File");

  abc_filter = get_abc_file_filter();
  filter_list = get_abc_filter_list(abc_filter);

  gtk_file_dialog_set_filters (gfd, G_LIST_MODEL (filter_list));
  gtk_file_dialog_set_default_filter (gfd, abc_filter);

  gtk_file_dialog_open (gfd,
                        GTK_WINDOW (self),
                        NULL,
                        file_open_cb,
                        G_OBJECT (self));

  g_object_unref (abc_filter);
  g_object_unref (filter_list);
}


static void
file_open_cb (GObject       *file_dialog,
              GAsyncResult  *res,
              gpointer       self)
{
  g_autoptr (GFile) file = gtk_file_dialog_open_finish (GTK_FILE_DIALOG (file_dialog),
                                                        res,
                                                        NULL);
  if (file) {
    open_file (self, file);
    //g_object_unref (file);
  }
  g_object_unref (file_dialog);
}


static void
open_file_cb (GtkSourceFileLoader *loader,
              GAsyncResult        *result,
              GabcWindow          *self)
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
    // Reposition the cursor so it's at the start of the text
    gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (self->buffer), &start);
    gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (self->buffer), &start);
    gtk_widget_grab_focus (GTK_WIDGET (self->main_text_view));
  }

  g_object_unref (loader);
}


static void
open_file (GabcWindow       *self,
           GFile            *file)
{
  GtkSourceFileLoader *loader;
  gtk_source_file_set_location(GTK_SOURCE_FILE (self->abc_source_file), file);
  loader = gtk_source_file_loader_new (GTK_SOURCE_BUFFER (self->buffer),
                                       GTK_SOURCE_FILE (self->abc_source_file));

  gtk_source_file_loader_load_async (loader,
                                     G_PRIORITY_DEFAULT,
                                     NULL, NULL, NULL, NULL,
                                     (GAsyncReadyCallback) open_file_cb,
                                     self);
}



static void
gabc_window_save_file_handler (GSimpleAction *action G_GNUC_UNUSED,
                               GVariant      *parameter G_GNUC_UNUSED,
                               gpointer       user_data)
{
  GabcWindow *self = user_data;
  if (gtk_source_file_get_location(self->abc_source_file) == NULL)
  {
    gabc_window_save_file_dialog (NULL, NULL, self);
  }
  else
  {
    gabc_window_save_to_abc_file_location (self);
  }
}

static void
gabc_window_save_file_dialog (GSimpleAction *action G_GNUC_UNUSED,
                              GVariant      *parameter G_GNUC_UNUSED,
                              gpointer       user_data)
{
  GtkFileDialog *gfd;
  GtkFileFilter *abc_filter;
  GListStore *filter_list;

  GabcWindow *self = user_data;

  gfd = gtk_file_dialog_new ();
  gtk_file_dialog_set_title (gfd, "Save abc File");

  abc_filter = get_abc_file_filter();
  filter_list = get_abc_filter_list(abc_filter);

  gtk_file_dialog_set_filters (gfd, G_LIST_MODEL (filter_list));
  gtk_file_dialog_set_default_filter (gfd, abc_filter);

  gtk_file_dialog_save (gfd,
                        GTK_WINDOW (self),
                        NULL,
                        gabc_window_file_save_dialog_cb,
                        G_OBJECT (self));
  g_object_unref (abc_filter);
  g_object_unref (filter_list);
}


static void
gabc_window_file_save_dialog_cb (GObject       *file_dialog,
                                 GAsyncResult  *res,
                                 gpointer       user_data)
{
  GabcWindow *self = user_data;
  g_autoptr (GFile) file = gtk_file_dialog_save_finish (GTK_FILE_DIALOG (file_dialog),
                                                        res,
                                                        NULL);
  if (file) {
    gtk_source_file_set_location(self->abc_source_file, file);
    gabc_window_save_to_abc_file_location (self);
  }
  g_object_unref (file_dialog);

}



static void
gabc_window_save_to_abc_file_location(GabcWindow *self)
{
  GtkSourceFileSaver *saver = gtk_source_file_saver_new (
                                    self->buffer,
                                    self->abc_source_file);
  gtk_source_file_saver_save_async (saver,
                                    G_PRIORITY_DEFAULT,
                                    NULL, NULL, NULL, NULL,
                                    (GAsyncReadyCallback) gabc_window_save_file_async_cb,
                                    self);
}

static void
gabc_window_save_file_async_cb (GtkSourceFileSaver *saver,
                                GAsyncResult       *result,
                                GabcWindow         *self)
{
  GError *error = NULL;

  if (!gtk_source_file_saver_save_finish (saver, result, &error))
  {
    g_printerr ("Error saving file: %s\n", error->message);
    g_clear_error (&error);
  }
  g_object_unref (saver);
}

static void
gabc_window_engrave_file (GSimpleAction *action G_GNUC_UNUSED,
                          GVariant      *parameter G_GNUC_UNUSED,
                          gpointer       user_data)
{
  gchar *abc_file_path, *ps_file_path;
  GabcWindow *self = user_data;
  abc_file_path = gabc_window_write_buffer_to_file (self);
  ps_file_path = gabc_window_write_ps_file (abc_file_path, self);
  gabc_window_play_media_file (ps_file_path, self);

  g_free (abc_file_path);
  g_free (ps_file_path);
}

static void
gabc_window_play_file  (GSimpleAction *action G_GNUC_UNUSED,
                        GVariant      *parameter G_GNUC_UNUSED,
                        gpointer       user_data)
{
  gchar *abc_file_path, *midi_file_path;
  GabcWindow *self = user_data;
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
  GFile *parent_file;
  gchar *parent_dir;
  gchar *original_basename;
  gchar *new_file_path;
  gchar *new_basename;
  gchar **tokens;
  GFile *file;

  file = g_file_new_for_path(file_path);
  original_basename = g_file_get_basename(file);
  parent_file = g_file_get_parent(file);
  parent_dir = g_file_get_path(parent_file);

  tokens = g_strsplit(original_basename, ".", 0);

  new_basename = g_strconcat(tokens[0], ".", extension, NULL);
  new_file_path = g_build_filename (parent_dir, new_basename, NULL);

  g_object_unref (parent_file);
  g_free (parent_dir);
  g_free (original_basename);
  g_free (new_basename);
  g_strfreev(tokens);
  g_object_unref (file);

  return new_file_path;
}


gchar *
gabc_window_write_ps_file (gchar *file_path, GabcWindow *self)
{
  gchar *standard_output;
  gchar *standard_error;
  gint exit_status;
  GError *error = NULL;
  gboolean result;

  gchar *ps_file_path;
  gchar *working_dir_path;

  gchar *fmt_file_path;
  gchar *page_numbering_mode;

  GFile *working_file;
  GFile *working_dir_file;

  gint idx;
  gchar *cmd[20];

  if (gtk_source_file_get_location(self->abc_source_file) == NULL)
  {
    // copy the str so we can free it later.
    working_dir_path = g_strdup (g_getenv ("XDG_CACHE_HOME"));
  }
  else
  {
    // gtk_source_file_get_location data is owned by the instance.
    working_file = gtk_source_file_get_location (self->abc_source_file);
    working_dir_file = g_file_get_parent (working_file);
    working_dir_path = g_file_get_path (working_dir_file);
    g_object_unref (working_dir_file); // also working dir path...
  }

  ps_file_path = set_file_extension (file_path, (gchar *)("ps"));

  idx = 0;
  cmd[idx++] = (gchar *)("abcm2ps");

  if (g_settings_get_boolean (self->settings, "abcm2ps-show-errors")) {
      cmd[idx++] = (gchar *)("-i");
  }

  fmt_file_path = g_settings_get_string (self->settings, "abcm2ps-fmt-file-path");

  if (fmt_file_path != 0)
  {
    cmd[idx++] = "-F";
    cmd[idx++] = fmt_file_path;
  }

  page_numbering_mode = g_settings_get_string (self->settings, "abcm2ps-page-numbering");
  cmd[idx++] = (gchar *)("-N");
  cmd[idx++] = (page_numbering_mode);

  cmd[idx++] = (gchar *)("-O");
  cmd[idx++] = ps_file_path;
  cmd[idx++] = file_path;
  cmd[idx++] = NULL;

  result = g_spawn_sync (working_dir_path, (gchar **)cmd, NULL,
                      G_SPAWN_SEARCH_PATH, NULL, NULL,
                      &standard_output, &standard_error,
                      &exit_status, &error);

  if (result != TRUE )
  {
    gabc_log_window_append_to_log (self->log_window, error->message);
    g_clear_error (&error);
  }

  gabc_log_window_append_to_log (self->log_window, standard_output);
  gabc_log_window_append_to_log (self->log_window, standard_error);

  //g_array_free
  g_free (standard_output);
  g_free (standard_error);
  g_free (working_dir_path);
  g_free (fmt_file_path);
  g_free (page_numbering_mode);

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

  GFile *path_file;
  GFile *midi_file;

  gint idx;
  gchar *cmd[10];

  path_file = g_file_new_for_path(file_path);
  abc_basename = g_file_get_basename (path_file);
  midi_file_path = set_file_extension (file_path, (gchar*)("mid"));
  midi_file = g_file_new_for_path(midi_file_path);
  midi_basename = g_file_get_basename (midi_file);

  idx = 0;
  cmd[idx++] = (gchar *)("abc2midi");
  cmd[idx++] = abc_basename;
  cmd[idx++] = (gchar *)("-o");
  cmd[idx++] = midi_basename;
  cmd[idx++] = NULL;

  result = g_spawn_sync (g_getenv("XDG_CACHE_HOME"), (gchar **)cmd, NULL,
                      G_SPAWN_SEARCH_PATH, NULL, NULL,
                      &standard_output, &standard_error,
                      &exit_status, &error);

  if (result != TRUE ) {
    gabc_log_window_append_to_log (self->log_window, error->message);
    g_clear_error (&error);
  }

  gabc_log_window_append_to_log (self->log_window, standard_output);
  gabc_log_window_append_to_log (self->log_window, standard_error);

  g_object_unref (path_file);
  g_object_unref (midi_file);
  g_free (standard_output);
  g_free (standard_error);
  g_free (abc_basename);
  g_free (midi_basename);

  return midi_file_path;
}


static void
play_media_cb (GtkFileLauncher *launcher,
               GAsyncResult    *result,
               gpointer         data)
{
  GError *error = NULL;
  GabcWindow *self = GABC_WINDOW(data);

  if (!gtk_file_launcher_launch_finish (launcher, result, &error))
  {
    gabc_log_window_append_to_log (self->log_window, error->message);
    g_clear_error (&error);
  }
  g_object_unref (launcher);
}


static void
gabc_window_play_media_file (gchar *file_path, GabcWindow *self)
{
  GFile *media_file = g_file_new_for_path ((char *)file_path);
  GtkFileLauncher *launcher = gtk_file_launcher_new (media_file);
  gtk_file_launcher_launch (launcher,
                            GTK_WINDOW (self),
                            NULL,
                            (GAsyncReadyCallback) play_media_cb,
                            self);
  g_object_unref (media_file);
}


/*
 * LOG DIALOG
 */
static void
gabc_window_open_log_dialog (GSimpleAction *action,
                             GVariant      *parameter,
                             gpointer       user_data)
{
  GabcWindow *parent = user_data;
  g_assert (GABC_IS_WINDOW (parent));
  gtk_widget_set_visible (GTK_WIDGET (parent->log_window), true);
}

