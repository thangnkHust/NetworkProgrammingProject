# <img src="https://cdn.dribbble.com/users/4930498/screenshots/10585845/chatroom.jpg" width="100" />Chat application via `TCP`

***
# Install
## 1. Install gtk 3:
```
sudo apt-get install libgtk-3-dev
```
## 2. Complie:  
- ### 2.1. In MacOS  

    ```
    make
    ```
    or complile with all warning
    ```
    make w
    ```
- ### 2.2. In Ubuntu

    In file server.c : add command   
    ``` 
    #include <crypt.h> 
    ```

    In makefile : edit main to 
    ``` 
    main: clean
        clear
        gcc -w -g -o client client-chat.c -lgthread-2.0 `pkg-config gtk+-3.0 --cflags --libs`
        gcc -o server server.c -g -w -pthread -lcrypt
    ```

## 3. Run server:
```
make server
```
or 
```
make s
```
## 4. Run client
```
make run
```
or for debug by gdb
```
make debug
```
## 5. Clear output file:
```
make clean
```
----
