#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "jrb.h"
#include "btree.h"

GtkWidget *window;
GtkWidget *frame;
GtkWidget *about_dialog;
GtkWidget *textView;
GtkWidget *textSearch;
GtkListStore *list;
BTA * data = NULL;
time_t t_begin;
time_t t_end;
char t_string[500];
static char code[128] = { 0 };
JRB TEST_RESULT;
void beginTest(char * s) {
	strcpy(t_string, s);
	if (jrb_find_str(TEST_RESULT, s) == NULL) {
		jrb_insert_str(TEST_RESULT, strdup(s), new_jval_v(make_jrb()));
	}
	t_begin = clock();
}
void endTest() {
	t_end = clock();
	int time = (int)(t_end - t_begin);
	printf("%s: %lfs.\n", t_string, (double)(time) / 1000000);
	JRB tmp = jrb_find_str(TEST_RESULT, t_string);
	tmp = (JRB)jval_v(jrb_val(tmp));
	jrb_insert_int(tmp, (int)(time), JNULL);
}
void thonKeHieuSuatTrungBinh() {
	JRB ptr, ptmp, subptr;
	long totaltime;
	int counttest;
	printf(" ________________________________________________________\n");
	printf("| Tên hàm        |        Thời gian chạy trung bình      |\n");
	printf("|________________|_______________________________________|\n");
	jrb_traverse(ptr, TEST_RESULT) {
		if (strcmp("Sửa từ", jval_s(ptr->key) ) == 0)
			printf("| %-19s|", jval_s(ptr->key));
		else printf("| %-18s|", jval_s(ptr->key));
		totaltime = counttest = 0;
		ptmp = (JRB)jval_v(jrb_val(ptr));
		jrb_traverse(subptr, ptmp) {
			totaltime += jval_i(subptr->key);
			counttest++;
		}
		printf("%19lfs%20s\n",  (double)totaltime / counttest / 1000000, "|" );
		printf("|________________|_______________________________________|\n");
	}
}
const char* soundex(const char *s)
{
	static char out[5];
	int c, prev, i;

	out[0] = out[4] = 0;
	if (!s || !*s) return out;

	out[0] = *s++;

	/* first letter, though not coded, can still affect next letter: Pfister */
	prev = code[(int)out[0]];
	for (i = 1; *s && i < 4; s++) {
		if ((c = code[(int) * s]) == prev) continue;

		if (c == -1) prev = 0;	/* vowel as separator */
		else if (c > 0) {
			out[i++] = c + '0';
			prev = c;
		}
	}
	while (i < 4) out[i++] = '0';
	return out;
}

void add_code(const char *s, int c)
{
	while (*s) {
		code[(int)*s] = code[0x20 ^ (int) * s] = c;
		s++;
	}
}

void init()
{
	static const char *cls[] =
	{ "AEIOU", "", "BFPV", "CGJKQSXZ", "DT", "L", "MN", "R", 0};
	int i;
	for (i = 0; cls[i]; i++)
		add_code(cls[i], i - 1);
}


void set_size(GtkWidget * gw, int width, int height) {
	gtk_widget_set_size_request(gw, width, height);
}

void set_pos(GtkWidget * gw, int x, int y) {
	gtk_fixed_put(GTK_FIXED(frame), gw, x, y);
}

void make_about_dialog() {
	about_dialog = gtk_about_dialog_new();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), "Từ điển Anh-Việt");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), "1.3");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog), "2017 - 2710 BigTrym Team+");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog),
	                             "http://bigtrym.ml");
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(about_dialog), "http://bigtrym.ml");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog), "Chương trình từ điển sử dụng"
	                              " cấu trúc B-tree, bài tập sản phẩm"
	                              " cho bài tập môn Lập trình C nâng cao.");
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(about_dialog), NULL);

}
int search_b_tree(char * key, char* value) {
	int rsize = 0;
	beginTest("Tìm kiếm");
	btsel(data, key, value, 5000, &rsize);
	endTest();
	return rsize;
}
void set_textView_text(char * text) {
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
	if (buffer == NULL) {
		printf("Get buffer fail!");
		buffer = gtk_text_buffer_new(NULL);
	}
	gtk_text_buffer_set_text(buffer, text, -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(textView), buffer);
}
void find_in_dict(GtkWidget * widget, gpointer No_need) {
	int rsize;
	char key[50];
	char mean[5000];
	strcpy(key, gtk_entry_get_text(GTK_ENTRY(textSearch)));
	rsize = search_b_tree(key, mean);
	if (rsize == 0)
		set_textView_text("\nRất tiếc, từ này hiện không có trong từ điển."
		                  "\n\nGợi ý:\t-Nhấn tab để tìm từ gần giống từ vừa nhập!"
		                  "\n\t\t-Hoặc nhấn nút \"Thêm từ\", để bổ sung vào từ điển.");
	else
		set_textView_text(mean);
}
int prefix(const char * big, const char * small) {
	int small_len = strlen(small);
	int big_len = strlen(big);
	int i;
	if (big_len < small_len)
		return 0;
	for (i = 0; i < small_len; i++)
		if (big[i] != small[i])
			return 0;
	return 1;
}

int commond_char( char * str1, char * str2, int start) {
	int i;
	int slen1 = strlen(str1);
	int slen2 = strlen(str2);
	int slen  = (slen1 < slen2) ? slen1 : slen2;
	for ( i = start; i < slen; i++)
		if (str1[i] != str2[i])
			return i;
	return i;
}

void jrb_to_list(JRB nextWordArray, int number) {
	GtkTreeIter Iter;
	JRB tmp;
	int sochia = number / 9;
	int max = 8;
	if (sochia == 0) sochia = 1;
	jrb_traverse(tmp, nextWordArray) {
		if ((number--) % sochia == 0)  {
			gtk_list_store_append(list, &Iter);
			gtk_list_store_set(list, &Iter, 0, jval_s(tmp->key), -1 );
			if (max-- < 1)
				return;
		}
	}
}

int insert_insoundexlist(char * soundexlist , char * newword,  char * word, char * soundexWord) {
	if (strcmp(soundexWord, soundex(newword)) == 0) {
		if (strcmp(newword, word) != 0) {
			strcat(soundexlist, newword);
			strcat(soundexlist, "\n");
			return 1;
		}
	}
	else
		return 0;
}
void suggest(char * word, gboolean Tab_pressed) {
	beginTest("Gợi ý");
	char nextword[100], prevword[100];
	int i, NumOfCommondChar, minNumOfCommondChar = 1000;
	int max;
	GtkTreeIter Iter;
	JRB tmp, nextWordArray = make_jrb();
	BTint value, existed = 0;
	strcpy(nextword, word);
	int wordlen = strlen(word);
	gtk_list_store_clear(list);
	if (bfndky(data, word, &value) ==  0) {
		existed = 1;
		gtk_list_store_append(list, &Iter);
		gtk_list_store_set(list, &Iter, 0, nextword, -1 );
	}
	if (!existed)
		btins(data, nextword, "", 1);

	for (i = 0; i < 1945; i++) {
		bnxtky(data, nextword, &value);
		if (prefix(nextword, word)) {
			jrb_insert_str(nextWordArray, strdup(nextword), JNULL);
		}
		else break;
	}

	if (!existed && Tab_pressed) {
		if (jrb_empty(nextWordArray)) {
			char soundexlist[1000] = "Ý bạn là:\n";
			char soundexWord[50];
			strcpy(nextword, word);
			strcpy(prevword, word);
			strcpy(soundexWord, soundex(word));
			max = 5;
			for (i = 0; (i < 10000 ) && max; i++) {
				if (bprvky(data , prevword, &value) == 0)
					if (insert_insoundexlist(soundexlist, prevword, word, soundexWord))
						max--;
			}
			max = 5;
			for (i = 0; (i < 10000 ) && max; i++) {
				if (bnxtky(data, nextword, &value) == 0)
					if (insert_insoundexlist(soundexlist, nextword, word, soundexWord))
						max--;
			}
			set_textView_text(soundexlist);
		}
		else {
			strcpy(nextword, jval_s(jrb_first(nextWordArray)->key));
			jrb_traverse(tmp, nextWordArray) {
				NumOfCommondChar = commond_char(nextword, jval_s(tmp->key), wordlen);
				if (minNumOfCommondChar > NumOfCommondChar)
					minNumOfCommondChar = NumOfCommondChar;
			}

			if ((minNumOfCommondChar  != 1000) && (minNumOfCommondChar > wordlen)) {
				nextword[NumOfCommondChar] = '\0';
				gtk_entry_set_text(GTK_ENTRY(textSearch), nextword);
				gtk_editable_set_position(GTK_EDITABLE(textSearch), NumOfCommondChar);
			}
		}

	}
	else
		jrb_to_list(nextWordArray, i);
	if (!existed)
		btdel(data, word);
	jrb_free_tree(nextWordArray);
	endTest();
}


gboolean on_key_down(GtkWidget * entry, GdkEvent * event, gpointer No_need) {
	GdkEventKey *keyEvent = (GdkEventKey *)event;
	char word[50];
	int len;
	strcpy(word, gtk_entry_get_text(GTK_ENTRY(textSearch)));
	if (keyEvent->keyval == GDK_KEY_Tab) {
		suggest(word,  TRUE);
	}
	else {
		if (keyEvent->keyval != GDK_KEY_BackSpace) {
			len = strlen(word);
			word[len] = keyEvent->keyval;
			word[len + 1] = '\0';
		}
		else {
			len = strlen(word);
			word[len - 1] = '\0';
		}
		suggest(word, FALSE);
	}
	return FALSE;
}
void Show_about_dialog(GtkWidget * widget, gpointer dialog) {
	make_about_dialog();
	gtk_dialog_run(GTK_DIALOG(about_dialog));
	gtk_widget_destroy (about_dialog);
}
void Show_message(GtkWidget * parent , GtkMessageType type,  char * mms, char * content) {
	GtkWidget *mdialog;
	mdialog = gtk_message_dialog_new(GTK_WINDOW(parent),
	                                 GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 type,
	                                 GTK_BUTTONS_OK,
	                                 "%s", mms);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(mdialog), "%s",  content);
	gtk_dialog_run(GTK_DIALOG(mdialog));
	gtk_widget_destroy(mdialog);
}
void Add_word_to_dict(GtkWidget * widget, gpointer Array) {
	GtkWidget* inputtext = ((GtkWidget**)Array)[0];
	GtkWidget* mean = ((GtkWidget**)Array)[1];
	GtkTextIter st_iter;
	GtkTextIter ed_iter;
	BTint x;
	int result;
	gtk_text_buffer_get_start_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter);
	gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &ed_iter);
	gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter, &ed_iter, FALSE);
	char * wordtext = (char*)gtk_entry_get_text(GTK_ENTRY(inputtext));
	char * meantext =  gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter, &ed_iter, FALSE);
	if (wordtext[0] == '\0' || meantext[0] == '\0')
		Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_WARNING, "Cảnh báo!", "Không được bỏ trống phần nào.");
	else if (bfndky(data, wordtext, &x ) == 0)
		Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Từ vừa nhập đã có trong từ điển.");
	else
	{
		beginTest("Thêm từ");
		result = btins(data, wordtext, meantext, strlen(meantext) + 1);
		endTest();
		if ( result == 0)
			Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_INFO, "Thành công!", "Đã thêm từ vừa nhập vào từ điển.");
		else
			Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Có lỗi bất ngờ xảy ra.");
	}
}
void Edit_word_in_dict(GtkWidget * widget, gpointer Array) {
	GtkWidget* inputtext = ((GtkWidget**)Array)[0];
	GtkWidget* mean = ((GtkWidget**)Array)[1];
	GtkTextIter st_iter;
	GtkTextIter ed_iter;
	BTint x;
	int result;
	gtk_text_buffer_get_start_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter);//Lay chi so dau buffer
	gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &ed_iter);//Lay chi so cuoi buffer
	char * wordtext = (char*)gtk_entry_get_text(GTK_ENTRY(inputtext));//Lay noi dung o nhap tu tieng anh
	char * meantext =  gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter, &ed_iter, FALSE);// Lay toan bo text trong Textview(cho hienthi nghia)
	if (wordtext[0] == '\0' || meantext[0] == '\0') // Bo trong thi canh bao
		Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_WARNING, "Cảnh báo!",
		             "Không được bỏ trống phần nào.");
	else if (bfndky(data, wordtext, &x ) != 0)//Neu tim thay canh bao
		Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_ERROR, "Xảy ra lỗi!",
		             "Không tìm thấy từ này trong từ điển.");
	else {
		beginTest("Sửa từ");//Bat dau do
		result = btupd(data, wordtext, meantext, strlen(meantext) + 1);//Cap nhat la word va nghia
		endTest();//Ket thuc do
		if (result == 0)
			Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_INFO, "Thành công!",
			             "Đã cập nhật lại nghĩa của từ trong từ điển.");// Cap nhat thanh cong
		else//Cap nhat that bai
			Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Có lỗi bất ngờ xảy ra.");
	}
}
void Delete_word_from_dict(char * word) {
	beginTest("Xóa từ");
	int result = btdel(data, word);
	endTest();
	char anu[100] = "Đã xóa từ ";
	if (result == 0)
		Show_message(window, GTK_MESSAGE_INFO, "Thành công!", strcat(strcat(anu, word), " khỏi từ điển"));
	else
		Show_message(window, GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Có lỗi bất ngờ xảy ra.");
	set_textView_text("");
	gtk_entry_set_text(GTK_ENTRY(textSearch), "");
	gtk_widget_grab_focus(textSearch);
}
void destroy_something(GtkWidget * widget, gpointer gp) {
	gtk_widget_destroy(gp);
}
void Show_add_dialog(GtkWidget * widget, gpointer dialog) {
	GtkWidget *adddialog;
	adddialog = gtk_dialog_new_with_buttons("Thêm từ", GTK_WINDOW(window),
	                                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL, NULL);

	GtkWidget *dialog_ground = gtk_fixed_new();
	GtkWidget* tframe = gtk_frame_new("Từ vựng:");
	GtkWidget* bframe = gtk_frame_new("Nghĩa:");
	GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget * OkButton =  gtk_button_new_with_label("Thêm");
	GtkWidget * CancelButton = gtk_button_new_with_label("Hủy");
	GtkWidget* inputtext = gtk_entry_new();
	GtkWidget* mean = gtk_text_view_new();
	GtkWidget* scroll = gtk_scrolled_window_new(NULL, NULL);

	gtk_box_pack_start(GTK_BOX(box), OkButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(box), CancelButton, TRUE, TRUE, 2);
	set_size(tframe, 300, 50);
	set_size(bframe, 300, 200);
	set_size(box, 300, 50);
	set_size(OkButton, 100, 40);
	set_size(CancelButton, 100, 40);
	gtk_widget_set_margin_left(inputtext, 2);
	gtk_widget_set_margin_right(inputtext, 2);
	gtk_fixed_put(GTK_FIXED(dialog_ground), tframe, 0, 0);
	gtk_fixed_put(GTK_FIXED(dialog_ground), bframe, 0, 55);
	gtk_fixed_put(GTK_FIXED(dialog_ground), box, 0, 260);
	gtk_container_add(GTK_CONTAINER(tframe), inputtext);
	gtk_container_add(GTK_CONTAINER(scroll), mean);
	gtk_container_add(GTK_CONTAINER(bframe), scroll);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(mean), GTK_WRAP_WORD_CHAR);//Chong tran be ngang
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(adddialog))), dialog_ground,  TRUE, TRUE, 0);
	GtkWidget * data_array[3];
	data_array[0] = inputtext;
	data_array[1] = mean;
	data_array[2] = adddialog;
	g_signal_connect(OkButton, "clicked", G_CALLBACK(Add_word_to_dict), data_array);
	g_signal_connect(CancelButton, "clicked", G_CALLBACK(destroy_something), adddialog);
	gtk_widget_show_all(adddialog);
	gtk_dialog_run(GTK_DIALOG(adddialog));
	gtk_widget_destroy(adddialog);
}
void Show_edit_dialog(GtkWidget * widget, gpointer dialog) {
	BTint x;
	if (gtk_entry_get_text(GTK_ENTRY(textSearch))[0] == 0 ||
	        bfndky(data, (char*)gtk_entry_get_text(GTK_ENTRY(textSearch)), &x) != 0) {
		Show_message(window, GTK_MESSAGE_WARNING, "Cảnh báo:", "Từ vừa nhập không có trong từ điển!");
		return;
	}
	find_in_dict(NULL, NULL);
	GtkWidget *editdialog;
	editdialog = gtk_dialog_new_with_buttons("Sửa từ", GTK_WINDOW(window),
	             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL, NULL);

	GtkWidget *dialog_ground = gtk_fixed_new();
	GtkWidget* tframe = gtk_frame_new("Từ vựng:");
	GtkWidget* bframe = gtk_frame_new("Nghĩa:");
	GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget * OkButton =  gtk_button_new_with_label("Lưu");
	GtkWidget * CancelButton = gtk_button_new_with_label("Hủy");
	GtkWidget* inputtext = gtk_search_entry_new();
	GtkWidget* mean = gtk_text_view_new();
	GtkWidget* scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(box), OkButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(box), CancelButton, TRUE, TRUE, 2);
	set_size(tframe, 300, 50);
	set_size(bframe, 300, 200);
	set_size(box, 300, 50);
	set_size(OkButton, 100, 40);
	set_size(CancelButton, 100, 40);
	gtk_widget_set_margin_left(inputtext, 2);
	gtk_widget_set_margin_right(inputtext, 2);
	gtk_fixed_put(GTK_FIXED(dialog_ground), tframe, 0, 0);
	gtk_fixed_put(GTK_FIXED(dialog_ground), bframe, 0, 55);
	gtk_fixed_put(GTK_FIXED(dialog_ground), box, 0, 260);
	gtk_container_add(GTK_CONTAINER(tframe), inputtext);
	gtk_container_add(GTK_CONTAINER(scroll), mean);
	gtk_container_add(GTK_CONTAINER(bframe), scroll);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(mean), GTK_WRAP_WORD_CHAR);//Chong tran be ngang
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(editdialog))), dialog_ground,  TRUE, TRUE, 0);
	gtk_widget_set_sensitive(tframe, FALSE);
	gtk_entry_set_text(GTK_ENTRY(inputtext), gtk_entry_get_text(GTK_ENTRY(textSearch)));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(mean), gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView)));
	GtkWidget * data_array[3];
	data_array[0] = inputtext;
	data_array[1] = mean;
	data_array[2] = editdialog;
	g_signal_connect(OkButton, "clicked", G_CALLBACK(Edit_word_in_dict), data_array);
	g_signal_connect(CancelButton, "clicked", G_CALLBACK(destroy_something), editdialog);
	gtk_widget_show_all(editdialog);
	gtk_dialog_run(GTK_DIALOG(editdialog));
	gtk_widget_destroy(editdialog);
}

void Show_delete_dialog(GtkWidget * widget, gpointer dialog) {
	BTint x;
	if (gtk_entry_get_text(GTK_ENTRY(textSearch))[0] == 0 ||
	        bfndky(data, (char*)gtk_entry_get_text(GTK_ENTRY(textSearch)), &x) != 0) {
		Show_message(window, GTK_MESSAGE_WARNING, "Cảnh báo:", "Từ vừa nhập không có trong từ điển!");
		return;
	}
	GtkWidget *deldialog;
	deldialog = gtk_message_dialog_new(GTK_WINDOW(window),
	                                   GTK_DIALOG_DESTROY_WITH_PARENT,
	                                   GTK_MESSAGE_QUESTION,
	                                   GTK_BUTTONS_YES_NO,
	                                   "Xóa: \"%s\"?", gtk_entry_get_text(GTK_ENTRY(textSearch)));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(deldialog), "Bạn thực sự muốn xóa từ \"%s\" chứ?",
	        gtk_entry_get_text(GTK_ENTRY(textSearch)));

	int result = gtk_dialog_run(GTK_DIALOG(deldialog));
	if (result == GTK_RESPONSE_YES)
		Delete_word_from_dict((char*)gtk_entry_get_text(GTK_ENTRY(textSearch)));
	gtk_widget_destroy(deldialog);
}

int main(int argc, char** argv) {
	//Mo du lieu tu dien B-Tree
	data = btopn("AnhViet.dat", 0, 1);

	GtkWidget *groupBox;
	GtkWidget *inputBox;
	GtkWidget *outputBox;
	GtkWidget *buttonBox;

	GtkWidget *addButton;
	GtkWidget *delButton;
	GtkWidget *editButton;
	GtkWidget *infoButton;
	GtkWidget *searchButton;
	GtkWidget *scrolling;


	GtkEntryCompletion *comple;

	//Khoi dong Qua trinh test Hieu suat
	TEST_RESULT = make_jrb();
	// Khoi tao GTK
	gtk_init(&argc, &argv);
	init();//Khoi dong soundex

	// Khoi tao cua so
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	//gtk_window_set_default_size(GTK_WINDOW(window), -1, -1);
	gtk_window_set_title(GTK_WINDOW(window), "Từ điển Anh-Việt");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	//Khoi tao nen
	frame = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window), frame);
	gtk_widget_set_margin_bottom(frame, 5);
	gtk_widget_set_margin_right(frame, 5);
	//Khoi tao GroupBox
	groupBox = gtk_frame_new("Nhập từ:");
	set_size(groupBox, 560, 60);
	set_pos(groupBox, 10, 5);

	//Khoi tao hop chua cac nut
	buttonBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	set_size(buttonBox, 100, 250);
	set_pos(buttonBox, 10, 75);

	//khoi tao hop chua textView hien thi nghia
	outputBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	set_size(outputBox, 450, 250);
	set_pos(outputBox, 120, 75);

	//Khoi tao hop chua o tim kiem va nut tim kiem
	inputBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(GTK_CONTAINER(groupBox), inputBox);

	//Khoi tao cac nut chuc nang
	editButton = gtk_button_new_with_label("Sửa từ");
	delButton = gtk_button_new_with_label("Xóa từ");
	addButton = gtk_button_new_with_label("Thêm từ");
	infoButton = gtk_button_new_with_label("Thông tin");

	searchButton = gtk_button_new_with_label("Tìm kiếm");
	gtk_widget_set_margin_bottom(searchButton, 10);
	gtk_widget_set_margin_top(searchButton, 10);

	//Khoi tao o nhap va textView hien thi nghia
	textSearch = gtk_search_entry_new();
	set_size(textSearch, 450, 20);
	//
	textView = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textView), GTK_WRAP_WORD_CHAR);//Chong tran be ngang

	//Khoi tao thanh keo truot cho textView
	scrolling = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolling), textView);


	comple = gtk_entry_completion_new();
	gtk_entry_completion_set_text_column(comple, 0);
	list = gtk_list_store_new(10, G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING);

	gtk_entry_completion_set_model(comple, GTK_TREE_MODEL(list));
	gtk_entry_set_completion(GTK_ENTRY(textSearch), comple);
	//Dat cac thanh phan vao hop tuong ung
	gtk_box_pack_start(GTK_BOX(buttonBox), addButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(buttonBox), editButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(buttonBox), delButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(buttonBox), infoButton, TRUE, TRUE, 2);

	gtk_box_pack_start(GTK_BOX(inputBox), textSearch, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(inputBox), searchButton, TRUE, TRUE, 5);

	gtk_box_pack_start(GTK_BOX(outputBox), scrolling, TRUE, TRUE, 2);

	gtk_widget_show_all(window);


	g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);//Ket thuc chuong trinh khi dong cua so chinh

	//Gan su kien cho cac nut
	g_signal_connect(searchButton, "clicked", G_CALLBACK(find_in_dict), NULL);
	g_signal_connect(addButton, "clicked", G_CALLBACK(Show_add_dialog), NULL);
	g_signal_connect(editButton, "clicked", G_CALLBACK(Show_edit_dialog), NULL);
	g_signal_connect(delButton, "clicked", G_CALLBACK(Show_delete_dialog), NULL);
	g_signal_connect(infoButton, "clicked", G_CALLBACK(Show_about_dialog), about_dialog);
	g_signal_connect(textSearch, "key-press-event", G_CALLBACK(on_key_down), NULL);
	g_signal_connect(textSearch, "activate", G_CALLBACK(find_in_dict), NULL);

	gtk_main();

	// Dong du lieu B-Tree
	thonKeHieuSuatTrungBinh();
	btcls(data);
	return 0;
}