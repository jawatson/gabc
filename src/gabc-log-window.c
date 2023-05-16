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
  gtk_text_buffer_insert_at_cursor (self->log_buffer, text, -1);
  gtk_text_buffer_insert_at_cursor (self->log_buffer, "\n", -1);
}

static void
gabc_log_window_init (GabcLogWindow *win)
{
  //gtk_widget_init_template (GTK_WIDGET (win));

  GtkTextView *log_text_view;
  GtkScrolledWindow *scrolled_window;

  log_text_view = GTK_TEXT_VIEW (gtk_text_view_new ());
  gtk_text_view_set_editable(log_text_view, false);
  gtk_text_view_set_monospace (log_text_view, true);
  gtk_text_view_set_left_margin (log_text_view, 10);
  gtk_text_view_set_right_margin (log_text_view, 10);

  win->log_buffer = gtk_text_view_get_buffer (log_text_view);

  scrolled_window = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new ());
  gtk_scrolled_window_set_child (scrolled_window, GTK_WIDGET (log_text_view));
  gtk_scrolled_window_set_min_content_width (scrolled_window, 600);
  gtk_scrolled_window_set_min_content_height (scrolled_window, 600);
  gtk_scrolled_window_set_policy (scrolled_window ,
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_ALWAYS) ;

 gtk_window_set_child (GTK_WINDOW (win), GTK_WIDGET (scrolled_window));

}


static void
gabc_log_window_class_init (GabcLogWindowClass *class)
{

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
