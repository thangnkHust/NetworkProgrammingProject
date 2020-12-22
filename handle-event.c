#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "string-constant.h"
#include "integer-constant.h"
#include "client.c"
// #include "server.c"

#define runInUIThread(x) \
	gdk_threads_enter(); \
	x;                   \
	gdk_threads_leave();

extern GtkWidget *window;
extern GtkWidget *frame;
extern GtkWidget *userListBox;
extern char *inBuf;
extern GtkWidget *initUserList(int, int, char[][32], int);
extern void addButtonToUserListBox(char[][32], int);
extern void sendThread(char *);
extern GtkWidget *loginDialog;
extern GtkWidget *registerDialog;
extern GtkWidget *inputUsername;
extern GtkWidget *inputPassword;
extern GtkWidget *inputUsernameRegister;
extern GtkWidget *inputPasswordRegister;
extern GtkWidget *yournameLabel;
extern GtkWidget *messageInput;
extern char *you;
extern char onlineUsers[USER_NUM_MAX][32];
extern int onlineUserCount;
extern GtkWidget *publicChannelButton;
extern char *currentChannel;
extern char *publicStream;
extern GtkWidget *chatArea;
extern void clearBuf(char *);
extern int sendRequest();
extern void showLoginDialog();
extern void showRegisterDialog();
extern void showMessage(GtkWidget *, GtkMessageType, char *, char *);
extern char * saveToUserMessageStream(char *, char *);
extern void showMainWindow();
extern void updateUserList(char[][32], int);
extern int setButtonFocus(GtkWidget *, char *);
extern int findUserMessageStream(char * );
extern User onlineUsersStream[USER_NUM_MAX];
char username[100];
char password[100];
char *ltrim(char *string, char junk);

void clearStreamList(){
	int i; 
	for(i = 0; i < USER_NUM_MAX; i++){
		if(strlen(onlineUsersStream[i].username) > 0){
			memset(onlineUsersStream[i].username, 0, strlen(onlineUsersStream[i].username));
			free(onlineUsersStream[i].stream);
		}
	}
}
void handleLogoutButtonClicked(GtkWidget *widget, gpointer *data)
{
	// destroySomething(NULL, window);
	gtk_widget_hide(window);		// hide mainChat widget --> show loginWidget 
	clearBuf(inBuf);
	clearStreamList();
	currentChannel = PUBLIC;
	sprintf(inBuf, "%c", LOGOUT_ACTION);
	sendRequest();
	showLoginDialog();
}
// TODO: Login success
int onLoginSuccess(char *message)
{
	//success
	runInUIThread(showMessage(loginDialog, GTK_MESSAGE_WARNING, LOGIN_SUCCESS, WELLCOME));
	runInUIThread(gtk_entry_set_text(GTK_ENTRY(inputPassword), BLANK));
	runInUIThread(showMainWindow());
	runInUIThread(gtk_widget_set_visible(loginDialog, FALSE));
	gtk_label_set_text(GTK_LABEL(yournameLabel), username);
	clearBuf(inBuf);
	sprintf(inBuf, "%c", GET_PUBLIC_STREAM);
	sendRequest();
	// return 0;
}

int onRegisterSuccess()
{
	//success
	runInUIThread(showMessage(registerDialog, GTK_MESSAGE_ERROR, REGISTER_SUCCESS, BACK_LOGIN));
	runInUIThread(gtk_widget_hide(registerDialog));
	gtk_widget_show_all(loginDialog);
	// return 0;
}

void *showBubbleNotify(void *notify){	
	char command[200];
	sprintf(command, "notify-send \"%s\"", notify);
	system(command);
}
int onUserTagged(char * sender){
	char notify[69];
	sprintf(notify, TAGGED_NOTIFY, sender);
	showBubbleNotify(notify);
}

int onSentUsername()
{
	you = username;
	clearBuf(inBuf);
	sprintf(inBuf, "%c%s", SEND_PASSWORD_ACTION, password);
	sendRequest();
}

// TODO: send password register
int onSendPasswordRegister(){
	clearBuf(inBuf);
	sprintf(inBuf, "%c%s", SEND_PASSWORD_REGISTER_ACTION, password);
	sendRequest();
}
int onLoginFailed(char *message)
{
	//invalid
	runInUIThread(showMessage(loginDialog, GTK_MESSAGE_ERROR, LOGIN_FAILED, message));
	// showMessage(loginDialog, GTK_MESSAGE_ERROR, LOGIN_FAILED, message);
}
int onRegisterFailed(char *message)
{
	//invalid
	runInUIThread(showMessage(registerDialog, GTK_MESSAGE_ERROR, REGISTER_FAILED, message));
	// showMessage(loginDialog, GTK_MESSAGE_ERROR, LOGIN_FAILED, message);
}
void handleLoginButtonClicked(GtkWidget *widget, gpointer gp)
{
	strcpy(username, (char *)gtk_entry_get_text(GTK_ENTRY(inputUsername)));
	strcpy(password, (char *)gtk_entry_get_text(GTK_ENTRY(inputPassword)));

	if (strlen(username) < 1 || strlen(password) < 1)
		showMessage(loginDialog, GTK_MESSAGE_WARNING, LOGIN_FAILED, NOT_EMPTY);
	else
	{
		clearBuf(inBuf);
		sprintf(inBuf, "%c%s", SEND_USER_ACTION, username);
		sendRequest();
	}
}

void handleRegisterButtonClicked(GtkWidget *widget, gpointer gp)
{
	// puts("Hello Register");
	gtk_widget_hide(loginDialog);
	showRegisterDialog();
}

void handleRegisterAccountClicked(GtkWidget *widget, gpointer gp)
{
	strcpy(username, (char *)gtk_entry_get_text(GTK_ENTRY(inputUsernameRegister)));
	strcpy(password, (char *)gtk_entry_get_text(GTK_ENTRY(inputPasswordRegister)));

	if (strlen(username) < 1 || strlen(password) < 1)
		showMessage(registerDialog, GTK_MESSAGE_WARNING, REGISTER_FAILED, NOT_EMPTY);
	else
	{
		clearBuf(inBuf);
		sprintf(inBuf, "%c%s", SEND_USER_REGISTER_ACTION, username); 
		sendRequest();
	}
}

void onChannelButtonClicked(GtkWidget *widget, gpointer data)
{
	int i = 0, count ;
	char name[50];
	currentChannel = (char *)data;	// Duoc truyen khi click vao channel
	char * x = strchr(currentChannel, BRACKET);
	if (x != NULL)
	{
		*x = '\0'; // Remove message count from user name example: hau(2) => hau
	}
	// showMessage(window, GTK_MESSAGE_INFO, "haha", currentChannel);
	// printf("***%d\n", onlineUserCount);
	for(i = 0; i< onlineUserCount; i++){
		puts(onlineUsers[i]);
		sscanf(onlineUsers[i], "%[^(](%d", name, &count);	// Sao chep du lieu tu onlineUser -> name, count
		// if (strcmp(name, currentChannel) == 0)
		// {
			// strcpy(onlineUsers[i], currentChannel);
		// 	break;
		// }
  }
	if (strcmp(currentChannel, PUBLIC) == 0)
	{
		printf("setButtonFocus\n");
		setButtonFocus(publicChannelButton, DOWN);
		textViewSetText(chatArea, publicStream);
	}else{
		int id = findUserMessageStream(currentChannel);
		if (id != -1)
		{
			onlineUsersStream[id].newMessage = 0;
			textViewSetText(chatArea, onlineUsersStream[id].stream);
		}
	}
	updateUserList(onlineUsers, onlineUserCount);
	// showMessage(window, GTK_MESSAGE_INFO, "haha", currentChannel);
}

void onExit(GtkWidget *widget, gpointer data)
{
	exit(0);
}

void backLogin(GtkWidget *widget, gpointer data){
	gtk_widget_hide(registerDialog);
	showLoginDialog();
}

void onSendButtonClicked(GtkWidget *widget, gpointer data)
{
	clearBuf(inBuf);
	char text[100];
	char *entryText;
	entryText = (char *)gtk_entry_get_text(GTK_ENTRY(messageInput));
	if(strlen(entryText) != 0){
		strcpy(text, entryText);
		gtk_entry_set_text(GTK_ENTRY(messageInput), BLANK);
		if (strcmp(currentChannel, PUBLIC) == 0)
			sprintf(inBuf, "%c%s", CHANNEL_MESSAGE_ACTION, text);
		else{
			char temp [MAXLINE];
			sprintf(temp, "%s:%s", you, text);
			char * xstream = saveToUserMessageStream(currentChannel, temp);
			printf("%s\n", xstream);
			textViewSetText(chatArea, xstream);
			sprintf(inBuf, "%c%s#%s", PRIVATE_MESSAGE_ACTION, currentChannel, text);
		}
		sendRequest();
	}
}

char *ltrim(char *string, char junk)
{
    char* original = string;
    char *p = original;
    int trimmed = 0;
    do
    {
        if (*original != junk || trimmed)
        {
            trimmed = 1;
            *p++ = *original;
        }
    }
    while (*original++ != '\0');
    return string;
}