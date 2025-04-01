
#pragma once

#include <gtk/gtk.h>
#include <adwaita.h>
#include "gabc-window.h"

G_BEGIN_DECLS

#define GABC_PREFS_WINDOW_TYPE (gabc_prefs_window_get_type ())

G_DECLARE_FINAL_TYPE (GabcPrefsWindow, gabc_prefs_window, GABC, PREFS_WINDOW, AdwPreferencesDialog)

GabcPrefsWindow        *gabc_prefs_window_new          (GabcWindow *win);

G_END_DECLS

