#ifndef LineTracker
#define LineTracker


#include "Config.h"
#include "MotorDriver.h"
#include <util/delay.h>
#include <avr/io.h>
#include "AsyncTimer.h"


extern bool LineBCheck;


void trackLine1() {
    int leftState = digitalRead(LEFT_SENSOR);
    int centerState = digitalRead(CENTER_SENSOR);
    int rightState = digitalRead(RIGHT_SENSOR);


    if (leftState == LOW && centerState == HIGH && rightState == LOW) {
        motor.forward();
    }
    else if (leftState == HIGH && centerState == LOW && rightState == LOW) {
        motor.turnRight();
    }
    else if ( leftState == LOW && centerState == LOW && rightState == HIGH) {
        motor.turnLeft();
    }
    else if (leftState == HIGH && centerState == HIGH && rightState == HIGH) {
        motor.stop();
    }
    else {
        motor.forward();
    }
}

void trackLine2(bool &lineTrackingDone) {
    int leftState = digitalRead(LEFT_SENSOR);
    int centerState = digitalRead(CENTER_SENSOR);
    int rightState = digitalRead(RIGHT_SENSOR);

    if (leftState == LOW && centerState == LOW && rightState == LOW) {
        if (!Timer2.running) {
            Timer2.start();
        }
        motor.forward();

        if (Timer2.timertime >= 500) {
            motor.stop();
            lineTrackingDone = true;
            Timer2.running = false;
            Timer2.timertime = 0;
            return;
        }
    }
    else {
        Timer2.running = false;
        Timer2.timertime = 0;

        if (leftState == LOW && centerState == HIGH && rightState == LOW) {
            motor.forward();
        }
        else if (leftState == HIGH && centerState == LOW && rightState == LOW) {
            motor.turnRight();
        }
        else if (leftState == LOW && centerState == LOW && rightState == HIGH) {
            motor.turnLeft();
        }
        else if (leftState == HIGH && centerState == HIGH && rightState == HIGH) {
            motor.stop();
        }
        else {
            motor.forward();
        }
    }
}


#endif




