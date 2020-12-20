#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include "handle-event.c"

extern GtkWidget *window;
extern GtkWidget *frame;
extern GtkWidget *chatArea;
extern GtkWidget *messageInput;
extern char *you;
extern char *currentChannel;
char onlineUsers[USER_NUM_MAX][32];
extern int onlineUserCount;
extern char *publicStream;
GtkWidget *userListBox;
GtkWidget *loginDialog = NULL;
GtkWidget *registerDialog = NULL;
GtkWidget *inputUsername;
GtkWidget *inputPassword;
GtkWidget *inputRePassword;
GtkWidget *yournameLabel;
GtkWidget *publicChannelButton;
GtkWidget *userListScroller;
GtkWidget *chatOutputScroller;

GMutex mutex_interface;

void set_size(GtkWidget *gw, int width, int height)
{
	gtk_widget_set_size_request(gw, width, height);
}

void set_pos(GtkWidget *gw, int x, int y)
{
	gtk_fixed_put(GTK_FIXED(frame), gw, x, y);
}

void destroySomething(GtkWidget *widget, gpointer gp)
{
	gtk_widget_destroy(gp);
}

void showMessage(GtkWidget *parent, GtkMessageType type, char *mms, char *content)
{
	GtkWidget *mdialog;
	mdialog = gtk_message_dialog_new(GTK_WINDOW(parent),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 type,
									 GTK_BUTTONS_OK,
									 "%s", mms);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(mdialog), "%s", content);
	gtk_widget_show_all(mdialog);
	gtk_dialog_run(GTK_DIALOG(mdialog));
	gtk_widget_destroy(mdialog);
}

void initLoginDialog()
{
	loginDialog = gtk_dialog_new_with_buttons(LOGIN, GTK_WINDOW(window), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL, NULL);

	GtkWidget *dialog_ground = gtk_fixed_new();
	GtkWidget *tframe = gtk_frame_new(USERNAME);
	GtkWidget *bframe = gtk_frame_new(PASSWORD);
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *loginButton = gtk_button_new_with_label(LOGIN);
	GtkWidget *cancelButton = gtk_button_new_with_label(CANCEL);
	GtkWidget *registerButton = gtk_button_new_with_label(REGISTER);
	inputUsername = gtk_entry_new();
	inputPassword = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(inputPassword), FALSE);

	gtk_box_pack_start(GTK_BOX(box), loginButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(box), cancelButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(box), registerButton, TRUE, TRUE, 2);

	set_size(tframe, 300, 50);
	set_size(bframe, 300, 50);
	set_size(box, 300, 50);
	set_size(loginButton, 100, 30);
	set_size(cancelButton, 100, 30);
	set_size(registerButton, 100, 30);

	gtk_widget_set_margin_start(inputUsername, 2);
	gtk_widget_set_margin_end(inputUsername, 2);
	gtk_widget_set_margin_start(inputPassword, 2);
	gtk_widget_set_margin_end(inputPassword, 2);

	gtk_fixed_put(GTK_FIXED(dialog_ground), tframe, 0, 20);
	gtk_fixed_put(GTK_FIXED(dialog_ground), bframe, 0, 80);
	gtk_fixed_put(GTK_FIXED(dialog_ground), box, 0, 150);

	gtk_container_add(GTK_CONTAINER(tframe), inputUsername);
	gtk_container_add(GTK_CONTAINER(bframe), inputPassword);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(loginDialog))), dialog_ground, TRUE, TRUE, 0);

	GtkWidget *data_array[3];
	data_array[0] = inputUsername;
	data_array[1] = inputPassword;
	data_array[2] = loginDialog;
	g_signal_connect(loginButton, "clicked", G_CALLBACK(handleLoginButtonClicked), data_array);
	g_signal_connect(cancelButton, "clicked", G_CALLBACK(onExit), loginDialog);
	g_signal_connect(registerButton, "clicked", G_CALLBACK(handleRegisterButtonClicked), registerDialog);
	g_signal_connect(inputUsername, "activate", G_CALLBACK(handleLoginButtonClicked), data_array);
	g_signal_connect(inputPassword, "activate", G_CALLBACK(handleLoginButtonClicked), data_array);
	g_signal_connect(loginDialog, "destroy", G_CALLBACK(onExit), NULL); //Ket thuc chuong trinh khi dong cua so chinh
	// gtk_widget_show_all(loginDialog);
	// gtk_dialog_run(GTK_DIALOG(loginDialog));
	// gtk_widget_destroy(loginDialog);
}

void initRegisterDialog()
{
	registerDialog = gtk_dialog_new_with_buttons(REGISTER, GTK_WINDOW(window), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL, NULL);

	GtkWidget *dialog_ground = gtk_fixed_new();
	GtkWidget *tframe = gtk_frame_new(USERNAME);
	GtkWidget *bframe = gtk_frame_new(PASSWORD);
	// GtkWidget *bframe = gtk_frame_new(PASSWORD);
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *registerButton = gtk_button_new_with_label(REGISTER);
	GtkWidget *cancelButton = gtk_button_new_with_label("BACK");
	inputUsername = gtk_entry_new();
	inputPassword = gtk_entry_new();
	// inputRePassword = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(inputPassword), FALSE);
	// gtk_entry_set_visibility(GTK_ENTRY(inputRePassword), FALSE);

	gtk_box_pack_start(GTK_BOX(box), registerButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(box), cancelButton, TRUE, TRUE, 2);

	set_size(tframe, 300, 50);
	set_size(bframe, 300, 50);
	set_size(box, 300, 50);
	set_size(registerButton, 100, 30);
	set_size(cancelButton, 100, 30);

	gtk_widget_set_margin_start(inputUsername, 2);
	gtk_widget_set_margin_end(inputUsername, 2);
	gtk_widget_set_margin_start(inputPassword, 2);
	gtk_widget_set_margin_end(inputPassword, 2);
	// gtk_widget_set_margin_start(inputRePassword, 2);
	// gtk_widget_set_margin_end(inputRePassword, 2);

	gtk_fixed_put(GTK_FIXED(dialog_ground), tframe, 0, 20);
	gtk_fixed_put(GTK_FIXED(dialog_ground), bframe, 0, 80);
	gtk_fixed_put(GTK_FIXED(dialog_ground), box, 0, 220);

	gtk_container_add(GTK_CONTAINER(tframe), inputUsername);
	gtk_container_add(GTK_CONTAINER(bframe), inputPassword);
	// gtk_container_add(GTK_CONTAINER(bframe), inputRePassword);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(registerDialog))), dialog_ground, TRUE, TRUE, 0);

	GtkWidget *data_array[3];
	data_array[0] = inputUsername;
	data_array[1] = inputPassword;
	data_array[2] = registerDialog;
	g_signal_connect(registerButton, "clicked", G_CALLBACK(handleRegisterAccountClicked), data_array);
	g_signal_connect(cancelButton, "clicked", G_CALLBACK(backLogin), registerDialog);
	g_signal_connect(inputUsername, "activate", G_CALLBACK(handleRegisterAccountClicked), data_array);
	g_signal_connect(inputPassword, "activate", G_CALLBACK(handleRegisterAccountClicked), data_array);
	g_signal_connect(registerDialog, "destroy", G_CALLBACK(onExit), NULL); //Ket thuc chuong trinh khi dong cua so chinh
}

void showLoginDialog()
{
	if (loginDialog == NULL)
		initLoginDialog();
	gtk_widget_show_all(loginDialog);
	gtk_dialog_run(GTK_DIALOG(loginDialog));
	return;
}

void showRegisterDialog()
{
	if (registerDialog == NULL)
		initRegisterDialog();
	// puts("Show register dialog");
	gtk_widget_show_all(registerDialog);
	gtk_dialog_run(GTK_DIALOG(registerDialog));
	return;
}

GtkWidget *initMessageInput(int x, int y)
{
	GtkWidget *inputGroupBox;
	GtkWidget *inputBox;
	GtkWidget *sendButton;

	//Khoi tao inputGroupBox
	inputGroupBox = gtk_frame_new(TEXT_INPUT_LABEL);
	set_size(inputGroupBox, 450, 60);
	set_pos(inputGroupBox, x, y);

	//Khoi tao inputBox chua o text va nut send
	inputBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(GTK_CONTAINER(inputGroupBox), inputBox);

	//Khoi tao o nhap va chatArea hien thi nghia
	messageInput = gtk_entry_new();
	set_size(messageInput, 388, 20);

	//send button
	sendButton = gtk_button_new_with_label(SEND_LABEL);
	gtk_widget_set_margin_bottom(sendButton, 5);
	gtk_widget_set_margin_top(sendButton, 0);

	gtk_box_pack_start(GTK_BOX(inputBox), messageInput, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(inputBox), sendButton, TRUE, TRUE, 5);

	g_signal_connect(sendButton, "clicked", G_CALLBACK(onSendButtonClicked), NULL);
	g_signal_connect(messageInput, "activate", G_CALLBACK(onSendButtonClicked), NULL);
	return messageInput;
}

void textViewSetText(GtkWidget *textView, char *text)
{
	char *x, *q;
	GtkTextBuffer *t_buffer;
	GtkTextIter start;
	GtkTextIter end;
	t_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
	if (t_buffer == NULL)
	{
		printf("Get buffer fail!");
		t_buffer = gtk_text_buffer_new(NULL);
	}
	gtk_text_buffer_set_text(t_buffer, text, -1);
	int i, lineCount = gtk_text_buffer_get_line_count(t_buffer);
	GtkTextTag *tag = gtk_text_buffer_create_tag(t_buffer, NULL,
												 "foreground", "blue",
												 "weight", PANGO_WEIGHT_BOLD, NULL);
	for (i = 0; i < lineCount - 1; ++i)
	{
		gtk_text_buffer_get_iter_at_line(t_buffer, &start, i);
		gtk_text_buffer_get_end_iter(t_buffer, &end);
		x = gtk_text_buffer_get_text(t_buffer, &start, &end, TRUE);
		q = strchr(x, ':');
		if (q > x &&  q < x + strlen(x))
		{
			gtk_text_buffer_get_iter_at_line_index(t_buffer, &end, i, q - x);
			gtk_text_buffer_apply_tag(t_buffer, tag, &start, &end);
		}
	}

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(textView), t_buffer);
	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(chatOutputScroller));
	gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));
	gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(chatOutputScroller), adj);
}

GtkWidget *initChatArea(int x, int y)
{
	GtkWidget *outputBox;

	//khoi tao hop chua chatArea hien thi khung chat
	outputBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	set_size(outputBox, 450, 280);
	set_pos(outputBox, x, y);

	//Khung chat
	chatArea = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(chatArea), GTK_WRAP_WORD_CHAR); //Chong tran be ngang

	//Khoi tao thanh keo truot cho chatArea
	chatOutputScroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(chatOutputScroller), chatArea);

	gtk_box_pack_start(GTK_BOX(outputBox), chatOutputScroller, TRUE, TRUE, 2);
	return chatArea;
}

void initCurrentUserBox(int x, int y)
{
	GtkWidget *currentUserGroupBox;
	GtkWidget *logoutButton;
	GtkWidget *containBox;
	currentUserGroupBox = gtk_frame_new(YOU);
	containBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(currentUserGroupBox), containBox);

	set_size(currentUserGroupBox, 115, 64);
	set_pos(currentUserGroupBox, x, y);
	yournameLabel = gtk_label_new(you);

	logoutButton = gtk_button_new_with_label(LOGOUT);
	set_size(logoutButton, 110, 34);
	// set_pos(logoutButton, 5, 30);

	gtk_box_pack_start(GTK_BOX(containBox), yournameLabel, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(containBox), logoutButton, TRUE, TRUE, 0);

	g_signal_connect(logoutButton, "clicked", G_CALLBACK(handleLogoutButtonClicked), NULL);
}

GtkWidget *initPublicChannelBox(int x, int y)
{
	GtkWidget *publicChannelGroupBox;
	publicChannelGroupBox = gtk_frame_new(PUBLIC);
	set_size(publicChannelGroupBox, 115, 54);
	set_pos(publicChannelGroupBox, x, y);

	publicChannelButton = gtk_button_new_with_label(PUBLIC);
	gtk_container_add(GTK_CONTAINER(publicChannelGroupBox), publicChannelButton);

	g_signal_connect(publicChannelButton, "clicked", G_CALLBACK(onChannelButtonClicked), PUBLIC);
	return publicChannelButton;
}

void delFromUserBox(gpointer child, gpointer user_data)
{
	gtk_container_remove(GTK_CONTAINER(userListBox), (GtkWidget *)child);
}

int setButtonFocus(GtkWidget *button, char *s)
{
	GdkRGBA color;
	gdk_rgba_parse(&color, s);
	gtk_widget_override_background_color(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
	// context = gtk_widget_get_style_context(button);
	// gtk_style_context_add_class(context,DOWN);
	return 0;
}
void addButtonToUserListBox(char n[][32], int count)
{
	int i = 0;
	if (strcmp(PUBLIC, currentChannel) == 0)
	{
		setButtonFocus(publicChannelButton, DOWN);
	}
	else
		setButtonFocus(publicChannelButton, UP);
	for (i = 0; i < count; ++i)
	{
		if (strcmp(n[i], you) != 0)
		{
			// puts("\t|");
			// printf("\t+--->{%s}{%s}\n", n[i], you);
			GtkWidget *userIndex = gtk_button_new_with_label(n[i]);
			set_size(userIndex, 110, 50);
			if (strcmp(n[i], currentChannel) == 0)
			{
				setButtonFocus(userIndex, DOWN);
			}
			gtk_box_pack_start(GTK_BOX(userListBox), userIndex, FALSE, FALSE, 0);
			g_signal_connect(userIndex, "clicked", G_CALLBACK(onChannelButtonClicked), n[i]);
		}
	}
	// printf("Setup success\n");
	gtk_widget_show_all(userListBox);
}

void updateUserList(char n[][32], int count)
{
	GList *childs = gtk_container_get_children(GTK_CONTAINER(userListBox));
	g_list_foreach(childs, delFromUserBox, NULL);
	addButtonToUserListBox(n, count);
}
GtkWidget *initUserList(int x, int y, char names[][32], int amount)
{
	GtkWidget *userListGroupBox;

	// Khoi tao List user groupbox
	userListGroupBox = gtk_frame_new(ONLINE_LIST_LABEL);
	set_size(userListGroupBox, 115, 186);
	set_pos(userListGroupBox, x, y);

	//Khoi tao hop chua cac nut
	userListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	//Khoi tao thanh keo truot cho chatArea
	userListScroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(userListScroller), userListBox);

	gtk_container_add(GTK_CONTAINER(userListGroupBox), userListScroller);
	addButtonToUserListBox(names, onlineUserCount);

	return userListGroupBox;
}

void initMainWindow()
{
	// Khoi tao cua so
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	// gtk_window_set_default_size(GTK_WINDOW(window), -1, -1);
	gtk_window_set_title(GTK_WINDOW(window), APP_TITLE);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	//Khoi tao nen
	frame = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window), frame);
	gtk_widget_set_margin_bottom(frame, 5);
	gtk_widget_set_margin_end(frame, 5);

	// Include username and Logout button
	initCurrentUserBox(5, 4);
	// Include public channel and hanlder event public
	initPublicChannelBox(5, 94);
	initChatArea(120, 10);

	initUserList(5, 154, onlineUsers, 7);
	initMessageInput(120, 280);
	
	// Out program
	g_signal_connect(window, "destroy", G_CALLBACK(onExit), NULL); //Ket thuc chuong trinh khi dong cua so chinh
}

void showMainWindow()
{
	if (window == NULL)
	{
		initMainWindow();
	}
	gtk_widget_show_all(window);
	gtk_entry_grab_focus_without_selecting(GTK_ENTRY(messageInput));
}