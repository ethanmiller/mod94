#include <ncurses.h>
#include <sys/time.h>
#include <signal.h>
#include <math.h>

#define WINDIV 30
#define INTRVL 41666 // approx 24 times a second
#define WRDS 10
#define WLEN 15
// these are indexes for wrules values
#define RULE_START 0
#define RULE_X 1
#define RULE_Y 2
#define RULE_R 3
#define RULE_G 4
#define RULE_B 5
#define RULE_CT 6


void draws(int);
int handle_char(char in_char);

struct wordstor{
    char cwords[WRDS][WLEN];
    int wrules[WRDS][RULE_CT];
}; 
struct wordstor words;

int main(){
    char ch;
    int i;
    //init wordstor struct
    for(i = 0; i < WRDS; i++){
	clear_word(i);
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
    if(in_char > 126 || in_char < 32) return 0;

    // add char to current word
    words.cwords[windx][chindx++] = in_char;
    chindx %= WLEN;
    if(in_char == ' '){
	chindx = 0;
	extract_config(windx);
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
 * the animation starts from, floor((char - 31)/96 * 4)
 * (31 so we wont get 0, and 96 so that we cal floor() and never get 4
 *
 * 1,2,3 - Determine RGB colors 
 *
 * All subsequent - Determine a set of jumps for the next character to print out. 
 * For example : a is 97, minus 32 is 65. So jump 65 (with % 94) gets us 36, add 32 
 * for 68 = D. The next char (if we haven't looped back to the beginning already) 
 * starts the process again at 68.
 */

int extract_config(int i){
    // extracts the rules for a word, stores them in wrules
    int cpos = 0;
    // Starting position 0 - 3
    words.wrules[i][RULE_START] = floor(((float)words.cwords[i][cpos] - 31)/96 * 4);
    add_one_mod(&cpos, i); // ***
    // Based on the starting position, set x and y
    switch(words.wrules[i][0]){
	case 0:
	    // upper left
	    words.wrules[i][RULE_X] = 0;
	    words.wrules[i][RULE_Y] = 0;
	    break;
	case 1:
	    // upper right
	    words.wrules[i][RULE_X] = COLS - WINDIV;
	    words.wrules[i][RULE_Y] = 0;
	    break;
	case 2:
	    // lower right
	    words.wrules[i][RULE_X] = COLS - WINDIV;
	    words.wrules[i][RULE_Y] = LINES;
	    break;
	case 3:
	    // lower left
	    words.wrules[i][RULE_X] = 0;
	    words.wrules[i][RULE_Y] = LINES;
	    break;
    }
    // Red
    words.wrules[i][RULE_R] = floor(((float)words.cwords[i][cpos] - 31)/96 * 1000);
    add_one_mod(&cpos, i); // ***
    // Green
    words.wrules[i][RULE_G] = floor(((float)words.cwords[i][cpos] - 31)/96 * 1000);
    add_one_mod(&cpos, i); // ***
    // Blue
    words.wrules[i][RULE_B] = floor(((float)words.cwords[i][cpos] - 31)/96 * 1000);
    return 0;
}

int add_one_mod(int *ichar, int iword){
    *ichar += 1;
    char c = words.cwords[iword][*ichar];
    if(*ichar > WLEN || c == ' ' || c == '\0'){
	*ichar = 0;
    }
}

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
