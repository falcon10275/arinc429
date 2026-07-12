#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define MENU_WIDTH 30
#define MENU_HEIGHT 10

// Window pointers
WINDOW *menu_win;
WINDOW *win1;
WINDOW *win2;
WINDOW *win3;

// Thread status flags to prevent running the same task multiple times concurrently
bool task1_running = false;
bool task2_running = false;
bool task3_running = false;

// Mutex for safe, synchronized drawing to the screen across threads
pthread_mutex_t curses_mutex = PTHREAD_MUTEX_INITIALIZER;

// Menu items
const char *choices[] = {
    "Run Task 1",
    "Run Task 2",
    "Run Task 3",
    "Exit",
};
int n_choices = sizeof(choices) / sizeof(char *);

// Task 1: Increments a counter in Window 1
void *task1_func(void *arg) {
    if (task1_running) return NULL;
    task1_running = true;

    for (int i = 1; i <= 20; i++) {
        pthread_mutex_lock(&curses_mutex);
        wclear(win1);
        box(win1, 0, 0);
        mvwprintw(win1, 1, 1, "Task 1 Status:");
        mvwprintw(win1, 2, 1, "Progress: %d%%", i * 5);
        wrefresh(win1);
        pthread_mutex_unlock(&curses_mutex);
        usleep(200000); // Sleep 200ms
    }

    task1_running = false;
    return NULL;
}

// Task 2: Displays a bouncing visual character in Window 2
void *task2_func(void *arg) {
    if (task2_running) return NULL;
    task2_running = true;

    int x = 1, y = 2;
    int dx = 1;
    int max_x = getmaxx(win2) - 2;

    for (int i = 0; i < 40; i++) {
        pthread_mutex_lock(&curses_mutex);
        wclear(win2);
        box(win2, 0, 0);
        mvwprintw(win2, 1, 1, "Task 2 Status:");
        
        // Bounce logic
        x += dx;
        if (x >= max_x || x <= 1) dx = -dx;
        
        mvwprintw(win2, y, x, "*");
        wrefresh(win2);
        pthread_mutex_unlock(&curses_mutex);
        usleep(150000); // Sleep 150ms
    }

    task2_running = false;
    return NULL;
}

// Task 3: Simulates a data loading spinner in Window 3
void *task3_func(void *arg) {
    if (task3_running) return NULL;
    task3_running = true;

    char spinner[] = {'|', '/', '-', '\\'};

    for (int i = 0; i < 30; i++) {
        pthread_mutex_lock(&curses_mutex);
        wclear(win3);
        box(win3, 0, 0);
        mvwprintw(win3, 1, 1, "Task 3 Status:");
        mvwprintw(win3, 2, 1, "Loading... %c", spinner[i % 4]);
        wrefresh(win3);
        pthread_mutex_unlock(&curses_mutex);
        usleep(250000); // Sleep 250ms
    }

    pthread_mutex_lock(&curses_mutex);
    mvwprintw(win3, 2, 1, "Done!         ");
    wrefresh(win3);
    pthread_mutex_unlock(&curses_mutex);

    task3_running = false;
    return NULL;
}

// Helper function to print the menu options
void print_menu(WINDOW *menu_win, int highlight) {
    int x = 2;
    int y = 2;
    box(menu_win, 0, 0);
    mvwprintw(menu_win, 1, 2, "MAIN MENU");
    
    for (int i = 0; i < n_choices; ++i) {
        if (highlight == i + 1) {
            wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, y, x, "%s", choices[i]);
            wattroff(menu_win, A_REVERSE);
        } else {
            mvwprintw(menu_win, y, x, "%s", choices[i]);
        }
        ++y;
    }
    wrefresh(menu_win);
}

int main() {
    // Initialize ncurses environment
    initscr();
    clear();
    noecho();
    cbreak(); // Line buffering disabled
    curs_set(0); // Hide the blinking cursor

    int parent_x, parent_y;
    getmaxyx(stdscr, parent_y, parent_x);

    // Dynamic grid sizing calculation for the 4 windows
    int win_width = parent_x / 2;
    int win_height = parent_y / 2;

    // Allocate the 4 windows to screen quad-segments
    menu_win = newwin(win_height, win_width, 0, 0);
    win1     = newwin(win_height, win_width, 0, win_width);
    win2     = newwin(win_height, win_width, win_height, 0);
    win3     = newwin(win_height, win_width, win_height, win_width);

    // Initial base draw of window borders
    box(menu_win, 0, 0);
    box(win1, 0, 0);
    box(win2, 0, 0);
    box(win3, 0, 0);

    mvwprintw(win1, 1, 1, "Window 1: Idle");
    mvwprintw(win2, 1, 1, "Window 2: Idle");
    mvwprintw(win3, 1, 1, "Window 3: Idle");

    // Enable keypad input (arrow keys) on the menu window
    keypad(menu_win, TRUE);

    int highlight = 1;
    int choice = 0;
    int c;

    pthread_t thread1, thread2, thread3;

    print_menu(menu_win, highlight);

    // Main Interactive Loop
    while (1) {
        c = wgetch(menu_win);
        switch (c) {
            case KEY_UP:
                if (highlight == 1)
                    highlight = n_choices;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if (highlight == n_choices)
                    highlight = 1;
                else
                    ++highlight;
                break;
            case 10: // Enter Key
                choice = highlight;
                break;
            default:
                break;
        }

        print_menu(menu_win, highlight);

        if (choice != 0) {
            if (choice == 1) {
                pthread_create(&thread1, NULL, task1_func, NULL);
                pthread_detach(thread1); // Detach so resources clean up automatically
            } else if (choice == 2) {
                pthread_create(&thread2, NULL, task2_func, NULL);
                pthread_detach(thread2);
            } else if (choice == 3) {
                pthread_create(&thread3, NULL, task3_func, NULL);
                pthread_detach(thread3);
            } else if (choice == 4) {
                // Exit Option
                break;
            }
            choice = 0; // Reset choice selection
        }
    }

    // Clean up ncurses environment safely
    clrtoeol();
    refresh();
    endwin();
    return 0;
}
