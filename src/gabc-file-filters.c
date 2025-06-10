#include "gabc-file-filters.h"

GtkFileFilter *
gabc_file_filters_get_abc_file_filter (void)
{
  GtkFileFilter *abc_filter = gtk_file_filter_new();
  gtk_file_filter_add_pattern(abc_filter, "*.abc");
  return abc_filter;
}


GtkFileFilter *
gabc_file_filters_get_midi_file_filter (void)
{
  GtkFileFilter *midi_filter = gtk_file_filter_new();
  gtk_file_filter_add_pattern(midi_filter, "*.mid");
  gtk_file_filter_add_pattern(midi_filter, "*.midi");
  return midi_filter;
}


GListStore *
gabc_file_filters_get_filter_list (GtkFileFilter *filter)
{
  GListStore *filter_list = g_list_store_new( G_TYPE_OBJECT );
  g_list_store_append (filter_list, G_OBJECT (filter));
  return filter_list;
}
