#include<stdio.h>
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>

struct termios orig_termios;

void disableraw(){
    tcsetattr(STDIN_FILENO,TCSAFLUSH, &orig_termios);
}

void enableraw(){
    
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableraw);

    struct termios raw=orig_termios;
    raw.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw);

}


int main(){
    enableraw();
    char c;
    while(read(STDIN_FILENO,&c,1)==1 && c!='q');

    return 0;
}