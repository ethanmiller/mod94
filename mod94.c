#include <ncurses.h>
#include <sys/time.h>
#include <signal.h>

#define WINDIV 30
#define INTRVL 41666 // approx 24 times a second
#define WRDS 10
#define WLEN 15


void draws(int);
int handle_char(char in_char);

struct wordstor{
    char cwords[WRDS][WLEN];
    int poslist[WRDS][3];
}; 
struct wordstor words;

int main(){
    char ch;
    int i, ii;
    //init wordstor struct
    for(i = 0; i < WRDS; i++){
	clear_word(i);
	words.poslist[i][0] = 0;
	words.poslist[i][1] = 0;
	words.poslist[i][2] = 0;
    }
    setup();
    while(1){
	handle_char(getch());
    }
    endwin();
    return 0;
    // for scrolling see http://www.mkssoftware.com/docs/man3/curs_scroll.3.asp
}

int clear_word(int windx){
    int i;
    for(i = 0; i < WLEN; i++){
	words.cwords[windx][i] = '\0';
    }
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
    static int chindx = 0;
    if(charwin == NULL) charwin = newwin(LINES, WINDIV, 0, 0);
    if(in_char == 127) return 0; // NO DELETE

    // add char to current word
    words.cwords[windx][chindx++] = in_char;
    chindx %= WLEN;
    if(in_char == ' '){
	chindx = 0;
	windx++;
	windx %= WRDS;
	// clear in case we've used this array before
	clear_word(windx);
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

/*
 * Drawing rules:
 * The idea here is that each 'word' is a program that creates a pattern.
 * A word is any string of characters, space separated.
 * NOTE: The set of characters I'm using starts with 32 (space) and goes to 
 * 126 (~) - that's 94 chars.
 * The word can be of any length (loops back to the first character if the word 
 * isn't long enough), and the settings affected are determined
 * by the character at each position - as follows:
 *
 * 0 - The value of the character at this position determines what quadrant
 * the animation starts from, (char - 32)/94 * 4
 *
 * 1 - Determines the rate of animation.. (?)
 *
 * 2,3,4 - Determine RGB colors 
 *
 * All subsequent - Determine a set of jumps for the next character to print out. 
 * For example : a is 97, minus 32 is 65. So jump 65 (with % 94) gets us 36, add 32 
 * for 68 = D. The next char (if we haven't looped back to the beginning already) 
 * starts the process again at 68.
 */

void draws(int sig){
    static WINDOW *vizwin = NULL;
    int i, ii;
    if(vizwin == NULL) vizwin = newwin(LINES, COLS - WINDIV, 0, WINDIV);

    wclear(vizwin);
    // fish out each word, and it's xy position
    for(i = 0; i < WRDS; i++){
	for(ii = 0; ii < WLEN; ii++){
	    mvwprintw(vizwin, i, ii, "%c", words.cwords[i][ii]);
	}
	mvwprintw(vizwin, i, WLEN + 1, "%d", i % 10);
    }
    wrefresh(vizwin);
    signal(sig, draws);
}
