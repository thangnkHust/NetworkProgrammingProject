#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netdb.h>
#include "integer-constant.h"
#include "string-constant.h"
#include "data.h"


int client_sock_fd;
// wait = 9;
char *inBuf;
char outBuf[4096];
GMutex queueMutex;
char *bufferQueue[USER_NUM_MAX];
int queueSize = 0;
extern char onlineUsers[USER_NUM_MAX][32];
extern int onlineUserCount;
extern char *publicStream;
extern GtkWidget *chatArea;
extern void updateUserList(char n[][32], int);
extern void textViewSetText(GtkWidget *, char *);
extern char *currentChannel;
extern User onlineUsersStream[USER_NUM_MAX];
extern char * you;
extern void *showBubbleNotify(void *);
char *push(char *str)
{
    g_mutex_lock(&queueMutex);
    bufferQueue[queueSize] = strdup(str);
    puts("[PUSH]<===============================");
    puts(str);
    puts("<=====================================");
    queueSize++;
    g_mutex_unlock(&queueMutex);
    return bufferQueue[queueSize];
}

char * getStream(char *channel){
    int i = 0;
    for(;i < USER_NUM_MAX; i++){
        if(strcmp(channel, onlineUsersStream[i].username) == 0){
            return onlineUsersStream[i].stream;
        }
    }
    return NULL;
}
int pop(char *str)
{
    g_mutex_lock(&queueMutex);
    int current = queueSize;
    if (queueSize > 0)
    {
        queueSize--;
        strcpy(str, bufferQueue[queueSize]);
        puts("[POP]================================>");
        puts(str);
        puts("=====================================>");
        free(bufferQueue[queueSize]);
    }
    g_mutex_unlock(&queueMutex);
    return current;
}
void clearBuf(char *buff)
{
    memset(buff, 0, strlen(buff));
}
int sendRequest()
{
    int n = strlen(inBuf);
    puts("\t|");
    printf("\t+--->Send to server :{%s}\n", inBuf);
    printf("\n--------------->Sent %d bytes:<-----------------\n\n", n);
    return send(client_sock_fd, inBuf, n, 0);
}
char *splitMessage(char *message)
{
    char *split = strchr(message, '#');
    *split = '\0';
    return split + 1;
}
int handleLoginResponse(char *_message)
{
    char *value = _message;
    _message = splitMessage(_message);
    printf("value:%s\nmessage:%s\n", value, _message);
    if (strcmp(value, SUCCESS) == 0)
    {
        if (strcmp(_message, OK) == 0) // username Sent
        {
            onSentUsername();
        }
        else //password sent
        {
            onLoginSuccess(_message);
        }
    }
    else
    {
        onLoginFailed(_message);
    }
    return 0;
}

char * saveToUserMessageStream(char * sender, char * message){
    int i, found = 0;
    char temp[MAXLINE];
    for (i = 0; i< USER_NUM_MAX; i++){
        if(strcmp(onlineUsersStream[i].username, sender) == 0){ // user found
            printf("Find user at %d\n", i);
            printf("Current stream: %s\n", onlineUsersStream[i].stream);
            found = 1;
        }else if(onlineUsersStream[i].username[0] == '\0'){ //user not found, create new
            printf("User not found created at %d\n", i);
            strcpy(onlineUsersStream[i].username, sender);
            onlineUsersStream[i].stream = (char *)calloc(69 * MAXLINE , sizeof(char));
            found = 1;
        }
        if (found)
        {
            sprintf(temp, "%s\n", message);
            strcat(onlineUsersStream[i].stream, temp);
            return onlineUsersStream[i].stream;
        }
    }
    return sender;
}

int findUserMessageStream(char * sender){
    int i, found = 0;
    for (i = 0; i< USER_NUM_MAX; i++){
        if(strcmp(onlineUsersStream[i].username, sender) == 0){ // user found
            printf("Find user at %d\n", i);
            printf("Current stream: %s\n", onlineUsersStream[i].stream);
            found = 1;
        }else if(onlineUsersStream[i].username[0] == '\0'){ //user not found, create new
            printf("User not found created at %d\n", i);
            strcpy(onlineUsersStream[i].username, sender);
            onlineUsersStream[i].stream = (char *)calloc(69 * MAXLINE, sizeof (char));
            found = 1;
        }

        if (found)
        {
            return i;
        }
    }
    return -1;
}

int notifyMessageCount(char * sender){
    int i, count = 0;
    char name[32];
        
    for(i = 0; i< onlineUserCount; i++){
        count = 0;
        sscanf(onlineUsers[i], "%[^(](%d", name, &count);
        if (strcmp(name, sender) == 0)
        {
            sprintf(onlineUsers[i], "%s(%d)", name, count+1);
            puts(onlineUsers[i]);
            int id = findUserMessageStream(name);
            if (id != -1)
            {
                onlineUsersStream[i].newMessage = count+1;
            }
            break;
        }
    }
    updateUserList(onlineUsers, onlineUserCount);
}

int handlePrivateMessage(char *message)
{
    char *sender = message;
    message = splitMessage(message);
    char temp[MAXLINE];
    sprintf(temp, "%s:%s", sender, message);
    char * xstream =  saveToUserMessageStream(sender, temp);
    if (strcmp(currentChannel, sender) == 0)
    {
        textViewSetText(chatArea, xstream);
    }else{
        notifyMessageCount(sender);
    }
    return 0;
}


int handlePublicMessage(char *message)
{
    char *sender = message;
    message = splitMessage(message);
    char temp[MAXLINE];
    sprintf(temp, "%s:%s\n", sender, message);
    strcat(publicStream, temp);
    if (strcmp(currentChannel, PUBLIC) == 0)
    {
        textViewSetText(chatArea, publicStream);
    }
    sprintf(temp, "@%s", you);
    if (strstr(message, temp))
    {
        onUserTagged(sender);
    }
    return 0;
}
int handleOnlineUsersList(char *message)
{
    int i, j = 0;
    int messageLength = strlen(message);
    onlineUserCount = 0;
    for (i = 0; i < messageLength; i++)
    {
        if (message[i] == SEPARATOR)
        {
            message[i] = '\0';
            strcpy(onlineUsers[onlineUserCount], message + j);
            int id = findUserMessageStream(onlineUsers[onlineUserCount]);
            if (id != -1)
            {
                if (onlineUsersStream[id].newMessage)
                {
                    char temp[10];
                    sprintf(temp, "(%d)", onlineUsersStream[id].newMessage);
                    strcat(onlineUsers[onlineUserCount], temp);
                }
            }
            j = i + 1;
            onlineUserCount++;
        }
    }
    updateUserList(onlineUsers, onlineUserCount);
    return messageLength;
}

void handleGetStream(char *message)
{
    strcpy(publicStream, message);
    strcat(publicStream, "\n");
    textViewSetText(chatArea, publicStream);
}
void handleReponse(char *buff, int n)
{
    buff[n] = '\0';
    // printf("Received form server:\"{%s}{length:%d}\"\n", buff, n);
    printf("\n------------->Received %d bytes:<--------------\n\n", n);
    char action, *message;
    if (buff[strlen(buff) - 1] == '\n')
        buff[strlen(buff) - 1] = '\0';
    action = buff[0];
    message = buff + 1;
    puts(buff); //
    switch (action)
    {
    case LOGIN_RESPONSE_ACTION:
        handleLoginResponse(message);
        break;
    case GET_LIST_USER_ACTION:
        handleOnlineUsersList(message);
        break;
    case PRIVATE_MESSAGE_ACTION:
        handlePrivateMessage(message);
        break;
    case CHANNEL_MESSAGE_ACTION:
        handlePublicMessage(message);
        break;
    case GET_PUBLIC_STREAM:
        printf(">><<\n", strlen(message));
        if (strlen(message) > 1)
        {
            handleGetStream(message);
        }
        break;
    }
}
void signio_handler(int signo)
{
    char buffer[MAXLINE];
    clearBuf(buffer);
    char * ackIndex, * bufferStart;
    int n = recv(client_sock_fd, buffer, MAXLINE, 0);
    if (n > 0)
    {
        // printf("***SIGNAL IO\n");
        bufferStart = buffer;
        char * end = bufferStart + strlen(bufferStart);
        do{
            ackIndex = strstr(bufferStart, ACK);
            if (ackIndex != NULL)
            {   
                *ackIndex = '\0';
                push(bufferStart);
                bufferStart = ackIndex+1;
            }
        }while(ackIndex != NULL && bufferStart < end);
    }
    else
    {
        puts("Error IO");
    }
}

gboolean timer_exe(gpointer p)
{
    // printf("***Timeout in\n");
    char newBuffer[MAXLINE];
    if (pop(newBuffer) > 0)
        handleReponse(newBuffer, strlen(newBuffer));
    return TRUE;
}
void signal_SIGABRT(int signal)
{
    printf("SIGABRT\n");
}
int createClient(int argc, char *argv[])
{
    // inBuf is message send with client and server
    inBuf = (char *)malloc(MAXLINE * sizeof(char));
    // Step 1: Construct socket
    struct sockaddr_in server_socket;
    client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_socket.sin_family = AF_INET;
    server_socket.sin_port = htons(SERV_PORT);
    server_socket.sin_addr.s_addr = inet_addr(argc > 1?argv[1]:"127.0.0.1");
    printf("server IP = %s ", inet_ntoa(server_socket.sin_addr));

    // Step3: Request to connect server
    if (connect(client_sock_fd, (struct sockaddr *)&server_socket, sizeof(server_socket)) < 0){
        char * errorMessage = "Error in connecting to server\n";
        printf(errorMessage);
        showBubbleNotify(errorMessage);
    }
    else
        printf("connected to server\n");

    // Signal driven I/O mode and NONBlOCK mode so that recv will not block
    if (fcntl(client_sock_fd, F_SETFL, O_NONBLOCK | O_ASYNC))
        // if (fcntl(client_sock_fd, F_SETFL, O_NONBLOCK))
        printf("Error in setting socket to async, nonblock mode");

    signal(SIGIO, signio_handler); // assign SIGIO to the handler
    signal(SIGABRT, signal_SIGABRT);
    // set this process to be the process owner for SIGIO signal
    if (fcntl(client_sock_fd, F_SETOWN, getpid()) < 0)
        printf("Error in setting own to socket");
    return 0;
}