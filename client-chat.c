#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <string.h>
#include <stdio.h>
#include "string-constant.h"
#include "integer-constant.h"
#include "gui-design.c"


GtkWidget *window = NULL;
GtkWidget *frame;
GtkWidget *chatArea;
GtkWidget *messageInput;

char *you = "abc";
char *currentChannel = PUBLIC;			// Tro ve channel can thiet
char onlineUsers[USER_NUM_MAX][32];		// Users online
User onlineUsersStream[USER_NUM_MAX];	// Dung struct User
char *publicStream;
int onlineUserCount = 0;		// Dem user online


int main(int argc, char *argv[])
{

	//clear messageStream
	memset(onlineUsersStream, 0, USER_NUM_MAX * sizeof(User));
	//to be thread-aware
  if (!g_thread_supported ()){ g_thread_init(NULL); }
	// initialize GDK thread support
	gdk_threads_init();		// start thread first of 1 client user

	publicStream = (char *)malloc(1024 * MAXLINE);
	// Create client and connect to server
	createClient(argc, argv);
	// set timeout
	g_timeout_add(100, (GSourceFunc)timer_exe, NULL);
	gtk_init(&argc, &argv);

	// acquire the GDK lock
	gdk_threads_enter();

	initMainWindow();
	showLoginDialog();
	// start
	gtk_main();
	// release the GDK lock
	gdk_threads_leave();
	return 0;
}