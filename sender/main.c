#include <ncurses.h>
#include <unistd.h>
#include "flight_state.h"
#include "physics.h"
#include "render.h"

int main() {
    initscr();
    cbreak();             
    noecho();             
    keypad(stdscr, TRUE); 
    nodelay(stdscr, TRUE); 
    curs_set(0);          

    int ch;
    
    while (1) {
        ch = getch();
        if (ch == 'q' || ch == 'Q') {
            break;
        } 
        else if (ch == KEY_UP) {
            pitch_angle -= 2; 
            if (pitch_angle < -20) pitch_angle = -20;
        } 
        else if (ch == KEY_DOWN) {
            pitch_angle += 2; 
            if (pitch_angle > 20) pitch_angle = 20;
        } 
        else if (ch == KEY_LEFT) {
            bank_angle -= 5;  
            if (bank_angle < -45) bank_angle = -45;
        } 
        else if (ch == KEY_RIGHT) {
            bank_angle += 5;  
            if (bank_angle > 45) bank_angle = 45;
        } 
        else if (ch == 'w' || ch == 'W') {
            throttle_pct += 5; 
            if (throttle_pct > 100) throttle_pct = 100;
        } 
        else if (ch == 's' || ch == 'S') {
            throttle_pct -= 5; 
            if (throttle_pct < 0) throttle_pct = 0;
        }

        update_physics();
        render_dashboard();

        usleep(50000); 
    }

    endwin();
    return 0;
}