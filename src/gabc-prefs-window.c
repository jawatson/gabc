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

  GtkWidget *abc2midi_barfly_mode_combo;
  GtkWidget *abc2midi_midi_program_combo;

  AdwActionRow *abcm2ps_fmt_file_action_row;
  GtkButton *abcm2ps_fmt_clear_btn;
  GtkButton *abcm2ps_fmt_file_btn;
};


static void
gabc_prefs_clear_fmt_file_path (GtkWidget *widget,
                                gpointer   user_data);

GtkFileFilter *
get_fmt_file_filter (void);

GListStore *
get_fmt_filter_list (GtkFileFilter *fmt_filter);

static void
gabc_prefs_window_select_fmt_file (GabcPrefsWindow *self);

static void
file_open_cb (GObject       *file_dialog,
              GAsyncResult  *res,
              gpointer       self);


static void
gabc_prefs_set_fmt_file_path (GtkWidget *widget,
                                gpointer   user_data);

G_DEFINE_TYPE (GabcPrefsWindow, gabc_prefs_window, ADW_TYPE_PREFERENCES_DIALOG)


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

  g_settings_bind (self->settings, "abcm2ps-fmt-file-path",
                   self->abcm2ps_fmt_file_action_row, "subtitle",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->settings, "abc2midi-barfly-mode",
                   self->abc2midi_barfly_mode_combo, "active-id",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->settings, "abc2midi-midi-program",
                   self->abc2midi_midi_program_combo, "active-id",
                   G_SETTINGS_BIND_DEFAULT);

  //g_assert (GABC_IS_PREFS_WINDOW (self));
  g_signal_connect (self->abcm2ps_fmt_file_btn, "clicked", G_CALLBACK (gabc_prefs_set_fmt_file_path), self);
  g_signal_connect (self->abcm2ps_fmt_clear_btn, "clicked", G_CALLBACK (gabc_prefs_clear_fmt_file_path), self);

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
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), GabcPrefsWindow, abcm2ps_fmt_file_action_row);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), GabcPrefsWindow, abcm2ps_fmt_clear_btn);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), GabcPrefsWindow, abcm2ps_fmt_file_btn);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), GabcPrefsWindow, abcm2ps_page_number_combo);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), GabcPrefsWindow, abc2midi_barfly_mode_combo);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), GabcPrefsWindow, abc2midi_midi_program_combo);
}


GabcPrefsWindow *
gabc_prefs_window_new (GabcWindow *parent)
{
  return g_object_new (GABC_PREFS_WINDOW_TYPE, NULL);
}


static void
gabc_prefs_clear_fmt_file_path (GtkWidget *widget,
                                gpointer   user_data)
{
  GabcPrefsWindow *self = GABC_PREFS_WINDOW (user_data);
  //g_assert (GABC_IS_PREFS_WINDOW (self));
  g_settings_set_string (self->settings, "abcm2ps-fmt-file-path", "");
}

static void
gabc_prefs_set_fmt_file_path (GtkWidget *widget,
                                gpointer   user_data)
{
  GabcPrefsWindow *self = GABC_PREFS_WINDOW (user_data);
  //g_assert (GABC_IS_PREFS_WINDOW (self));
  g_print ("setting the path\n");
  gabc_prefs_window_select_fmt_file (self);
}

GtkFileFilter *
get_fmt_file_filter (void)
{
  GtkFileFilter *fmt_filter = gtk_file_filter_new();
  gtk_file_filter_add_pattern(fmt_filter, "*.fmt");
  return fmt_filter;
}


GListStore *
get_fmt_filter_list (GtkFileFilter *fmt_filter)
{
  GListStore *filter_list = g_list_store_new( G_TYPE_OBJECT );
  g_list_store_append (filter_list, G_OBJECT (fmt_filter));
  return filter_list;
}


static void
gabc_prefs_window_select_fmt_file (GabcPrefsWindow *self)
{
  GtkFileDialog *gfd;
  GtkFileFilter *fmt_filter;
  GListStore *filter_list;

  //GabcWindow *self = user_data;
  gfd = gtk_file_dialog_new ();
  gtk_file_dialog_set_title ( gfd, "Open abc File");

  fmt_filter = get_fmt_file_filter();
  filter_list = get_fmt_filter_list(fmt_filter);

  gtk_file_dialog_set_filters (gfd, G_LIST_MODEL (filter_list));
  gtk_file_dialog_set_default_filter (gfd, fmt_filter);

  gtk_file_dialog_open (gfd,
                        GTK_WINDOW (self),
                        NULL,
                        file_open_cb,
                        G_OBJECT (self));

  g_object_unref (fmt_filter);
  g_object_unref (filter_list);
}


static void
file_open_cb (GObject       *file_dialog,
              GAsyncResult  *res,
              gpointer       data)
{
  const gchar *fmt_file_path;

  GabcPrefsWindow *self = GABC_PREFS_WINDOW (data);
  g_autoptr (GFile) file = gtk_file_dialog_open_finish (GTK_FILE_DIALOG (file_dialog),
                                                        res,
                                                        NULL);
  if (file) {
    fmt_file_path = g_file_get_path(file);
    g_print ("%s\n", fmt_file_path);
    g_settings_set_string (self->settings, "abcm2ps-fmt-file-path", fmt_file_path);
  }

  //g_free (fmt_file_path);
  g_object_unref (file_dialog);
}

