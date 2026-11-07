#ifndef ASYNC_TIMER_H
#define ASYNC_TIMER_H

#include <Arduino.h>

// Global timer object
struct AsyncTimer1 {
    unsigned long timertime = 0;
    bool running = false;
    unsigned long startMillis = 0;


    void start() {
        startMillis = millis();
        timertime = 0;
        running = true;
    }


    void update() {
        if (running) {
            unsigned long elapsed = millis() - startMillis;
            if (elapsed < 2001) {
                timertime = elapsed;
            } else {
                timertime = 2001;
            }
        }
    }
};

struct AsyncTimer2 {
    unsigned long timertime = 0;
    bool running = false;
    unsigned long startMillis = 0;

    void start() {
        startMillis = millis();
        timertime = 0;
        running = true;
    }

    void update() {
        if (running) {
            unsigned long elapsed = millis() - startMillis;
            if (elapsed < 501) {
                timertime = elapsed;
            } else {
                timertime = 501;
            }
        }
    }
};

extern AsyncTimer1 Timer1;
extern AsyncTimer2 Timer2;


#endif
