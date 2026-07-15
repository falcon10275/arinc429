#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "state.h"
#include "threads.h"


int main() {
    srand(time(NULL));

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    WINDOW* win_alt  = newwin(18, 20, 1, 2);
    WINDOW* win_rpm  = newwin(6, 22, 1, 24);
    WINDOW* win_fuel = newwin(6, 22, 7, 24);
    WINDOW* win_hdg  = newwin(6, 22, 13, 24);
    WINDOW* win_log  = newwin(18, 30, 1, 48); 

    mvprintw(20, 2, "[Q] Quit");
    refresh();

    pthread_t threads[THREAD_COUNT];

    pthread_create(&threads[0], NULL, thread_altimeter, win_alt);
    pthread_create(&threads[1], NULL, thread_rpm, win_rpm);
    pthread_create(&threads[2], NULL, thread_fuel, win_fuel);
    pthread_create(&threads[3], NULL, thread_heading, win_hdg);
    pthread_create(&threads[5], NULL, thread_logger, win_log);
    pthread_create(&threads[5], NULL, thread_receive_429_udp, NULL);

    int ch;
    while (keep_running) {
        ch = getch();
        if (ch != ERR) {
            pthread_mutex_lock(&data_mutex);
            
            if (ch == 'q' || ch == 'Q') keep_running = 0; 

            pthread_mutex_unlock(&data_mutex);
        }
        usleep(30000); 
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    delwin(win_alt);
    delwin(win_rpm);
    delwin(win_fuel);
    delwin(win_hdg);
    delwin(win_log);
    endwin();

    return 0;
}