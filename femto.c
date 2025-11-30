/*** includes ***/
#include<ctype.h>
#include<stdio.h>
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/ioctl.h>

/*** defining ***/
#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/
//to make it global state
struct editor_config{
    int screenrows;
    int screencols;
    struct termios orig_termios;
};
struct editor_config E;


/*** terminal ***/
void die(const char *s){
    write(STDOUT_FILENO,"\x1b[2J",4);
    write(STDOUT_FILENO,"\x1b[H",3);
    perror(s);
    exit(1);
}

void disableraw(){
    if (tcsetattr(STDIN_FILENO,TCSAFLUSH, &E.orig_termios)==-1 ){
        die("tcsetattr");
    }
    
}

void enableraw(){
    
    if (tcgetattr(STDIN_FILENO, &E.orig_termios)==-1){
        die("tcgetattr");
    }
    atexit(disableraw);

    struct termios raw=E.orig_termios;
    raw.c_iflag &= ~(BRKINT|INPCK|ISTRIP|ICRNL| IXON);
    raw.c_lflag &= ~(ECHO |ICANON | ISIG |IEXTEN) ; 
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
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

int get_cursor_pos(int *rows,int *cols){

    char buf[32];
    unsigned int i =0;

    if(write(STDOUT_FILENO,"\x1b[6n",4) !=4) return -1;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';
    printf("\r\n&buf[1]: '%s'\r\n", &buf[1]);
    editor_readkey();
    return -1;

}

/*** input ***/
void editor_process_keypress(){
    char c = editor_readkey();
    //quits when ctrl-q is entered
    switch (c) {
        case CTRL_KEY('a'):// for now using ctrl-a , will change by final
        write(STDOUT_FILENO,"\x1b[2J",4);
        write(STDOUT_FILENO,"\x1b[H",3);
        exit(0);
        break;
    }
}

//to get the window size of terminal
int get_window_size(int *rows, int *cols){
    struct winsize ws;
    if(ioctl(STDOUT_FILENO,TIOCGWINSZ, &ws)==-1 || ws.ws_col==0){
        if(write(STDOUT_FILENO,"\x1b[999C\x1b[999B",12) !=12) return -1;// this does same as getting windows size like ioct, but this is used when ioct does not work
        return get_cursor_pos(rows,cols);
        return -1;
    }
    else{
        *rows=ws.ws_row;
        *cols=ws.ws_col;
        return 0;

    }

}

/*** output ***/

void draw_tildes(){
    for (int y=0;y<E.screenrows;y++){
        write(STDOUT_FILENO,"~",1);
        if(y>E.screenrows-1){
            write(STDOUT_FILENO,"\r\n",2);
            
        }
    }

}

void editor_screen_refresh(){
    write(STDOUT_FILENO,"\x1b[2J",4);
    //uses J command
    // this function basically clears the screen by using the escape sequence and writing into 4 bytes

    write(STDOUT_FILENO,"\x1b[H",3);
    //this reposistions the cursor using the H command

    //drawing the tildes
    draw_tildes();
    write(STDOUT_FILENO,"\x1b[H",3);



}



/*** init ***/

void init_editor(){
    if(get_window_size(&E.screenrows, &E.screencols)==-1){
        die("get_window_size");
    }
}
int main(){
    enableraw();
    init_editor();
    while (1) {
    editor_process_keypress();
    editor_screen_refresh();
    }
        
    return 0;
}