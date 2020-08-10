#include "radar.c.h"

void setup (void) {
    Serial.begin(9600);
    radar_setup();
    Serial.println("=== RADAR ===");
}

void loop (void) {
    static int zero = false;
    Radar_S s;
    int vel = radar_read(&s);
    if (vel == 0) {
        if (!zero) {
            Serial.println("========");
        }
        zero = true;
    } else {
        zero = false;
        //Serial.print((vel > 0) ? "->" : "<-");
        //Serial.print(' ');
        //Serial.println(abs(vel));
        show(&s);
    }
}
