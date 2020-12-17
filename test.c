#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <signal.h> 
void sighandler(int); 
int main() { 
  signal(SIGINT, sighandler); 
  while(1) { 
    printf("Chuan bi sleep trong mot vai giay ...\n"); 
    sleep(1); 
  } 
  return(0); 
} 
void sighandler(int signum) { 
  printf("Bat duoc tin hieu %d, chuan bi thoat ...\n", signum); 
  exit(1);
}