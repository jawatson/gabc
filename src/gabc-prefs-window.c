#include <gtk/gtk.h>

#include "gabc-application.h"
#include "gabc-window.h"
#include "gabc-prefs-window.h"

struct _GabcPrefsWindow
{
  AdwWindow parent;

  GSettings *settings;
  GtkWidget *errors;
  //GtkWidget *transition;
};

G_DEFINE_TYPE (GabcPrefsWindow, gabc_prefs_window, ADW_TYPE_PREFERENCES_WINDOW)

static void
gabc_prefs_window_init (GabcPrefsWindow *prefs)
{
  gtk_widget_init_template (GTK_WIDGET (prefs));
  prefs->settings = g_settings_new ("me.pm.m0dns.gabc");

  g_settings_bind (prefs->settings, "errors",
                   prefs->errors, "active",
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
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), GabcPrefsWindow, errors);
  //gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), GabcPrefsWindow, transition);
}

GabcPrefsWindow *
gabc_prefs_window_new (GabcWindow *win)
{
  return g_object_new (GABC_PREFS_WINDOW_TYPE, "transient-for", win, NULL);
}


