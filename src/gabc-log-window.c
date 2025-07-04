#include <gtk/gtk.h>

#include "gabc-window.h"
#include "gabc-log-window.h"

struct _GabcLogWindow
{
  AdwWindow parent;

  GtkTextBuffer *log_buffer;
  GtkButton     *log_clear_button;

};

G_DEFINE_TYPE (GabcLogWindow, gabc_log_window, ADW_TYPE_WINDOW)

void
gabc_log_window_append_to_log (GabcLogWindow *self, char *text)
{
  if (text != NULL)
  {
    gtk_text_buffer_insert_at_cursor (self->log_buffer, text, -1);
    gtk_text_buffer_insert_at_cursor (self->log_buffer, "\n", -1);
  }
}

static void
gabc_log_window_clear_log (GtkWidget *widget,
                           gpointer   data)
{
  GabcLogWindow *self = GABC_LOG_WINDOW (data);
  g_assert (GABC_IS_LOG_WINDOW (self));
  gtk_text_buffer_set_text (self->log_buffer, "", -1);
}


static void
gabc_log_window_init (GabcLogWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->log_clear_button, "clicked", G_CALLBACK (gabc_log_window_clear_log), self);
}

static void
gabc_log_window_dispose (GObject *gobject)
{
  gtk_widget_dispose_template (GTK_WIDGET (gobject), GABC_LOG_WINDOW_TYPE);

  G_OBJECT_CLASS (gabc_log_window_parent_class)->dispose (gobject);
}

static void
gabc_log_window_class_init (GabcLogWindowClass *klass)
{
  GtkWidgetClass *widget_class;

  G_OBJECT_CLASS (klass)->dispose = gabc_log_window_dispose;

  widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/me/pm/m0dns/gabc/gabc-log-window.ui");

  gtk_widget_class_bind_template_child (widget_class,
                                        GabcLogWindow,
                                        log_buffer);
  gtk_widget_class_bind_template_child (widget_class,
                                        GabcLogWindow,
                                        log_clear_button);

}

GabcLogWindow *
gabc_log_window_new (AdwApplicationWindow *win)
{
  return g_object_new (GABC_LOG_WINDOW_TYPE, "transient-for", win,
                                             "title", "Log Viewer",
                                             "hide-on-close", true,
                                             "destroy_with_parent", true,
                                             NULL);
}
