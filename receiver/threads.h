#ifndef THREADS_H
#define THREADS_H

void* thread_altimeter(void* arg);
void* thread_rpm(void* arg);
void* thread_fuel(void* arg);
void* thread_heading(void* arg);
void* thread_logger(void* arg);

#endif // THREADS_H