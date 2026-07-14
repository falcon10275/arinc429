#include "physics.h"
#include "flight_state.h"

void update_physics(void) {
    // 1. Calculate RPM from Throttle
    if (throttle_pct == 0) {
        rpm = 0;
    } else {
        rpm = 800 + (throttle_pct * 20); 
    }

    // 2. Calculate Altitude based on Pitch and RPM
    float thrust_factor = (float)rpm / 2800.0; 
    float vertical_speed = (pitch_angle * thrust_factor) - (2.0 * (1.0 - thrust_factor));
    
    // Auto-leveling logic
    if (bank_angle > 0) bank_angle -= 1;
    if (bank_angle < 0) bank_angle += 1;

    altitude += (vertical_speed * 0.5); 
    if (altitude < 0) altitude = 0;     
}