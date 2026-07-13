#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

// --- Global Shared State ---
typedef struct {
    int altitude;
    int rpm;
    float fuel;
    float heading;
} FlightData;

// Start with standard cruise values
FlightData fd = {10000, 2500, 100.0, 0.0}; 

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
        int base_rpm = fd.rpm;
        pthread_mutex_unlock(&data_mutex);
        
        // Add a slight visual jitter to the needle so the engine feels "alive" 
        // even when the user isn't actively changing the RPM
        int display_rpm = base_rpm + ((rand() % 20) - 10);
        if (display_rpm < 0) display_rpm = 0;

        pthread_mutex_lock(&screen_mutex);
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 0, 2, " ENGINE ");
        mvwprintw(win, 2, 2, "RPM: %04d", display_rpm);
        
        int bars = (display_rpm) / 400; // Max visual bars at 4000 RPM
        mvwprintw(win, 3, 2, "[");
        for(int i=0; i<10; i++) waddch(win, i < bars ? '#' : ' ');
        waddch(win, ']');
        wrefresh(win);
        pthread_mutex_unlock(&screen_mutex);
        
        usleep(100000); 
    }
    return NULL;
}

// Thread 2: Fuel Level (Burn rate tied to RPM)
void* thread_fuel(void* arg) {
    WINDOW* win = (WINDOW*)arg;
    while (keep_running) {
        pthread_mutex_lock(&data_mutex);
        
        // Calculate burn rate. 
        // Using 0.0001 multiplier so it drains noticeably for demo purposes.
        // E.g., 2500 RPM * 0.0001 = 0.25% fuel burned per tick.
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
        
        usleep(500000); // Ticks every half second
    }
    return NULL;
}

// Thread 3: Compass / Heading
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
        
        usleep(100000); // Sped up update rate so arrow keys feel responsive
    }
    return NULL;
}

// Thread 4: The Global Logger
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

// --- Main Execution ---
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

    // Updated control instructions
    mvprintw(20, 2, "CONTROLS: [ARROWS] Pitch/Yaw | [- /=] Throttle | [Q] Quit");
    refresh();

    pthread_t threads[5];

    pthread_create(&threads[0], NULL, thread_altimeter, win_alt);
    pthread_create(&threads[1], NULL, thread_rpm, win_rpm);
    pthread_create(&threads[2], NULL, thread_fuel, win_fuel);
    pthread_create(&threads[3], NULL, thread_heading, win_hdg);
    pthread_create(&threads[4], NULL, thread_logger, win_log);

    int ch;
    while (keep_running) {
        ch = getch();
        if (ch != ERR) {
            // Lock the data state before allowing the keyboard to change it
            pthread_mutex_lock(&data_mutex);
            
            if (ch == 'q' || ch == 'Q') {
                keep_running = 0; 
            } 
            else if (ch == KEY_UP) {
                fd.altitude += 20;
            } 
            else if (ch == KEY_DOWN) {
                fd.altitude -= 20;
                if (fd.altitude < 0) fd.altitude = 0;
            } 
            else if (ch == KEY_LEFT) {
                fd.heading -= 5.0; // Turn left 5 degrees
                if (fd.heading < 0) fd.heading += 360.0;
            } 
            else if (ch == KEY_RIGHT) {
                fd.heading += 5.0; // Turn right 5 degrees
                if (fd.heading >= 360.0) fd.heading -= 360.0;
            } 
            else if (ch == '=' || ch == '+') { // Support both in case user forgets Shift
                fd.rpm += 100;
                if (fd.rpm > 4000) fd.rpm = 4000; // Redline at 4000 RPM
            } 
            else if (ch == '-') {
                fd.rpm -= 100;
                if (fd.rpm < 0) fd.rpm = 0; // Engine off
            }

            pthread_mutex_unlock(&data_mutex);
        }
        usleep(30000); // Run keyboard check at ~33 FPS for smooth input
    }

    for (int i = 0; i < 5; i++) {
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