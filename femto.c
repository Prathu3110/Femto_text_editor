/*** includes ***/
#include<ctype.h>
#include<stdio.h>
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<errno.h>

/*** defining ***/
#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/
struct termios orig_termios;

/*** terminal ***/
void die(char *s){
    perror(s);
    exit(1);
}

void disableraw(){
    if (tcsetattr(STDIN_FILENO,TCSAFLUSH, &orig_termios)==-1 ){
        die("tcsetattr");
    }
    
}

void enableraw(){
    
    if (tcgetattr(STDIN_FILENO, &orig_termios)==-1){
        die("tgetattr");
    }
    atexit(disableraw);

    struct termios raw=orig_termios;
    raw.c_iflag &= ~(BRKINT|INPCK|ISTRIP|ICRNL| IXON);
    raw.c_lflag &= ~(ECHO |ICANON | ISIG |IEXTEN) ; 
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= ~(CS8);
    raw.c_cc[VMIN]=0;
    raw.c_cc[VTIME]=1;

    //for c_cflag CS8 we use | instead of & because CS8 is a bit mask and not a flag
    //ICANON- is used for canonical mode (which means input is read line by line) 
    //ISIG is used for operations such INTEREPT SUSPEND TERMINATE (like ctrl-c, ctrl-z)
    //IXON here is used for ctrl-s and ctrl-q which pauses and unpauses
    //IEXTEN is for ctrl-v , and when you press ctrl-v before the next chartacter is taken as literal input , but now its is disabled
    //ICRNL this here is used for disabling ctrl-m, stands for (CR- carriage return and NL= new line) this is used becasue ctrl-m inputs as 10 bytes while actually it should give 13 bytes

    if (tcsetattr(STDIN_FILENO,TCSAFLUSH, &raw) ==-1){
        die("tcsetattr");
    }

}

char editor_readkey(){
    int nread;
    char c;
    // Since we are using OPOST , we need to use \r\n for next line escape sequence instead of just\n
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

/*** input ***/
void editor_process_keypress(){
    char c = editor_readkey();
    //quits when ctrl-q is entered
    switch (c) {
        case CTRL_KEY('q'):
        exit(0);
        break;
    }
}

/*** output ***/

void editor_screen_refresh(){
    write(STDOUT_FILENO,"\x1b[2J",4);
    //uses J command
    // this function basically clears the screen by using the escape sequence and writing into 4 bytes

    write(STDOUT_FILENO,"\x1b[H",3);
    //this reposistions the cursor using the H command

    
}


/*** init ***/

int main(){
    enableraw();
    
    while (1) {
    editor_process_keypress();
    editor_screen_refresh();
    }
        
    return 0;
}