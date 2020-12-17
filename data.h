#pragma once    // Tranh include lai thu vien nhieu lan


typedef struct User{
    char username[32];  // Name of user
    char * stream;      // Message send
    int newMessage;
}User;