#include <gtk/gtk.h>
#include <gtk/gtklayoutmanager.h>
#include <gtk/gtkshortcut.h>

#define APP_NAME "AI Chat"

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *chat_box = NULL;
  GtkWidget *input_box;
  GtkTextBuffer *chat_buffer;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), APP_NAME);
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

  // Define Chat View Window
  chat_box = gtk_text_view_new();
  chat_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_box));
  gtk_text_buffer_set_text(chat_buffer, "TEST", -1);
  gtk_box_append(GTK_BOX(vbox), chat_box);

  // Define User data entry window
  input_box = gtk_text_view_new();
  gtk_box_append(GTK_BOX(vbox), input_box);

  gtk_widget_set_visible(vbox, true);

  gtk_window_set_child(GTK_WINDOW(window), vbox);
  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("org.gtk.ai_chat", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
