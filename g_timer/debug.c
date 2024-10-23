#include <curses.h>
#include <string.h>

int main()
{
    char str[80];

    initscr();
    //cbreak();
    //echo();
	int i=0, y, x;


	y=1;x=1;
	mvprintw(y++, x, "Timer status");
	mvprintw(y++, x, "============");
	for(i=0; i < 10; i++, y++){
		mvprintw(y, x, "Timer[%.2d]=%s", i, "func");
	}

	y=14;x=20;
	mvprintw(y++, x, "Thread pool status");
	mvprintw(y++, x, "==================");
	for(i=0; i < 10; i++, y++){
		mvprintw(y, x, "Thread[%.2d]=%s", i, "func");
	}

    getch();
    endwin();
    return 0;
}
