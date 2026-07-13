#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>

// --- Flight State Variables ---
int throttle_pct = 0;    // 0% to 100%
int bank_angle   = 0;    // -45 (Left) to +45 (Right) degrees
int pitch_angle  = 0;    // -20 (Dive) to +20 (Climb) degrees
float altitude   = 5000.0; 
int rpm          = 0;

// --- Helper Functions ---
void draw_panel_borders() {
    // Yoke Bounding Box
    mvprintw(3, 5,  "+-----------------------------------+");
    for(int i = 4; i <= 14; i++) {
        mvprintw(i, 5,  "|                                   |");
    }
    mvprintw(15, 5, "+-----------------------------------+");
    mvprintw(2, 18, " YOKE (PITCH/BANK) ");

    // Throttle Bounding Box
    mvprintw(3, 45, "+-------+");
    for(int i = 4; i <= 14; i++) {
        mvprintw(i, 45, "|       |");
    }
    mvprintw(15, 45,"+-------+");
    mvprintw(2, 45, " THROTTLE ");
}

void update_physics() {
    // 1. Calculate RPM from Throttle (Idle at 800, Max at 2800)
    if (throttle_pct == 0) {
        rpm = 0; // Engine off
    } else {
        rpm = 800 + (throttle_pct * 20); 
    }

    // 2. Calculate Altitude based on Pitch and RPM
    // A very simplified physics model: 
    // You need engine thrust (RPM) to climb. If you pitch up without thrust, you stall/sink.
    float thrust_factor = (float)rpm / 2800.0; 
    float vertical_speed = (pitch_angle * thrust_factor) - (2.0 * (1.0 - thrust_factor));
    
    // Auto-leveling logic for bank/pitch to simulate aerodynamics
    // If user lets go of the yoke, the plane naturally wants to level out slightly
    if (bank_angle > 0) bank_angle -= 1;
    if (bank_angle < 0) bank_angle += 1;

    altitude += (vertical_speed * 0.5); // Apply vertical speed to altitude
    if (altitude < 0) altitude = 0;     // Ground level
}

int main() {
    // Initialize ncurses
    initscr();
    cbreak();             // Disable line buffering
    noecho();             // Hide typed characters
    keypad(stdscr, TRUE); // Enable arrow keys
    nodelay(stdscr, TRUE); // Non-blocking input
    curs_set(0);          // Hide cursor

    int ch;
    
    while (1) {
        // --- 1. Input Handling ---
        ch = getch();
        if (ch == 'q' || ch == 'Q') {
            break;
        } 
        else if (ch == KEY_UP) {
            pitch_angle -= 2; // Push yoke forward (Dive)
            if (pitch_angle < -20) pitch_angle = -20;
        } 
        else if (ch == KEY_DOWN) {
            pitch_angle += 2; // Pull yoke back (Climb)
            if (pitch_angle > 20) pitch_angle = 20;
        } 
        else if (ch == KEY_LEFT) {
            bank_angle -= 5;  // Turn Yoke Left
            if (bank_angle < -45) bank_angle = -45;
        } 
        else if (ch == KEY_RIGHT) {
            bank_angle += 5;  // Turn Yoke Right
            if (bank_angle > 45) bank_angle = 45;
        } 
        else if (ch == 'w' || ch == 'W') {
            throttle_pct += 5; // Throttle Up
            if (throttle_pct > 100) throttle_pct = 100;
        } 
        else if (ch == 's' || ch == 'S') {
            throttle_pct -= 5; // Throttle Down
            if (throttle_pct < 0) throttle_pct = 0;
        }

        // --- 2. Physics & Logic ---
        update_physics();

        // --- 3. Rendering ---
        clear();

        // Draw Telemetry Dashboard
        attron(A_BOLD);
        mvprintw(0, 2,  "ALTITUDE: %05d FT", (int)altitude);
        mvprintw(0, 25, "RPM: %04d", rpm);
        mvprintw(0, 40, "BANK: %03d DEG", bank_angle);
        mvprintw(0, 58, "PITCH: %02d DEG", pitch_angle);
        attroff(A_BOLD);

        draw_panel_borders();

        // Draw the Yoke visualizer
        // Map bank_angle (-45 to 45) to X-axis (offset -12 to +12)
        // Map pitch_angle (-20 to 20) to Y-axis (offset -4 to +4)
        int yoke_center_x = 22;
        int yoke_center_y = 9;
        
        int yoke_x = yoke_center_x + (bank_angle / 3);
        int yoke_y = yoke_center_y - (pitch_angle / 4); // Invert Y so pull back = yoke moves down

        // Draw yoke shape depending on bank angle to simulate rotation
        if (bank_angle <= -15) {
            mvprintw(yoke_y, yoke_x - 3, "//=[O]=//");
        } else if (bank_angle >= 15) {
            mvprintw(yoke_y, yoke_x - 3, "\\\\=[O]=\\\\");
        } else {
            mvprintw(yoke_y, yoke_x - 3, "==[O]==");
        }

        // Draw the Throttle Lever
        // Track goes from Y=14 (0%) to Y=4 (100%)
        int track_y_start = 4;
        int track_length = 10;
        int throttle_pos = 14 - (throttle_pct / 10); // 10% increments

        // Draw track slot
        for (int i = 0; i <= track_length; i++) {
            mvaddch(track_y_start + i, 49, '|');
        }
        // Draw the lever handle
        mvprintw(throttle_pos, 47, "[==]"); 

        // Controls footer
        mvprintw(17, 2, "CONTROLS: [ARROWS] Move Yoke | [W/S] Throttle Up/Down | [Q] Quit");

        refresh();

        // --- 4. Frame Pacing ---
        usleep(50000); // 50ms delay (~20 FPS)
    }

    // Clean up ncurses on exit
    endwin();
    return 0;
}