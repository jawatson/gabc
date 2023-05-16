#include <gtk/gtk.h>

#include "gabc-window.h"
#include "gabc-log-window.h"

struct _GabcLogWindow
{
  GtkWindow parent;

  GtkTextBuffer *log_buffer;

};

G_DEFINE_TYPE (GabcLogWindow, gabc_log_window, GTK_TYPE_WINDOW)

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
gabc_log_window_init (GabcLogWindow *win)
{
  gtk_widget_init_template (GTK_WIDGET (win));

}


static void
gabc_log_window_class_init (GabcLogWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/me/pm/m0dns/gabc/gabc-log-window.ui");

  gtk_widget_class_bind_template_child (widget_class,
                                              GabcLogWindow,
                                              log_buffer);
}

GabcLogWindow *
gabc_log_window_new (GabcWindow *win)
{
  return g_object_new (GABC_LOG_WINDOW_TYPE, "transient-for", win,
                                             "title", "Log Viewer",
                                             "resizable", true,
                                             "hide-on-close", true,
                                             "destroy_with_parent", true,
                                             NULL);
}
