#ifndef THREADS_H
#define THREADS_H

#define THREAD_COUNT 6

void* thread_altimeter(void* arg);
void* thread_rpm(void* arg);
void* thread_fuel(void* arg);
void* thread_heading(void* arg);
void* thread_logger(void* arg);
void* thread_receive_429_udp(void* arg);

#endif // THREADS_H