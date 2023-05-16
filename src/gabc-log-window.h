

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

//#define EXAMPLE_APP_PREFS_TYPE (example_app_prefs_get_type ())

#define GABC_LOG_WINDOW_TYPE (gabc_log_window_get_type ())

G_DECLARE_FINAL_TYPE (GabcLogWindow, gabc_log_window, GABC, LOG_WINDOW, GtkWindow)

GabcLogWindow        *gabc_log_window_new          (GabcWindow *win);

void
gabc_log_window_append_to_log (GabcLogWindow *self, gchar *text);

G_END_DECLS
