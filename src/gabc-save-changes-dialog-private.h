/* gabc-save-changes-dialog-private.h
 *
 */

#pragma once

#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

void     _gabc_save_changes_dialog_run_async  (GtkWindow            *parent,
                                                 GCancellable         *cancellable,
                                                 GAsyncReadyCallback  callback,
                                                 gpointer              user_data);

gboolean _gabc_save_changes_dialog_run_finish (GAsyncResult         *result,
                                                 GError              **error);

G_END_DECLS
