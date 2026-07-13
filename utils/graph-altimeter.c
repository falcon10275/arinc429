#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>

void draw_hud_box(int start_y, int start_x, int height, int width) {
    // Draw a simple border for the altimeter tape
    for (int i = 0; i < height; i++) {
        mvaddch(start_y + i, start_x, '|');
        mvaddch(start_y + i, start_x + width, '|');
    }
    mvprintw(start_y - 1, start_x, "+-------+");
    mvprintw(start_y + height, start_x, "+-------+");
}

int main() {
    int ch;
    int altitude = 10000; // Starting altitude in feet

    // Initialize ncurses
    initscr();
    cbreak();             // Disable line buffering
    noecho();             // Don't echo pressed keys to the screen
    keypad(stdscr, TRUE); // Enable reading of arrow keys
    nodelay(stdscr, TRUE); // Make getch() non-blocking
    curs_set(0);          // Hide the terminal cursor

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    while (1) {
        // 1. Handle Input
        ch = getch();
        if (ch == 'q' || ch == 'Q') {
            break;
        } else if (ch == KEY_UP) {
            altitude += 20; // Ascend
        } else if (ch == KEY_DOWN) {
            altitude -= 20; // Descend
        }

        // Prevent negative altitude (flying underground!)
        if (altitude < 0) {
            altitude = 0;
        }

        // 2. Clear Screen for next frame
        clear();

        // 3. Calculate Layout
        int center_y = max_y / 2;
        int center_x = max_x / 2;
        int tape_height = 15;
        int tape_top = center_y - (tape_height / 2);

        // Draw Headers
        mvprintw(tape_top - 3, center_x - 4, "ALTIMETER");
        mvprintw(tape_top - 2, center_x - 3, "(FEET)");

        // Draw the bounding box
        draw_hud_box(tape_top, center_x - 5, tape_height, 13);

        // 4. Draw the Scrolling Tape
        // We calculate the nearest 100-foot increment to base our tape on
        int base_100 = (altitude / 100) * 100;

        // Iterate through an altitude range to draw ticks above and below
        for (int offset = 600; offset >= -600; offset -= 100) {
            int tick_alt = base_100 + offset;
            
            if (tick_alt < 0) continue; // Don't draw negative altitude ticks

            // Calculate vertical position on the screen
            // 25 feet of altitude difference = 1 row on the screen
            int delta_alt = tick_alt - altitude;
            int y_pos = center_y - (delta_alt / 25);

            // Only draw the tick if it falls inside our HUD box boundaries
            if (y_pos >= tape_top && y_pos < tape_top + tape_height) {
                // Don't draw the background tape text right in the middle
                if (abs(delta_alt) > 25) { 
                    mvprintw(y_pos, center_x - 3, "%05d -", tick_alt);
                }
            }
        }

        // 5. Draw the exact digital readout in the center
        attron(A_REVERSE); // Invert colors to make it pop like a real HUD bracket
        mvprintw(center_y, center_x - 7, " >[%05d]< ", altitude);
        attroff(A_REVERSE);

        // Draw instructions at the bottom
        mvprintw(max_y - 2, 2, "CONTROLS: [UP ARROW] Pitch Up | [DOWN ARROW] Pitch Down | [q] Quit");

        // 6. Render and wait
        refresh();
        usleep(30000); // 30ms delay (~33 FPS) for smooth scrolling
    }

    // Clean up and close ncurses properly
    endwin();
    return 0;
}