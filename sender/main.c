#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "../common/arinc_429.h"
#include "../common/udp_sender.h"

#include "flight_state.h"
#include "physics.h"
#include "render.h"

#define TARGET_IP "127.0.0.1"
#define TARGET_PORT 9999

// Mutex to protect shared flight state variables
pthread_mutex_t flight_data_mutex = PTHREAD_MUTEX_INITIALIZER;

// Control flag for the background thread
volatile int keep_running = 1;

void* arinc_sender_thread(void* arg)
{
    udp_sender_t* sender = (udp_sender_t*)arg;
    
    double local_altitude; 
    double local_rpm; 
    double local_bank_angle;

    while (keep_running) {
 
        // 1. Lock the mutex and copy variables to minimize lock hold time
        pthread_mutex_lock(&flight_data_mutex);
        local_altitude = altitude;
        local_rpm = rpm;
        local_bank_angle = bank_angle;
        pthread_mutex_unlock(&flight_data_mutex);

        // 2. Send out the Altitude
        uint32_t alt_word = encode_altitude(local_altitude, 1, SSM_NORMAL_OPERATION);
        udp_sender_send(sender, alt_word);

        // 3. Send out the RPM
        uint32_t rpm_word = encode_engine_rpm(local_rpm, 1, SSM_NORMAL_OPERATION);
        udp_sender_send(sender, rpm_word);

        // 4. Send out the Bank Angle
        uint32_t bank_angle_word = encode_bank_angle(local_bank_angle, 1, SSM_NORMAL_OPERATION);
        udp_sender_send(sender, bank_angle_word);

         usleep(50000);

    }

    return NULL;
}


int main() {
    initscr();
    cbreak();             
    noecho();             
    keypad(stdscr, TRUE); 
    nodelay(stdscr, TRUE); 
    curs_set(0);          

    udp_sender_t sender;
    pthread_t sender_thread_id;

    srand(time(NULL));

    if (udp_sender_init(&sender, TARGET_IP, TARGET_PORT) != 0) {
        endwin();
        fprintf(stderr, "Error: Failed to initialize UDP socket.\n");
        return EXIT_FAILURE;
    }

    // Start the background ARINC UDP sender thread
    if (pthread_create(&sender_thread_id, NULL, arinc_sender_thread, &sender) != 0) {
        endwin();
        fprintf(stderr, "Error: Failed to create sender thread.\n");
        return EXIT_FAILURE;
    }

    int ch;
    
    while (keep_running) {

        ch = getch();
        
        // Lock physics and state updates so the sender thread reads a consistent state
        pthread_mutex_lock(&flight_data_mutex);

        if (ch == 'q' || ch == 'Q') {
            keep_running = 0; // Signal background thread to stop
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

        if (keep_running) {
            update_physics();
            render_dashboard();
        }

        pthread_mutex_unlock(&flight_data_mutex);

        // Main thread UI update rate
        usleep(50000);
    }

    // Wait for the background thread to finish cleanly
    pthread_join(sender_thread_id, NULL);

    udp_sender_close(&sender);
    pthread_mutex_destroy(&flight_data_mutex);

    endwin();
    return 0;
}