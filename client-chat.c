#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include "string-constant.c"
#include "gui-design.c"




GtkWidget *window;
GtkWidget *frame;
GtkWidget *chatArea;
GtkWidget *messageInput;

char * you = "abc";
char * onlineUsers[] = {"hajau", "tuanx", "agihi", "sep", "baclaocong", "hotboyxx", "occho"};
int onlineUserCount = 7;


int main(int argc, char const *argv[])
{
	// Khoi tao GTK
	gtk_init(&argc, &argv);
	

	showLoginDialog();


	
	
		

	//Gan su kien cho cac nut
	// g_signal_connect(sendButton, "clicked", G_CALLBACK(find_in_dict), NULL);
	// g_signal_connect(addButton, "clicked", G_CALLBACK(Show_add_dialog), NULL);
	// g_signal_connect(editButton, "clicked", G_CALLBACK(Show_edit_dialog), NULL);
	// g_signal_connect(delButton, "clicked", G_CALLBACK(Show_delete_dialog), NULL);
	// g_signal_connect(infoButton, "clicked", G_CALLBACK(Show_about_dialog), about_dialog);
	// g_signal_connect(messageInput, "key-press-event", G_CALLBACK(on_key_down), NULL);
	// g_signal_connect(messageInput, "activate", G_CALLBACK(find_in_dict), NULL);



	return 0;
}