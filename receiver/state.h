#ifndef STATE_H
#define STATE_H

#include <pthread.h>

typedef struct {
    int altitude;
    int rpm;
    float fuel;
    float heading;
} FlightData;

// Extern declarations mean "these exist somewhere, trust me"
extern FlightData fd;
extern pthread_mutex_t screen_mutex;
extern pthread_mutex_t data_mutex;
extern volatile int keep_running;

#endif // STATE_H