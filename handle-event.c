#include <gtk/gtk.h>

extern GtkWidget *window;
extern GtkWidget *frame;
extern GtkWidget *userListBox;
extern GtkWidget * initUserList(int , int , char * [], int );
extern void addButtonToUserListBox(char * [], int);



void onLogoutButtonClicked(GtkWidget * widget, gpointer * data){
	destroySomething(NULL, window);
	showLoginDialog();
}



/**
Handle user click to login button
@param 	widget: button clicked
		gp: passed data array
*/
void onLoginButtonClicked(GtkWidget * widget, gpointer gp){
	GtkWidget *loginDialog = ((GtkWidget**)gp)[2];
	GtkWidget* inputUsername = ((GtkWidget**)gp)[0];
	GtkWidget* inputPassword = ((GtkWidget**)gp)[1];

	//code here
	//success
	// showMessage(loginDialog, GTK_MESSAGE_INFO, LOGIN_SUCCESS, WELLCOME );
	//invalid
	// showMessage(loginDialog, GTK_MESSAGE_ERROR, LOGIN_FAILED, ACCOUNT_INVALID );
	//already login
	showMessage(loginDialog, GTK_MESSAGE_WARNING, LOGIN_SUCCESS, SESSION_INVALID );

	destroySomething(NULL, loginDialog);
	showMainWindow();
}

void onChannelButtonClicked(GtkWidget * widget, gpointer data){
	char * channel = (char*) data;
	showMessage(window, GTK_MESSAGE_INFO, "haha", channel);
}

void onSendButtonClicked(GtkWidget * widget, gpointer data){
	char * newlist[] = {"admin", "user"};
	updateUserList(newlist, 2);
}