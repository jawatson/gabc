/* editor-save-changes-dialog.c
 *
 * Copyright 2021 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "editor-save-changes-dialog.h"

#include "config.h"

#include <glib/gi18n.h>

#include <adwaita.h>
#include "gabc-save-changes-dialog-private.h"
#include "gabc-window.h"

static void
gabc_save_changes_dialog_response (AdwAlertDialog *dialog,
                                     const char   *response,
                                     gpointer      user_data);

static void
gabc_save_changes_dialog_discard (AdwAlertDialog *dialog,
                                     gpointer     user_data);

static void
gabc_save_changes_dialog_save_handler (AdwAlertDialog *dialog,
                                     gpointer  user_data);

static void
gabc_save_changes_dialog_save_cb (GtkSourceFileSaver *saver,
                                GAsyncResult         *result,
                                gpointer              user_data);

//TODO break up this definition
static AdwDialog *
_gabc_save_changes_dialog_new ( GtkWindow *parent, gpointer user_data)
{
  AdwDialog *dialog;

  dialog = adw_alert_dialog_new (_("Save Changes?"),
                                 _("Open documents contain unsaved changes. Changes which are not saved will be permanently lost."));

  adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
                                  "cancel", _("_Cancel"),
                                  "discard", _("_Discard"),
                                  "save", _("_Save"),
                                  NULL);
  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
                                            "discard", ADW_RESPONSE_DESTRUCTIVE);
  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
                                            "save", ADW_RESPONSE_SUGGESTED);

  adw_alert_dialog_set_prefer_wide_layout (ADW_ALERT_DIALOG (dialog), TRUE);

  g_signal_connect_data (dialog,
                         "response",
                         G_CALLBACK (gabc_save_changes_dialog_response),
                         user_data,
                         NULL,
                         0);

  return dialog;
}



void
_gabc_save_changes_dialog_run_async (GtkWindow           *parent,
                                       GCancellable        *cancellable,
                                       GAsyncReadyCallback  callback,
                                       gpointer             user_data)
{
  g_autoptr(GTask) task = NULL;
  AdwDialog *dialog;

  // TEST CODE
  GabcWindow *self;
  self = (GabcWindow *)user_data;
  g_assert (GABC_IS_WINDOW (self));

  g_return_if_fail (!parent || GTK_IS_WINDOW (parent));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  dialog = _gabc_save_changes_dialog_new ( parent, user_data );

  // https://docs.gtk.org/gio/class.Task.html
  task = g_task_new (parent, cancellable, callback, user_data);
  g_task_set_source_tag (task, _gabc_save_changes_dialog_run_async);

  g_object_set_data_full (G_OBJECT (dialog),
                          "TASK",
                          g_steal_pointer (&task),
                          g_object_unref);

  adw_dialog_present (dialog, GTK_WIDGET (parent));
}


/* Handle the closing dialog repsonse */
static void
gabc_save_changes_dialog_response (AdwAlertDialog *dialog,
                                     const char   *response,
                                     gpointer     user_data)
{
  g_assert (ADW_IS_ALERT_DIALOG (dialog));

  g_print ("do the response\n");

  // TEST CODE
  GabcWindow *self;
  self = (GabcWindow *)user_data;
  g_assert (GABC_IS_WINDOW (self));
  GtkSourceFile *f = self->abc_source_file;

  if (!g_strcmp0 (response, "discard"))
    {
      g_print ("discard\n");
      gabc_save_changes_dialog_discard (dialog, user_data);
    }
  else if (!g_strcmp0 (response, "save"))
    {
      g_print("do the save\n");
      gabc_save_changes_dialog_save_handler (dialog, user_data);
    }
    else
    {
      g_print ("user cancelled\n");
      GTask *task = g_object_get_data (G_OBJECT (dialog), "TASK");
      g_task_return_new_error (task,
                               G_IO_ERROR,
                               G_IO_ERROR_CANCELLED,
                               "The user cancelled the request");
    }

}

static void
gabc_save_changes_dialog_discard (AdwAlertDialog *dialog,
                                     gpointer     user_data)
{
  GTask *task;

  // Do any cleanup here
  g_assert (dialog != NULL);

  task = g_object_get_data (G_OBJECT (dialog), "TASK");
  g_task_return_boolean (task, TRUE);
}

static void
gabc_save_changes_dialog_save_handler (AdwAlertDialog *dialog,
                                     gpointer  user_data)
{
  GabcWindow *self;
  GTask *task;
  self = (GabcWindow *)user_data;
  g_assert (GABC_IS_WINDOW (self));

  g_print ("gabc_window_save_file_handler\n");

  if (gtk_source_file_get_location(self->abc_source_file) == NULL)
  {
    g_print (" save file dialog");
    //gabc_window_save_file_dialog (NULL, NULL, self);
  }
  else
  {
    g_print ("save to abc file location\n");
    GtkSourceFileSaver *saver = gtk_source_file_saver_new (
                                    self->buffer,
                                    self->abc_source_file);

    task = g_object_get_data (G_OBJECT (dialog), "TASK");
    g_object_set_data_full (G_OBJECT (saver),
                          "TASK",
                          g_steal_pointer (&task),
                          g_object_unref);

    gtk_source_file_saver_save_async (saver,
                                    G_PRIORITY_DEFAULT,
                                    NULL, NULL, NULL, NULL,
                                    (GAsyncReadyCallback) gabc_save_changes_dialog_save_cb,
                                    self);
  }
}


static void
gabc_save_changes_dialog_save_cb (GtkSourceFileSaver *saver,
                                GAsyncResult         *result,
                                gpointer              user_data)
{
  GTask *task;
  g_print ("in the save cb\n");
  // Do any cleanup here
  g_assert (saver != NULL);

  task = g_object_get_data (G_OBJECT (saver), "TASK");
  g_task_return_boolean (task, TRUE);

}


gboolean
_gabc_save_changes_dialog_run_finish (GAsyncResult  *result,
                                        GError       **error)
{
  g_return_val_if_fail (G_IS_TASK (result), FALSE);
  g_print("in _gabc_save_changes_dialog_run_finish\n");
  return g_task_propagate_boolean (G_TASK (result), error);
}
 
