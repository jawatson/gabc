/* gabc-save-changes-dialog.c
 *
 */

#include "config.h"

#include <adwaita.h>
#include "gabc-save-changes-dialog-private.h"
#include "gabc-file-filters.h"
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
gabc_save_changes_dialog_show_file_save_dialog (GabcWindow *parent,
                                         GTask *task);

static void
gabc_save_changes_dialog_show_file_save_dialog_cb (GObject *file_dialog,
                                 GAsyncResult  *res,
                                 gpointer       user_data);

static void
gabc_save_changes_dialog_save (GabcWindow *self, GTask *task);

static void
gabc_save_changes_dialog_save_cb (GtkSourceFileSaver *saver,
                                GAsyncResult         *result,
                                gpointer              user_data);

//TODO break up this definition
static AdwDialog *
_gabc_save_changes_dialog_new ( GtkWindow *parent, gpointer user_data)
{
  AdwDialog *dialog;

  dialog = adw_alert_dialog_new ("Save Changes?",
                                 "Open documents contain unsaved changes. Changes which are not saved will be permanently lost.");

  adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
                                  "cancel", "Cancel",
                                  "discard", "Discard",
                                  "save", "Save",
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

  if (!g_strcmp0 (response, "discard"))
    {
      g_print ("discard\n");
      gabc_save_changes_dialog_discard (dialog, user_data);
    }
  else if (!g_strcmp0 (response, "save"))
    {
      gabc_save_changes_dialog_save_handler (dialog, user_data);
    }
    else
    {
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
  GabcWindow *parent;
  GTask *task;
  parent = (GabcWindow *)user_data;
  g_assert (GABC_IS_WINDOW (parent));

  task = g_object_get_data (G_OBJECT (dialog), "TASK");
  g_object_ref (task);
  g_assert (G_IS_TASK (task));

  if (gtk_source_file_get_location(parent->tunebook->abc_source_file) == NULL)
  {
    g_print (" save file dialog");
    gabc_save_changes_dialog_show_file_save_dialog (parent, task);
  }
  else
  {
    g_print ("save to abc file location\n");
    gabc_save_changes_dialog_save (parent, task);
  }
}

static void
gabc_save_changes_dialog_show_file_save_dialog (GabcWindow *parent, GTask *task)
{
  GtkFileDialog *gfd;
  GtkFileFilter *abc_filter;
  GListStore *filter_list;

  g_assert (G_IS_TASK (task));

  gfd = gtk_file_dialog_new ();
  gtk_file_dialog_set_title (gfd, "Save abc File");

  //g_object_ref (task);`

  abc_filter = gabc_file_filters_get_abc_file_filter();
  filter_list = gabc_file_filters_get_filter_list(abc_filter);

  gtk_file_dialog_set_filters (gfd, G_LIST_MODEL (filter_list));
  gtk_file_dialog_set_default_filter (gfd, abc_filter);

  g_object_set_data_full (G_OBJECT ( gfd ),
                        "TASK",
                        g_steal_pointer (&task),
                        g_object_unref);

  g_print ("about to open the save\n");
  gtk_file_dialog_save (gfd,
                        GTK_WINDOW (parent),
                        NULL,
                        gabc_save_changes_dialog_show_file_save_dialog_cb,
                        G_OBJECT (parent));
  g_object_unref (abc_filter);
  g_object_unref (filter_list);
}


static void
gabc_save_changes_dialog_show_file_save_dialog_cb (GObject *file_dialog,
                                 GAsyncResult  *res,
                                 gpointer       user_data)
{
  GTask *task;
  GabcWindow *self = user_data;

  task = g_object_get_data (G_OBJECT (file_dialog), "TASK");
  g_object_ref (task);
  g_assert (G_IS_TASK (task));

  g_autoptr (GFile) file = gtk_file_dialog_save_finish (GTK_FILE_DIALOG (file_dialog),
                                                        res,
                                                        NULL);
  if (file) {
    gtk_source_file_set_location(self->tunebook->abc_source_file, file);
    g_print ("Save the file now\n");
    gabc_save_changes_dialog_save (self, task);
  }
  else
  {
    g_print("handle a failed save");
    g_task_return_new_error (task,
                               G_IO_ERROR,
                               G_IO_ERROR_CANCELLED,
                               "The user cancelled the file save");
  }
  g_object_unref (file_dialog);
}


static void
gabc_save_changes_dialog_save (GabcWindow *self, GTask *task)
{
  GtkSourceFileSaver *saver = gtk_source_file_saver_new (
                                  (GtkSourceBuffer *) self->tunebook,
                                  self->tunebook->abc_source_file);

  g_assert (G_IS_TASK (task));

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
 
