#include <errno.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>

#include "../common/arinc_429.h"
#include "threads.h"
#include "state.h"

#define BUF_SIZE 1024
#define TARGET_IP "127.0.0.1"
#define TARGET_PORT 9999


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
        double current_alt = fd.altitude;
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
        double base_rpm = fd.rpm;
        pthread_mutex_unlock(&data_mutex);
        
        pthread_mutex_lock(&screen_mutex);
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 0, 2, " ENGINE ");
        mvwprintw(win, 2, 2, "RPM: %04d", (int) base_rpm);
        
        int bars = (int)(base_rpm / 280.0); // The aircraft has a max of 2800 RPM
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
        mvwprintw(win, 2, 2, "%05.1f%%", current_fuel);
        
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
        double current_hdg = fd.bank_angle;
        pthread_mutex_unlock(&data_mutex);

        pthread_mutex_lock(&screen_mutex);
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 0, 2, " BANK ANGLE ");
        mvwprintw(win, 2, 4, "%lf DEG", current_hdg);
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
        pthread_mutex_lock(&screen_mutex);
        box(win, 0, 0); 
        mvwprintw(win, 0, 2, " ARINC 429 LOG ");
        wrefresh(win);
        wrefresh(sub_win);
        // Loop through all new messages in the queue
        while (log_tail != log_head) {
            arinc429_word_t current_word = log_queue[log_tail];
            log_tail = (log_tail + 1) % LOG_QUEUE_SIZE; // Advance tail
            wprintw(sub_win, "RAW: 0x%08X Label: 0x%02X\n", current_word.raw, current_word.fields.label);
            wprintw(sub_win, "  Data: %-8u\n", current_word.fields.data);
            wprintw(sub_win, "    SDI: %-5u SSM: %-6u\n", current_word.fields.sdi, current_word.fields.ssm);

        }
        wrefresh(sub_win);
        pthread_mutex_unlock(&screen_mutex);
        pthread_mutex_unlock(&data_mutex);

        usleep(100000); 
    }
    delwin(sub_win); 
    return NULL;
}


void* thread_receive_429_udp(void* arg) {
    (void)arg;
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUF_SIZE];

    // 1. Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        exit(EXIT_FAILURE);
    }

    // Zero out the socket address structures
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // 2. Configure the server address (localhost:9999)
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(TARGET_IP);
    server_addr.sin_port = htons(TARGET_PORT);

    // 3. Bind the socket to the address and port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (keep_running) {
        arinc429_word_t word;

        int n = recvfrom(sockfd, buffer, BUF_SIZE, MSG_DONTWAIT,
                         (struct sockaddr *)&client_addr, &client_len);

        if (n < 0) {
            // Check if the error is just "no data available right now"
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Adding a small sleep prevents this tight while(1) loop 
                // from consuming 100% of your CPU core while waiting.
                // 10,000 microseconds = 10 milliseconds
                usleep(10000); 
                continue;
            } else {
                // A genuine network/socket error occurred
                usleep(100000); // Sleep briefly before retrying to avoid log spam on critical failure
                continue;
            }
        }

        // Check if the received packet has at least 4 bytes for a 32-bit word
        if ((unsigned long)n >= sizeof(uint32_t)) {

            uint32_t raw_data;

            // Safely copy the first 4 bytes from the buffer into a 32-bit integer
            memcpy(&raw_data, buffer, sizeof(uint32_t));

            // Assign to the union. 
            word.raw = ntohl(raw_data);

            pthread_mutex_lock(&data_mutex);
                 // Add to queue
                log_queue[log_head].raw = word.raw;
                log_head = (log_head + 1) % LOG_QUEUE_SIZE; // Advance head, wrap around
                
                // 1. Extract the human-readable label from the word
                uint8_t label = extract_arinc_label(word.raw);
                DecodedArincWord decoded_word;
                switch (label)
                {
                    case LABEL_ALTITUDE : 
                        decoded_word = decode_altitude(word.raw);
                        fd.altitude = decoded_word.value;
                        break;
                    case LABEL_BANK_ANGLE : 
                        decoded_word = decode_bank_angle(word.raw);
                        fd.bank_angle = decoded_word.value;
                        break;
                   case LABEL_ENGINE_RPM : 
                        decoded_word = decode_engine_rpm(word.raw);
                        fd.rpm = decoded_word.value;
                        break;
                    default : break;
                }


            pthread_mutex_unlock(&data_mutex);
        }
    }

    close(sockfd);
    return NULL;
}

