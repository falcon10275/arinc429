#include <ncurses.h>
#include "render.h"
#include "flight_state.h"

void draw_panel_borders(void) {
    mvprintw(3, 5,  "+-----------------------------------+");
    for(int i = 4; i <= 14; i++) {
        mvprintw(i, 5,  "|                                   |");
    }
    mvprintw(15, 5, "+-----------------------------------+");
    mvprintw(2, 18, " YOKE (PITCH/BANK) ");

    mvprintw(3, 45, "+-------+");
    for(int i = 4; i <= 14; i++) {
        mvprintw(i, 45, "|       |");
    }
    mvprintw(15, 45,"+-------+");
    mvprintw(2, 45, " THROTTLE ");
}

void render_dashboard(void) {
    clear();

    // Telemetry Dashboard
    attron(A_BOLD);
    mvprintw(0, 2,  "ALTITUDE: %05d FT", (int) altitude);
    mvprintw(0, 25, "RPM: %04d", (int) rpm);
    mvprintw(0, 40, "BANK: %03d DEG", (int) bank_angle);
    mvprintw(0, 58, "PITCH: %02d DEG", (int) pitch_angle);
    attroff(A_BOLD);

    draw_panel_borders();

    // Yoke visualizer
    int yoke_center_x = 22;
    int yoke_center_y = 9;
    int yoke_x = yoke_center_x + (bank_angle / 3);
    int yoke_y = yoke_center_y - (pitch_angle / 4);

    if (bank_angle <= -15) {
        mvprintw(yoke_y, yoke_x - 3, "//=[O]=//");
    } else if (bank_angle >= 15) {
        mvprintw(yoke_y, yoke_x - 3, "\\\\=[O]=\\\\");
    } else {
        mvprintw(yoke_y, yoke_x - 3, "==[O]==");
    }

    // Throttle visualizer
    int track_y_start = 4;
    int track_length = 10;
    int throttle_pos = 14 - (throttle_pct / 10);

    for (int i = 0; i <= track_length; i++) {
        mvaddch(track_y_start + i, 49, '|');
    }
    mvprintw(throttle_pos, 47, "[==]"); 

    // Footer
    mvprintw(17, 2, "CONTROLS: [ARROWS] Move Yoke | [W/S] Throttle Up/Down | [Q] Quit");

    refresh();
}