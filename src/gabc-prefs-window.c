#include <gtk/gtk.h>

#include "gabc-application.h"
#include "gabc-window.h"
#include "gabc-prefs-window.h"

struct _GabcPrefsWindow
{
  AdwWindow parent;

  GSettings *settings;
  GtkWidget *abcm2ps_errors_switch;
  GtkWidget *abcm2ps_page_number_combo;

  GtkButton *fmt_dir_btn;
  //GtkWidget *transition;
};

G_DEFINE_TYPE (GabcPrefsWindow, gabc_prefs_window, ADW_TYPE_PREFERENCES_WINDOW)

static void
gabc_prefs_window_init (GabcPrefsWindow *prefs)
{
  gtk_widget_init_template (GTK_WIDGET (prefs));
  prefs->settings = g_settings_new ("me.pm.m0dns.gabc");


  /*
   music_dir_btn.clicked.connect (() => {
                pick_music_folder_async.begin (app, this, (dir) => {
                    music_dir_btn.label = get_display_name (dir);
                    app.reload_music_store ();
                }, (obj, res) => pick_music_folder_async.end (res));
            });


   */

  g_settings_bind (prefs->settings, "abcm2ps-show-errors",
                   prefs->abcm2ps_errors_switch, "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (prefs->settings, "abcm2ps-page-numbering",
                   prefs->abcm2ps_page_number_combo, "active-id",
                   G_SETTINGS_BIND_DEFAULT);
 /*
  g_settings_bind (prefs->settings, "transition",
                   prefs->transition, "active-id",
                   G_SETTINGS_BIND_DEFAULT);
 * */
}

static void
gabc_prefs_window_dispose (GObject *object)
{
  GabcPrefsWindow *prefs;

  prefs = GABC_PREFS_WINDOW (object);

  g_clear_object (&prefs->settings);

  G_OBJECT_CLASS (gabc_prefs_window_parent_class)->dispose (object);
}

static void
gabc_prefs_window_class_init (GabcPrefsWindowClass *klass)
{
  G_OBJECT_CLASS (klass)->dispose = gabc_prefs_window_dispose;

  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
                                               "/me/pm/m0dns/gabc/gabc-prefs-window.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), GabcPrefsWindow, abcm2ps_errors_switch);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), GabcPrefsWindow, abcm2ps_page_number_combo);
}

GabcPrefsWindow *
gabc_prefs_window_new (GabcWindow *win)
{
  return g_object_new (GABC_PREFS_WINDOW_TYPE, "transient-for", win, NULL);
}

/*
static void
async void pick_music_folder_async (Application app, Gtk.Window? parent, FolderPicked picked) {
        var music_dir = app.get_music_folder ();
        var dialog = new Gtk.FileDialog ();
        dialog.initial_folder = music_dir;
        dialog.modal = true;
        try {
            var dir = yield dialog.select_folder (parent, null);
            if (dir != null && dir != music_dir) {
                app.settings?.set_string ("music-dir", ((!)dir).get_uri ());
                picked ((!)dir);
            }
        } catch (Error e) {
        }
    }
*/
