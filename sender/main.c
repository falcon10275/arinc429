#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../common/arinc_429.h"
#include "../common/udp_sender.h"

#include "flight_state.h"
#include "physics.h"
#include "render.h"

#define TARGET_IP "127.0.0.1"
#define TARGET_PORT 9999


void send_arinc_data_over_udp(udp_sender_t* sender)
{
        arinc429_word_t arinc_word;
        
        // Send out the Altitude
        arinc_word.raw = 0;
        arinc429_set_label(&arinc_word, ARINC_ALTITUDE_LABEL);       
        arinc429_set_sdi(&arinc_word, 2);           
        arinc429_set_data(&arinc_word, altitude);     
        arinc429_set_ssm(&arinc_word, 3);           
        arinc429_set_parity(&arinc_word, 1);       

        // Send the raw 32-bit arinc packet
        udp_sender_send(sender, arinc_word.raw);

        // Send out the RPM
        arinc_word.raw = 0;
        arinc429_set_label(&arinc_word, ARINC_RPM_LABEL);       
        arinc429_set_sdi(&arinc_word, 2);           
        arinc429_set_data(&arinc_word, rpm);     
        arinc429_set_ssm(&arinc_word, 3);           
        arinc429_set_parity(&arinc_word, 1);       

        // Send the raw 32-bit arinc packet
        udp_sender_send(sender, arinc_word.raw);

        // Send out the RPM
        arinc_word.raw = 0;
        arinc429_set_label(&arinc_word, ARINC_BANK_ANGLE_LABEL);       
        arinc429_set_sdi(&arinc_word, 2);           
        arinc429_set_data(&arinc_word, bank_angle);     
        arinc429_set_ssm(&arinc_word, 3);           
        arinc429_set_parity(&arinc_word, 1);       

        // Send the raw 32-bit arinc packet
        udp_sender_send(sender, arinc_word.raw);
}


int main() {
    initscr();
    cbreak();             
    noecho();             
    keypad(stdscr, TRUE); 
    nodelay(stdscr, TRUE); 
    curs_set(0);          

    udp_sender_t sender;

    srand(time(NULL));

    if (udp_sender_init(&sender, TARGET_IP, TARGET_PORT) != 0) {
        fprintf(stderr, "Error: Failed to initialize UDP socket.\n");
        return EXIT_FAILURE;
    }

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
        send_arinc_data_over_udp(&sender);

        usleep(50000);


    }

    udp_sender_close(&sender);

    endwin();
    return 0;
}