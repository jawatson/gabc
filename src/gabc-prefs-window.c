#include <gtk/gtk.h>

#include "gabc-application.h"
#include "gabc-window.h"
#include "gabc-prefs-window.h"

struct _GabcPrefsWindow
{
  AdwWindow parent;

  GSettings *settings;
  GtkWidget *dark_btn;
  GtkWidget *abcm2ps_errors_switch;
  GtkWidget *abcm2ps_page_number_combo;

  GtkButton *fmt_dir_btn;
};

/*
static void
gabc_prefs_window_update_theme (GtkWidget *widget,
                                gpointer   user_data);
*/
G_DEFINE_TYPE (GabcPrefsWindow, gabc_prefs_window, ADW_TYPE_PREFERENCES_WINDOW)


static void
gabc_prefs_window_init (GabcPrefsWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  self->settings = g_settings_new ("me.pm.m0dns.gabc");

  g_settings_bind (self->settings, "dark-theme",
                   self->dark_btn, "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->settings, "abcm2ps-show-errors",
                   self->abcm2ps_errors_switch, "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->settings, "abcm2ps-page-numbering",
                   self->abcm2ps_page_number_combo, "active-id",
                   G_SETTINGS_BIND_DEFAULT);

  g_assert (GABC_IS_PREFS_WINDOW (self));
  //g_signal_connect (self->dark_btn, "state-set", G_CALLBACK (gabc_prefs_window_update_theme), self);
  /*
   music_dir_btn.clicked.connect (() => {
                pick_music_folder_async.begin (app, this, (dir) => {
                    music_dir_btn.label = get_display_name (dir);
                    app.reload_music_store ();
                }, (obj, res) => pick_music_folder_async.end (res));
            });
   */
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
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), GabcPrefsWindow, dark_btn);
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
gabc_prefs_window_update_theme (GtkWidget *widget,
                                gpointer   user_data)
{

  //g_assert (GABC_IS_PREFS_WINDOW (user_data));
  g_print ("it's ok");
  //GabcPrefsWindow *self = GABC_PREFS_WINDOW (user_data);
  //g_assert (GABC_IS_PREFS_WINDOW (self));
  g_print ("update the theme\n");
}
*/
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
