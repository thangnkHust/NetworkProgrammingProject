#include <gtk/gtk.h>

static void onClicked(GtkWidget *widget, gpointer *data)
{
  g_print("Clicked\n");
}

static void
activate(GtkApplication *app, gpointer user_data)
{
  GtkWidget *window;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Hello GTK");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

  GtkWidget *button, *button_box, *label;
  label = gtk_label_new("Hello World");
  button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
  button = gtk_button_new_with_label("Click me");

  gtk_container_add(GTK_CONTAINER(button_box), button);

  GtkWidget *containerBox;
  containerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

  gtk_box_pack_start(containerBox, label, FALSE, FALSE, 50);
  gtk_box_pack_start(containerBox, button_box, FALSE, FALSE, 10);

  gtk_container_add(GTK_CONTAINER(window), containerBox);

  g_signal_connect(button, "clicked", G_CALLBACK(onClicked), NULL);

  gtk_widget_show_all(window);
}

int main(int argc, char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new("hello.world", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}