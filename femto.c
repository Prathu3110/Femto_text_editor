#include<ctype.h>
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
    raw.c_iflag &= ~(ICRNL| IXON);
    raw.c_lflag &= ~(ECHO |ICANON | ISIG |IEXTEN) ; 
    
    //ICANON- is used for canonical mode (which means input is read line by line) 
    //ISIG is used for operations such INTEREPT SUSPEND TERMINATE (like ctrl-c, ctrl-z)
    //IXON here is used for ctrl-s and ctrl-q which pauses and unpauses
    //IEXTEN is for ctrl-v , and when you press ctrl-v before the next chartacter is taken as literal input , but now its is disabled
    //ICRNL this here is used for disabling ctrl-m, stands for (CR- carriage return and NL= new line) this is used becasue ctrl-m inputs as 10 bytes while actually it should give 13 bytes

    tcsetattr(STDIN_FILENO,TCSAFLUSH, &raw);

}


int main(){
    enableraw();
    char c;
    while(read(STDIN_FILENO,&c,1)==1 && c!='q'); //quits when q is entered
    if(iscntrl(c)){
        printf("%d \n", c);

    }
    else{
        printf("%d ('%c')\n",c,c);
    }
    return 0;
}