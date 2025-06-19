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
struct _GabcTunebook
{
  GtkSourceBuffer               parent_instance;
  GtkSourceFile                *abc_source_file;
  gboolean                      is_modified;
};



G_DEFINE_TYPE (GabcTunebook, gabc_tunebook, GTK_SOURCE_TYPE_BUFFER)

static gchar * gabc_tunebook_set_midi_program (gchar * file_content, gint midi_program);

GabcTunebook*
gabc_tunebook_new (void)
{
  GabcTunebook *self;

  self = g_object_new (GABC_TYPE_TUNEBOOK, NULL);

  return g_steal_pointer (&self);

}

static void
gabc_tunebook_class_init (GabcTunebookClass *klass)
{
  g_print ("In the class init\n");
  //GObjectClass *object_class = G_OBJECT_CLASS (klass);
  //GtkTextBufferClass *buffer_class = GTK_TEXT_BUFFER_CLASS (klass);
}


static void
gabc_tunebook_init (GabcTunebook *self)
{
  GtkSourceLanguageManager *lm;
  GtkSourceLanguage *language;
  gchar *id;

  g_print ("in the init\n");

  self->abc_source_file = gtk_source_file_new ();

  lm = gtk_source_language_manager_get_default ();
  id = "abc";
  language = gtk_source_language_manager_get_language (lm, id);
  if (language == NULL)
  {
    g_print ("No language found for language id '%s'\n", id);
  }
  else
  {
    //TODO Why does the following line cause so many warnings?
    // TODO move this to the tunebook class.
    gtk_source_buffer_set_language ((GtkSourceBuffer *) self, language);
  }
}


void
gabc_tunebook_open_file (GabcTunebook      *self,
                         GFile           *file)
{
  g_print ("gabc_tunebook_open_file\n");
  g_print ("File: %s\n",g_file_get_path(file));
  GtkSourceFileLoader *loader;

  gtk_source_file_set_location(GTK_SOURCE_FILE (self->abc_source_file), file);
  g_print ("File: %s",g_file_get_path( gtk_source_file_get_location (self->abc_source_file)));
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
    g_print ("Error\n");
    g_printerr ("Error loading file: %s\n", error->message);
    g_clear_error (&error);
  }
  else
  {
    g_print ("gabc_tunebook_open_file_cb: No error");
    gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (self), &start);
    gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (self), &start);
    self->is_modified = FALSE;
  }
  g_object_unref (loader);
}

void
gabc_tunebook_append_file (GabcTunebook       *self,
                           GFile            *file)
{
  g_file_load_contents_async (file,
                              NULL,
                              (GAsyncReadyCallback) gabc_tunebook_append_file_cb,
                              self);
}

void
gabc_tunebook_append_file_cb  (GObject       *source_object,
                                GAsyncResult  *result,
                                GabcTunebook    *self)
{
  GFile *file = G_FILE (source_object);
  GtkTextIter end;

  g_autofree char *contents = NULL;
  gsize length = 0;

  g_autoptr (GError) error = NULL;

  g_file_load_contents_finish (file,
                               result,
                               &contents,
                               &length,
                               NULL,
                               &error);
  /*
  if (error != NULL)
    {
      gabc_log_window_append_to_log (self->log_window, error->message);
      g_clear_error (&error);
      return;
    }

  if (!g_utf8_validate (contents, length, NULL))
    {
      gchar *err_msg = g_strdup_printf ("Unable to load the contents of %s.  File is not encoded with UTF-8\n", g_file_peek_path (file));
      gabc_log_window_append_to_log (self->log_window, err_msg);
      g_free (err_msg);
      return;
    }*/

  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (self), &end);

  gtk_text_buffer_insert(GTK_TEXT_BUFFER (self), &end, "\n\n", -1);

  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (self), &end);

  gtk_text_buffer_insert(GTK_TEXT_BUFFER (self),
                         &end,
                         contents,
                         -1);
  g_object_unref (file);
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
    self->is_modified = FALSE;
  }
  g_object_unref (saver);
}

gchar*
gabc_tunebook_write_to_scratch_file (GabcTunebook  *self, GSettings *settings)
{
  GtkTextIter start;
  GtkTextIter end;
  char *text;
  gchar *file_path;
  gint midi_program; //TODO

  gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (self), &start);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (self), &end);
  text = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (self), &start, &end, FALSE);

  self->is_modified = self->is_modified || gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (self));
  //g_print("gabc_window_write_buffer_to_file set buffer_is_modified to %s\n", self->buffer_is_modified ? "TRUE" : "FALSE");

  gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (self), FALSE);

  // Preprocess the file here
  midi_program = g_settings_get_enum (settings, "abc2midi-midi-program");
  if (midi_program < 128)
    {
    text = gabc_tunebook_set_midi_program (text, midi_program);
    }

  file_path = g_build_filename (g_getenv("XDG_CACHE_HOME"), "gabc_scratch.abc", NULL);
  g_file_set_contents(file_path, text, -1, NULL);

  g_free (text);
  return file_path;
}


static gchar *
gabc_tunebook_set_midi_program (gchar * file_content, gint midi_program)
{
  gchar *preprocessed_text;
  GError *error = NULL;
  GRegex *end_of_header_regex;
  gchar *program_str = g_strdup_printf ("\\0\\n%%%%MIDI program %d", midi_program);

  end_of_header_regex = g_regex_new ("^K:.*$", G_REGEX_MULTILINE, 0, &error);

  if ( error )
  {
    //gabc_log_window_append_to_log (self->log_window, error->message);
    g_clear_error (&error);
  }

  preprocessed_text = g_regex_replace (end_of_header_regex,
                          file_content,
                          -1,
                          0,
                          program_str,
                          G_REGEX_MATCH_DEFAULT,
                          NULL);
  g_free (program_str);
  g_regex_unref (end_of_header_regex);
  return preprocessed_text;
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
  gboolean buffer_is_modified;
  buffer_is_modified = gtk_text_buffer_get_modified ( (GtkTextBuffer *) self);
  return self->is_modified || buffer_is_modified;
}


void
gabc_tunebook_clear (GabcTunebook *self)
{
  gtk_text_buffer_set_text (GTK_TEXT_BUFFER (self), "", -1);
  gtk_source_file_set_location (self->abc_source_file, NULL);
  self->is_modified = FALSE;
}


GtkSourceFile*
gabc_tunebook_get_abc_source_file (GabcTunebook *self)
{
  //return self->abc_source_file;
  /*
   GFile*
   gtk_source_file_get_location (
      GtkSourceFile* file
  )
  */

  g_print ("gabc_tunebook_get_abc_source_file:\n");
  g_print ("%s\n", g_file_get_path (gtk_source_file_get_location (self->abc_source_file)));
  return self->abc_source_file;
}


void
gabc_tunebook_set_abc_source_file (GabcTunebook *self, GFile *file)
{
  g_print("setting the source file\n");
  gtk_source_file_set_location (self->abc_source_file, file);
}

