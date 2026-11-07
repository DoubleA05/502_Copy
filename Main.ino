#include "Config.h"
#include "MotorDriver.h"
#include "LineTracker.h"
#include "AsyncTimer.h"
#include <util/delay.h>
#include <avr/io.h>


ControlMotor motor;
AsyncTimer1 Timer1;
AsyncTimer2 Timer2;

bool LineBCheck = false;
bool lineTrackingDone = false;
bool delayDone = false;

void setup() {
    motor.init();

    pinMode(LEFT_SENSOR, INPUT);
    pinMode(CENTER_SENSOR, INPUT);
    pinMode(RIGHT_SENSOR, INPUT);

    Timer1.start();
}

void loop() {
  Timer1.update();
    Timer2.update();

    if (!lineTrackingDone) {

        if (Timer1.timertime <= 2000) {
            trackLine1();
        } 
        
        else {
            trackLine2(lineTrackingDone);
        }
    } 
    else {
        
        if (!delayDone) {
            motor.stop();
            _delay_ms(3000);
            delayDone = true;
        }
        // Maze navigation code goes here
    }
}
