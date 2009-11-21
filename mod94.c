#include <ncurses.h>
#include <sys/time.h>
#include <signal.h>

void hup(int);

int main(){
    char ch;
    init();
    while(1){
	ch = getch();
	printw("%c", ch);
	refresh(); // Print it on to the real screen
    }
    endwin();
    return 0;
}

int init(){
    signal(SIGALRM, hup);
    start_anim();
    initscr(); // Start curses mode
    cbreak();  // Line buffering disabled
    noecho();  // Don't echo() while we do getch
    return 0;
}

int start_anim(unsigned int seconds){
    struct itimerval ival;
    ival.it_interval.tv_usec = 0;
    ival.it_interval.tv_sec = 5;
    ival.it_value.tv_usec = 0;
    ival.it_value.tv_sec = 5;
    setitimer(ITIMER_REAL, &ival, NULL);
    return 0;
}

void hup(int sig){
    printw(" hup! ");
    refresh();
    signal(sig, hup);
}
