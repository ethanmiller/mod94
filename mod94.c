#include <ncurses.h>
#include <sys/time.h>
#include <signal.h>

#define WINDIV 30
#define INTRVL 41666 // approx 24 times a second

void draws(int);
int handle_char(char in_char);

int main(){
    char ch;
    setup();
    while(1){
	ch = getch();
	handle_char(ch);
    }
    endwin();
    return 0;
    // for scrolling see http://www.mkssoftware.com/docs/man3/curs_scroll.3.asp
}

int setup(){
    signal(SIGALRM, draws);
    start_anim();
    initscr(); // Start curses mode
    cbreak();  // Line buffering disabled
    noecho();  // Don't echo() while we do getch
    return 0;
}

int handle_char(char in_char){
    static WINDOW *charwin = NULL;
    if(charwin == NULL){
	charwin = newwin(LINES, WINDIV, 0, 0);
    }
    wprintw(charwin, "%c", in_char);
    wrefresh(charwin);
    return 0;
}

int start_anim(unsigned int seconds){
    struct itimerval ival;
    ival.it_interval.tv_usec = INTRVL;
    ival.it_interval.tv_sec = 0;
    ival.it_value.tv_usec = INTRVL;
    ival.it_value.tv_sec = 0;
    setitimer(ITIMER_REAL, &ival, NULL);
    return 0;
}

void draws(int sig){
    static WINDOW *vizwin = NULL;
    if(vizwin == NULL){
	vizwin = newwin(LINES, COLS - WINDIV, 0, WINDIV);
    }
    wprintw(vizwin, " hup! ");
    wrefresh(vizwin);
    signal(sig, draws);
}
