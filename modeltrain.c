#include <ncurses.h>  //needs -lncurses gcc flag
#include <stdlib.h>  //usleep
#include <unistd.h>	//mallloc

/*==================================== typedefs ==============================================================*/

typedef struct traincar{
	char car;
	int x,y;
	void(*charSet)(void);
}traincar;

/*==================================== macros,typedefs, and constants =========================================*/

#define MID_X  (COLS/2)
#define MID_Y  (LINES/2)
#define LEFT  (MID_X - 10)
#define RIGHT  (MID_X + 10)
#define BOTTOM (MID_Y + 5)
#define TOP  (MID_Y) - 5
#define SLEEP_TIME_BASE 299999

static traincar *train[6];
static int speed = 1;
static int run = 1;
WINDOW *prompt_win;

/*==================================== function prototypes  ===================================================*/

void printTrack(void);
void printTrain(void);
void setTrainPiecesXY(const int i);
void init(void);
void initTrain(void);
void runTrain(void);
void cleanUp(void);
void setChar1(void);
void setChar2(void);
void printPrompt(void);
void waitForInput(void);
void printGame(void);
void setTrainColorOn(const int *const i);
void setTrainColorOff(const int *const i);


/*==================================== main program ===========================================================*/

int main (void) {
	init();
	initTrain();
	printGame();
	waitForInput();
	runTrain();
	cleanUp();
	return 0;
}

void printPrompt (void) {
	switch(speed){
		case 2:  
			mvwaddstr(prompt_win,0,COLS/2-5, "LOW SPEED");
			break;
		case 6:  
			mvwaddstr(prompt_win,0,COLS/2-5, "MAX SPEED!!!!!!");
			break;
		case 1:  
			mvwaddstr(prompt_win,0,COLS/2-5, "Stopped!");
			break;
		default: //do nothing
			break;
	}
	mvwaddstr(prompt_win,1,COLS/2-15, "press Right arrow for increased speed");
	mvwaddstr(prompt_win,2,COLS/2-15, "Left arrow for decreased speed");
	mvwaddstr(prompt_win,3,COLS/2-15, "Up arrow to exit");
}

//waits until either up arrow or right arrow is pressed
void waitForInput(void) {
	int c;
	while((c = getch()) != KEY_RIGHT && c != KEY_UP);
		if(c == KEY_UP) {
			run = 0;
		}
		else {
			speed++;
		}
}

void runTrain(void) {
	while(run) {
		switch(getch()) {
			case KEY_UP:
			 	run = 0;
				break;
			case KEY_RIGHT: 
				speed++;
				if (speed > 6) {
					speed = 6;
			}
				break;
			case KEY_LEFT: 
				speed--;
				if(speed <= 1) {
					speed = 1;
					printGame();
					waitForInput();
				}
				break;
			default: //do nothing
				break;
			}
		usleep(SLEEP_TIME_BASE/speed);
		printGame();
	}
	cleanUp();
}

void init(void) {
	initscr();
	noecho();	      //dont display key strokes
	cbreak();	     //disable line buffering
	curs_set(0);	//dont display cursor
	start_color();                          
	init_pair(1,COLOR_WHITE,COLOR_BLACK);  //set color pairs for tracks
	init_pair(2,COLOR_YELLOW,COLOR_BLACK);  //set color pairs for engine
	init_pair(3,COLOR_GREEN,COLOR_BLACK); //set color pair for train cars
	init_pair(4,COLOR_RED,COLOR_BLACK);   //set color pairs for caboose
	keypad(stdscr, TRUE);	             //enable keypad
	nodelay(stdscr,TRUE);	            //sets getch as non-blocking
	prompt_win = newwin(5,100,BOTTOM+1,0);
}

void printGame(void) {
	clear();
	wclear(prompt_win);
	printTrack();
	printTrain();
	printPrompt();
	refresh();
	wrefresh(prompt_win);
}

void printTrack(void) {
	int i;

	//turns on color attribute for tracks
	attron(COLOR_PAIR(1));

	//set top and bottom row of tracks
	for(i = LEFT; i <= RIGHT; i++) {
		mvprintw(TOP,i,"#");
		mvprintw(BOTTOM,i,"#");
	}

	//set left and right column of tracks
	for(i = TOP; i <= BOTTOM; i++) {
		mvprintw(i,LEFT,"#");
		mvprintw(i,RIGHT,"#");
	}

	//turns off color attribute for tracks
	attroff(COLOR_PAIR(1));
}

void printTrain(void) {
	int i;
	for (i = 0; i < 6; i++) {
		setTrainPiecesXY(i);
		if (i == 0 || i == 5){
			train[i]->charSet();
		}
		setTrainColorOn(&i);
		mvaddch(train[i]->y,train[i]->x,train[i]->car);
		setTrainColorOff(&i);
	}
}

void setTrainColorOn(const int *const i) {
	switch(*i) {
		case 0: attron(COLOR_PAIR(2));
			break;
		case 1: //fall through
		case 2:
		case 3:
		case 4: attron(COLOR_PAIR(3));
			break;	
		case 5: attron(COLOR_PAIR(4));
			break;
		default: //should never reach here
			break;	
	}
}
void setTrainColorOff(const int *const i) {
	switch(*i) {
		case 0:attroff(COLOR_PAIR(2));
			break;
		case 1: //fall through
		case 2: 
		case 3:
		case 4:attroff(COLOR_PAIR(3));
			break;	
		case 5: attroff(COLOR_PAIR(4));
			break;
		default: //should never reach here
			break;	
	}
}

//sets where each train car should be as it moves around the track
void setTrainPiecesXY(const int i) {

	//if train car is moving on left or right tracks then move it up or down as needed
	if (train[i]->y != BOTTOM && train[i]->y != TOP) {
		if(train[i]->x == RIGHT){
			train[i]->y--;
		}
		else {
			train[i]->y++;
		}
	}
	//if train car is on bottom track then move it right or if hits the corner track, moves train car up right track
	else if (train[i]->y == BOTTOM) {
		if(train[i]->x != RIGHT) {
			train[i]->x++;
		}
		else {
			train[i]->y--;
		}
	}
	//if train car is on top track then move it left or if it hits corner track moves train car down left track
	else {
		if(train[i]->x == LEFT){
			train[i]->y++;
		}
		else {
			train[i]->x--;
		}
	}
}

void initTrain(void) {
	int i;
	const size_t size_traincars = sizeof(traincar);
	for(i = 0; i < 6; i++) {
		train[i] = malloc(size_traincars);
	}
	for( i = 0; i <6; i++) {
		train[i]->x = MID_X - i;
		train[i]->y = BOTTOM;
		train[i]->car = '+';
	}

	train[0]->car = '>';
	train[0]->charSet = setChar1;
	train[5]->car = '<';
	train[5]->charSet = setChar2;
}

void cleanUp(void) {
	int i;
	for(i = 0; i < 6; i++){
		free(train[i]);
	}
	endwin(); 	//ends ncrusive control of terminal 
}

//changes direction of 'engine' as it moves around track
void setChar1(void) {
	if(train[0]->y == BOTTOM) {
		train[0]->car = '>';
	}
	else if (train[0]->y == TOP) {
		train[0]->car = '<';
	}
	else if (train[0]->x == RIGHT) {
		train[0]->car = '^';
	}
	else {
		train[0]->car = 'v';
	}

}

//changes direction of 'caboose' as it moves around track
void setChar2(void) {
	if(train[5]->y == BOTTOM) {
		train[5]->car = '<';
	}
	else if (train[5]->y == TOP) {
		train[5]->car = '>';
	}
	else if (train[5]->x == RIGHT) {
		train[5]->car = 'v';
	}
	else {
		train[5]->car = '^';
	}

}

