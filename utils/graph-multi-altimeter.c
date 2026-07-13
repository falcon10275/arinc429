#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

// --- Global Shared State ---
// Removed temp and status_nominal
typedef struct {
    int altitude;
    int rpm;
    float fuel;
    float heading;
} FlightData;

FlightData fd = {10000, 2500, 100.0, 0.0}; // Initial values

// Locks
pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;   
volatile int keep_running = 1;

// --- Helper: Draw Altimeter ---
void draw_altimeter(WINDOW* win, int altitude) {
    int height, width;
    getmaxyx(win, height, width);

    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 0, 2, " ALTIMETER ");

    int center_y = height / 2;
    int center_x = width / 2;
    int base_100 = (altitude / 100) * 100;

    for (int offset = 600; offset >= -600; offset -= 100) {
        int tick_alt = base_100 + offset;
        if (tick_alt < 0) continue; 
        
        int delta_alt = tick_alt - altitude;
        int y_pos = center_y - (delta_alt / 25);

        if (y_pos > 0 && y_pos < height - 1) {
            if (abs(delta_alt) > 25) { 
                mvwprintw(win, y_pos, center_x - 4, "%05d -", tick_alt);
            }
        }
    }

    wattron(win, A_REVERSE);
    mvwprintw(win, center_y, center_x - 6, " >[%05d]< ", altitude);
    wattroff(win, A_REVERSE);
    wrefresh(win);
}

// --- Thread Functions ---

// Thread 0: Altimeter
void* thread_altimeter(void* arg) {
    WINDOW* win = (WINDOW*)arg;
    while (keep_running) {
        pthread_mutex_lock(&data_mutex);
        fd.altitude += (rand() % 121) - 60; 
        if (fd.altitude < 0) fd.altitude = 0;
        int current_alt = fd.altitude;
        pthread_mutex_unlock(&data_mutex);

        pthread_mutex_lock(&screen_mutex);
        draw_altimeter(win, current_alt);
        pthread_mutex_unlock(&screen_mutex);
        
        usleep(150000); 
    }
    return NULL;
}

// Thread 1: Engine RPM
void* thread_rpm(void* arg) {
    WINDOW* win = (WINDOW*)arg;
    while (keep_running) {
        pthread_mutex_lock(&data_mutex);
        fd.rpm += (rand() % 60) - 30;
        int current_rpm = fd.rpm;
        pthread_mutex_unlock(&data_mutex);
        
        pthread_mutex_lock(&screen_mutex);
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 0, 2, " ENGINE ");
        mvwprintw(win, 2, 2, "RPM: %04d", current_rpm);
        
        int bars = (current_rpm - 2000) / 100;
        mvwprintw(win, 3, 2, "[");
        for(int i=0; i<10; i++) waddch(win, i < bars ? '#' : ' ');
        waddch(win, ']');
        wrefresh(win);
        pthread_mutex_unlock(&screen_mutex);
        
        usleep(100000); 
    }
    return NULL;
}

// Thread 2: Fuel Level
void* thread_fuel(void* arg) {
    WINDOW* win = (WINDOW*)arg;
    while (keep_running) {
        pthread_mutex_lock(&data_mutex);
        fd.fuel -= 0.05;
        if (fd.fuel < 0) fd.fuel = 0;
        float current_fuel = fd.fuel;
        pthread_mutex_unlock(&data_mutex);

        pthread_mutex_lock(&screen_mutex);
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 0, 2, " FUEL ");
        mvwprintw(win, 2, 2, "LBS: %.1f%%", current_fuel);
        if (current_fuel < 20.0) {
            wattron(win, A_STANDOUT);
            mvwprintw(win, 4, 2, " LOW FUEL ");
            wattroff(win, A_STANDOUT);
        }
        wrefresh(win);
        pthread_mutex_unlock(&screen_mutex);
        
        usleep(500000); 
    }
    return NULL;
}

// Thread 3: Compass / Heading
void* thread_heading(void* arg) {
    WINDOW* win = (WINDOW*)arg;
    while (keep_running) {
        pthread_mutex_lock(&data_mutex);
        fd.heading += 0.5;
        if (fd.heading >= 360) fd.heading -= 360;
        float current_hdg = fd.heading;
        pthread_mutex_unlock(&data_mutex);

        pthread_mutex_lock(&screen_mutex);
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 0, 2, " HEADING ");
        mvwprintw(win, 2, 4, "HDG: %03d DEG", (int)current_hdg);
        wrefresh(win);
        pthread_mutex_unlock(&screen_mutex);
        
        usleep(250000); 
    }
    return NULL;
}

// Thread 4: The Global Logger
void* thread_logger(void* arg) {
    WINDOW* win = (WINDOW*)arg;
    
    WINDOW* sub_win = derwin(win, 16, 28, 1, 1);
    scrollok(sub_win, TRUE); 

    while (keep_running) {
        // 1. Safely copy all current data (removed temp and status)
        pthread_mutex_lock(&data_mutex);
        int alt = fd.altitude;
        int rpm = fd.rpm;
        float fuel = fd.fuel;
        int hdg = (int)fd.heading;
        pthread_mutex_unlock(&data_mutex);

        // 2. Draw to the screen
        pthread_mutex_lock(&screen_mutex);
        box(win, 0, 0); 
        mvwprintw(win, 0, 2, " TELEMETRY LOG ");
        wrefresh(win);

        // Print to the scrolling inner window
        wprintw(sub_win, "[LOG] ALT:%05d RPM:%04d\n", alt, rpm);
        wprintw(sub_win, "      FUEL:%.1f%% HDG:%03d\n", fuel, hdg);
        wprintw(sub_win, "----------------------------\n");
        wrefresh(sub_win);
        pthread_mutex_unlock(&screen_mutex);
        
        usleep(1500000); 
    }
    
    delwin(sub_win); 
    return NULL;
}


// --- Main Execution ---
int main() {
    srand(time(NULL));

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    // Create Windows (Removed temp and stat, moved log window left to x=48)
    WINDOW* win_alt  = newwin(18, 20, 1, 2);
    WINDOW* win_rpm  = newwin(6, 22, 1, 24);
    WINDOW* win_fuel = newwin(6, 22, 7, 24);
    WINDOW* win_hdg  = newwin(6, 22, 13, 24);
    WINDOW* win_log  = newwin(18, 30, 1, 48); 

    mvprintw(20, 2, "CONTROLS: [Q] Quit | Minimum terminal size: 80x24");
    refresh();

    // Array of threads (Reduced to 5)
    pthread_t threads[5];

    // Launch Threads
    pthread_create(&threads[0], NULL, thread_altimeter, win_alt);
    pthread_create(&threads[1], NULL, thread_rpm, win_rpm);
    pthread_create(&threads[2], NULL, thread_fuel, win_fuel);
    pthread_create(&threads[3], NULL, thread_heading, win_hdg);
    pthread_create(&threads[4], NULL, thread_logger, win_log);

    int ch;
    while (keep_running) {
        ch = getch();
        if (ch == 'q' || ch == 'Q') {
            keep_running = 0; 
        }
        usleep(50000); 
    }

    // Wait for remaining 5 threads
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    // Cleanup
    delwin(win_alt);
    delwin(win_rpm);
    delwin(win_fuel);
    delwin(win_hdg);
    delwin(win_log);
    endwin();

    return 0;
}