#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <util/delay.h>

class ControlMotor {
public:
  void init();
  void setSpeed(int left, int right);
  void forward();
  void backward();
  void turnLeft();
  void turnRight();
  void stop();

private:
  void motorA(bool fwd, uint8_t speed);
  void motorB(bool fwd, uint8_t speed);
};

extern ControlMotor motor;

void ControlMotor::init() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
}

void ControlMotor::motorA(bool fwd, uint8_t speed) {
  digitalWrite(AIN1, fwd);
  digitalWrite(AIN2, !fwd);
  analogWrite(PWMA, speed);
}

void ControlMotor::motorB(bool fwd, uint8_t speed) {
  digitalWrite(BIN1, fwd);
  digitalWrite(BIN2, !fwd);
  analogWrite(PWMB, speed);
}

void ControlMotor::setSpeed(int left, int right) {
  motorA(true, left);
  motorB(true, right);
}

void ControlMotor::forward() {
  motorA(true, MOTOR_SPEED_MAX);
  motorB(true, MOTOR_SPEED_MAX);
}

void ControlMotor::backward() {
  motorA(false, MOTOR_SPEED_MAX);
  motorB(false, MOTOR_SPEED_MAX);
}

void ControlMotor::turnLeft() {
  motorA(true, MOTOR_SPEED_MAX);
  motorB(false, MOTOR_SPEED_MAX);
}

void ControlMotor::turnRight() {
  motorA(false, MOTOR_SPEED_MAX);
  motorB(true, MOTOR_SPEED_MAX);
}

void ControlMotor::stop() {
  motorA(true, 0);
  motorB(true, 0);
}

#endif
