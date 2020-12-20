#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
// #include <crypt.h>
#include <unistd.h>
#include "integer-constant.h"
#include "string-constant.h"

char buf[MAXLINE];
char *publicStream;
typedef struct $
{
    char username[32];
    char password[100];
    int fd;
} User;

User users[USER_NUM_MAX];
int auth[USER_NUM_MAX];

User userRegister;  // temp user save to File
int countUserOfFile;

int sendResponse(int connfd)
{
    if (strstr(buf, ACK) == NULL)
    {
        strcat(buf, ACK);
    }
    printf("Send to client socket %d:[%s]\n",connfd, buf);
    return send(connfd, buf, strlen(buf), 0);
}
int broadCast()
{
    int i;
    for (i = 0; i < USER_NUM_MAX; i++)
    {
        if (users[i].fd != NONE_SOCKET)
        {
            sendResponse(users[i].fd);
        }
    }
    return i;
}
int cleanBuffer()
{
    memset(buf, 0, MAXLINE);
    return 0;
}
int checkUser(char *name)
{
    int i;
    for (i = 0; i < USER_NUM_MAX; i++)
    {
        if (strcmp(name, users[i].username) == 0)
        {
            return i;
        }
    }
    return NONE_USER;
}
int getOnlineUsers(char *list)
{
    int i;
    printf("\nOnline user list: [");
    for (i = 0; i < USER_NUM_MAX; ++i)
    {
        if (users[i].fd != NONE_SOCKET)
        {
            printf("%s,", users[i].username);
            strcat(list, users[i].username);
            list[strlen(list)] = SEPARATOR;
        }
    }
    puts("]");
    return i;
}

int getOnlineUsersMessage()
{
    buf[0] = GET_LIST_USER_ACTION;
    return getOnlineUsers(buf + 1);
}
int handleUserGetOnlineMember(int connfd)
{
    int n = getOnlineUsersMessage();
    sendResponse(connfd);
    return n;
}

int broadcastOnlineUsers()
{
    cleanBuffer();
    int n = getOnlineUsersMessage();
    broadCast();
    return n;
}
int markUserNameInputed(int fdnum, int userId)
{
    auth[fdnum] = -1 - userId;
    return auth[fdnum];
}

int markUserLogged(int fdnum, int userId)
{
    auth[fdnum] = userId;
    return userId;
}

int validatePassword(int fdnum, char *password)
{
    char *encrypt = crypt(password, "salt");
    if (auth[fdnum] < 0)
    {
        User u = users[-1 - auth[fdnum]];
        printf("\n{\n  username:%s,\n  password:%s,\n  input_password:%s,\n  socket:%d\n}\n", u.username, u.password, password, u.fd);
        if (strcmp(u.password, encrypt) == 0)
        {
            if (u.fd != 0)
                return CODE_LOGGED_BY_ANOTHER;
            printf("%d", -1 - auth[fdnum]);
            return -1 - auth[fdnum];
        }
    }
    // free(encrypt);
    return CODE_PASSWORD_INCORRECT;
}
int initAuth()
{
    int i;
    for (i = 0; i < USER_NUM_MAX; i++)
    {
        auth[i] = -1;
    }
    return i;
}
int initUserList()
{
    memset(users, 0, sizeof users);
    printf("Reset user list on %ld bytes\n", sizeof users);
    return sizeof users;
}
int loadUserList(const char *source)
{
    FILE *f = fopen(source, "r+");
    char temp[200];
    int i;
    if (f == NULL)
    {
        puts("Read user list error");
        exit(-1);
    }
    for (i = 0; fgets(temp, 200, f) != NULL; i++)
    {
        sscanf(temp, "%s %s", users[i].username, users[i].password);
        // printf("User:{\n  username:\"%s\"\n  password:\"%s\"\n}\n", users[i].username, users[i].password);
    }
    fclose(f);
    return i;
}

void handleUserLogout(int connfd)
{
    int userId = auth[connfd];
    printf("\n{\n  username:%s,\n  status:logout\n}\n", users[auth[connfd]].username);
    users[userId].fd = NONE_SOCKET;
    auth[connfd] = NONE_USER;
    broadcastOnlineUsers();
}

int handleUserSendUsername(char *message, int connfd)
{
    int userId = checkUser(message);
    if (userId == NONE_USER)
    {
        sprintf(buf, "%c%s#%s", LOGIN_RESPONSE_ACTION, FAILED, USER_INVALID);
    }
    else
    {
        markUserNameInputed(connfd, userId);
        sprintf(buf, "%c%s#%s", LOGIN_RESPONSE_ACTION, SUCCESS, OK);
    }
    sendResponse(connfd);
    return userId;
}

int handleUserSendUsernameRegister(char *message, int connfd){
    int userId = checkUser(message);
    if (userId == NONE_USER)
    {
        sprintf(buf, "%c%s#%s", REGISTER_RESPONSE_ACTION, SUCCESS, OK);
        strcpy(userRegister.username, message);
        printf("****%s\n", userRegister.username);
    }
    else
    {
        sprintf(buf, "%c%s#%s", REGISTER_RESPONSE_ACTION, FAILED, REGISTER_FAILED);
    }
    sendResponse(connfd);
    return userId;
}

int handleUserSendPassword(char *message, int connfd)
{
    int userId = validatePassword(connfd, message);
    if (userId == CODE_PASSWORD_INCORRECT) // password incorrect
    {
        sprintf(buf, "%c%s#%s", LOGIN_RESPONSE_ACTION, FAILED, ACCOUNT_INVALID);
    }
    else
    {
        // password correct
        // user login
        if (userId == CODE_LOGGED_BY_ANOTHER) // Account are Already logged by other client
        {
            cleanBuffer();
            sprintf(buf, "%c%s#%s", LOGIN_RESPONSE_ACTION, FAILED, SESSION_INVALID);
        }
        else // Account logged normaly
        {
            sprintf(buf, "%c%s#%s", LOGIN_RESPONSE_ACTION, SUCCESS, WELLCOME);
            auth[connfd] = userId; // update auth, this connfd -> user
            users[userId].fd = connfd;
            printf("\n{\n  username:%s,\n  status:logged\n}\n", users[auth[connfd]].username);
        }
    }
    sendResponse(connfd);
    return userId;
}

int handleUserSendPasswordRegister(char *message){
    strcpy(userRegister.password, crypt(message, "salt"));
    strcpy(users[countUserOfFile].username, userRegister.username);
    strcpy(users[countUserOfFile].password, userRegister.password);
    FILE *f = fopen("users.txt", "w+");
    // printf("%s %s \n", users[0].username, users[0].password);
    for(int i = 0; i <= countUserOfFile; i++){
        fprintf(f, "%s\t%s%\n", users[i].username, users[i].password);
    }
    countUserOfFile++;
    fclose(f);
}

void handlePublicMessage(int connfd, char *message)
{
    char temp[MAXLINE];
    if (connfd == -1)// Server send
    {
        sprintf(temp, "%s:%s\n", ">", message);
        sprintf(buf, "%c%s#%s", CHANNEL_MESSAGE_ACTION, ">", message);
    }
    else //  Forward user's messsage
    {
        sprintf(temp, "%s:%s\n", users[auth[connfd]].username , message);

        sprintf(buf, "%c%s#%s", CHANNEL_MESSAGE_ACTION, users[auth[connfd]].username, message);
    }
    strcat(publicStream, temp);
    broadCast();
}

void handleGetStream(int connfd)
{
    sprintf(buf, "%c%s", GET_PUBLIC_STREAM, publicStream);
    sendResponse(connfd);
}
void handlePrivateMessage(int connfd, char *message)
{
    char *sender = users[auth[connfd]].username;
    char *split = strchr(message, '#');
    *split = '\0';
    char *m_username = message;
    message = split + 1;
    int userId = checkUser(m_username);
    if (userId != NONE_USER)
    {
        sprintf(buf, "%c%s#%s", PRIVATE_MESSAGE_ACTION, sender, message);
        sendResponse(users[userId].fd);
    }
    else
    {
        printf("Error: No user:{%s}\n", m_username);
    }
}
void *myThreadFun(void *vargp)
{
    sleep(1);
    char temp[200];
    cleanBuffer();
    sprintf(temp, "%s đã tham gia phòng chat!", (char*)vargp);
    handlePublicMessage(-1, temp);
    return NULL;
}
int handleMessage(int connfd)
{
    int n;  // return status of handle message
    char action, *message;  // action: find type of message
    char readBuf[MAXLINE];
    if ((n = recv(connfd, readBuf, MAXLINE, 0)) > 0)
    {
        readBuf[n] = '\0';
        if (readBuf[strlen(readBuf) - 1] == '\n')      // handle message
            readBuf[strlen(readBuf) - 1] = 0;
        printf("String received from client: %s\n", readBuf);
        action = readBuf[0];
        message = readBuf + 1;
        printf("\n{\n\taction:%c,\n\tmessage:%s\n}\n", action, message);
        cleanBuffer();
        switch (action)
        {
        case SEND_USER_ACTION:
            handleUserSendUsername(message, connfd);
            break;
        case SEND_USER_REGISTER_ACTION:
            handleUserSendUsernameRegister(message, connfd);
            break;
        case SEND_PASSWORD_ACTION:
            if (handleUserSendPassword(message, connfd) != CODE_PASSWORD_INCORRECT)
            {
                broadcastOnlineUsers();
                pthread_t thread_id;
                printf("Before Thread\n");
                pthread_create(&thread_id, NULL, myThreadFun, users[auth[connfd]].username);
            }
            break;
        case SEND_PASSWORD_REGISTER_ACTION:
            handleUserSendPasswordRegister(message);
            break;
        case LOGOUT_ACTION:
            handleUserLogout(connfd);
            break;
        case GET_LIST_USER_ACTION:
            handleUserGetOnlineMember(connfd);
            break;
        case CHANNEL_MESSAGE_ACTION:
            handlePublicMessage(connfd, message);
            break;
        case PRIVATE_MESSAGE_ACTION:
            handlePrivateMessage(connfd, message);
            break;
        case GET_PUBLIC_STREAM:
            handleGetStream(connfd);
            break;
        default:
            puts("\n--------------------UNKNOW ACTION-----------------");
            printf("Send to socket:%d\n", connfd);
        }
    }
    return n;
}

int handleNewConnection(int connfd)
{
    return 0;
}
int createServer()
{
    int maxfd, listenfd, connfd, i, rv;     // rv = recvBytes
    socklen_t clilen;
    fd_set readfds, master;
    struct sockaddr_in cliaddr, servaddr;

    // Step 1: creation of the socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0); 

    // preparation of the socket address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    // Step 2: Bind address to socket
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) >= 0)
    {
        puts("Server address is one of:");
        system("ifconfig | perl -nle'/dr:(\\S+)/ && print $1'");
        printf("Server is running at port %d\n", SERV_PORT);
    }
    else
    {
        perror("bind failed");
        return 0;
    }

    // Step 3: Listen request from client
    listen(listenfd, USER_NUM_MAX);

    printf("%s\n", "Server running...waiting for connections.");
    // Step 4: Assign initial value for the fd_set
    maxfd = listenfd;
    FD_ZERO(&master);  //FD_ZERO works like memset 0;
    FD_ZERO(&readfds); //clear the master and temp sets
    FD_SET(maxfd, &master);

    // fprintf(stdout, "Current maxfd: %d\n", maxfd);
    // fflush(stdout);
    printf("Current maxfd: %d\n", maxfd);
    while (1)
    {
        readfds = master;
        rv = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (rv == -1)
        {
            puts("error");
        }
        for (i = 0; i <= maxfd; i++)
        {
            // printf("***i = %d-listenfd = %d\n", i, listenfd);
            if (FD_ISSET(i, &readfds))
            {
                if (i == listenfd)      // New connection on socket
                {
                    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
                    if (connfd == -1)
                    {
                        perror("Error Accept");
                    }
                    else
                    {
                        if (connfd > maxfd)
                        { // keep track of the max
                            maxfd = connfd;
                        }
                        FD_SET(connfd, &master);
                        printf("New connection on socket %d\n", connfd);
                        handleNewConnection(connfd);
                    }
                }
                else    // Send message
                {
                    // printf("**handle mesage %d", i);
                    int r = handleMessage(i);
                    if (r <= 0)
                    { // got error or connection closed by client
                        if (r == 0)
                        { // connection closed
                            printf("Socket %d hung up\n", i);
                        }
                        else
                        {
                            perror("recv");
                        }
                        if (auth[i] != -1)
                            handleUserLogout(i);
                        close(i);           // bye!
                        FD_CLR(i, &master); // remove from master set
                        auth[i] = -1;
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    publicStream = (char *)malloc(1024 * MAXLINE);
    countUserOfFile = loadUserList("users.txt");
    initAuth();
    createServer();
}