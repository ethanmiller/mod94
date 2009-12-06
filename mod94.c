#include <ncurses.h>
#include <sys/time.h>
#include <signal.h>

#define WINDIV 30
#define INTRVL 41666 // approx 24 times a second
#define NFOUR 94
#define WLEN 15


void draws(int);
int handle_char(char in_char);

char chlist[NFOUR] = {'\0'};
char whold[WLEN] = {'\0'};
int poslist[NFOUR] = {0};

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
    static int windx = 0;
    static int clindx = 0;
    int i, got_gap;
    if(charwin == NULL) charwin = newwin(LINES, WINDIV, 0, 0);
    
    // add char to whold
    whold[windx++] = in_char;
    windx %= WLEN;
    if(in_char == ' '){
	//wprintw(charwin, "%d", clindx);
	// copy word to chlist, and clear whold
	for(i = 0; i < windx; i++){
	    chlist[clindx++] = whold[i];
	    clindx %= NFOUR;
	    whold[i] = '\0';
	}
	// since we're prob overwriting prev words,
	// clear till we get to a gap
	got_gap = 0;
	i = clindx;
	while(!got_gap){
	   if(chlist[i] == '\0' || chlist[i] == ' '){
	       got_gap = 1;
	   }else{
	       chlist[i++] = ' ';
	       i %= NFOUR;
	   } 
	}
	// start new word in whold
	windx = 0;
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
    static int idx = 0;
    int i;
    if(vizwin == NULL) vizwin = newwin(LINES, COLS - WINDIV, 0, WINDIV);

    wclear(vizwin);
    for(i = 0; i < NFOUR; i++){
	mvwprintw(vizwin, 0, i, "%c", chlist[i]);
	mvwprintw(vizwin, 1, i, "%d", i % 10);
    }
    wrefresh(vizwin);

    /*
    // kind of wasting time here, but I want to make sure
    // I'm cycling through arrays correctly
    if(chlist[idx] != '\0'){
	if(chlist[idx] == ' '){
	    while(chlist[idx + 1] == ' '){
		idx++;
	    }
	}
	wprintw(vizwin, "%c", chlist[idx++]);
	wrefresh(vizwin);
    }else{
	if(idx != 0) idx = 0;
    }
    */
    signal(sig, draws);
}
