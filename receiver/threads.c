#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include "threads.h"
#include "state.h"

// Internal helper just for the altimeter thread
static void draw_altimeter(WINDOW* win, int altitude) {
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

void* thread_altimeter(void* arg) {
    WINDOW* win = (WINDOW*)arg;
    while (keep_running) {
        pthread_mutex_lock(&data_mutex);
        int current_alt = fd.altitude;
        pthread_mutex_unlock(&data_mutex);

        pthread_mutex_lock(&screen_mutex);
        draw_altimeter(win, current_alt);
        pthread_mutex_unlock(&screen_mutex);
        usleep(150000); 
    }
    return NULL;
}

void* thread_rpm(void* arg) {
    WINDOW* win = (WINDOW*)arg;
    while (keep_running) {
        pthread_mutex_lock(&data_mutex);
        int base_rpm = fd.rpm;
        pthread_mutex_unlock(&data_mutex);
        
        int display_rpm = base_rpm + ((rand() % 20) - 10);
        if (display_rpm < 0) display_rpm = 0;

        pthread_mutex_lock(&screen_mutex);
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 0, 2, " ENGINE ");
        mvwprintw(win, 2, 2, "RPM: %04d", display_rpm);
        
        int bars = (display_rpm) / 400; 
        mvwprintw(win, 3, 2, "[");
        for(int i=0; i<10; i++) waddch(win, i < bars ? '#' : ' ');
        waddch(win, ']');
        wrefresh(win);
        pthread_mutex_unlock(&screen_mutex);
        usleep(100000); 
    }
    return NULL;
}

void* thread_fuel(void* arg) {
    WINDOW* win = (WINDOW*)arg;
    while (keep_running) {
        pthread_mutex_lock(&data_mutex);
        float burn_rate = (float)fd.rpm * 0.0001; 
        fd.fuel -= burn_rate;
        if (fd.fuel < 0) fd.fuel = 0;
        float current_fuel = fd.fuel;
        pthread_mutex_unlock(&data_mutex);

        pthread_mutex_lock(&screen_mutex);
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 0, 2, " FUEL ");
        mvwprintw(win, 2, 2, "LBS: %05.1f%%", current_fuel);
        
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

void* thread_heading(void* arg) {
    WINDOW* win = (WINDOW*)arg;
    while (keep_running) {
        pthread_mutex_lock(&data_mutex);
        float current_hdg = fd.heading;
        pthread_mutex_unlock(&data_mutex);

        pthread_mutex_lock(&screen_mutex);
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 0, 2, " HEADING ");
        mvwprintw(win, 2, 4, "HDG: %03d DEG", (int)current_hdg);
        wrefresh(win);
        pthread_mutex_unlock(&screen_mutex);
        usleep(100000); 
    }
    return NULL;
}

void* thread_logger(void* arg) {
    WINDOW* win = (WINDOW*)arg;
    WINDOW* sub_win = derwin(win, 16, 28, 1, 1);
    scrollok(sub_win, TRUE); 

    while (keep_running) {
        pthread_mutex_lock(&data_mutex);
        int alt = fd.altitude;
        int rpm = fd.rpm;
        float fuel = fd.fuel;
        int hdg = (int)fd.heading;
        pthread_mutex_unlock(&data_mutex);

        pthread_mutex_lock(&screen_mutex);
        box(win, 0, 0); 
        mvwprintw(win, 0, 2, " TELEMETRY LOG ");
        wrefresh(win);

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