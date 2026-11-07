#include <Servo.h>

// -------------------- PIN DEFINES --------------------
#define ENA 5
#define ENB 6
#define BIN_1 7
#define AIN_1 8
#define STBY 3

#define leftSensor   A0
#define centerSensor A1
#define rightSensor  A2

#define trigPin 13
#define echoPin 12
#define SERVO_PIN 10

// -------------------- GLOBALS --------------------
Servo servoMotor;

int baseSpeed = 85;
int turnSpeed = 140;

bool inMaze = false;
bool climbRamp = false;
bool mazeTransitionStarted = false;

unsigned long rampStart = 0;

// -------------------- SERVO --------------------
void servoSlowWrite(int target){
  servoMotor.write(target);
  delay(120);
}

// -------------------- ULTRASONIC --------------------
long getDistance(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  long d = duration * 0.034 / 2;

  if (d == 0 || d > 350) return 350;
  return d;
}

long filteredDistance(){
  long d1 = getDistance(); delay(10);
  long d2 = getDistance(); delay(10);
  long d3 = getDistance();
  return min(d1, min(d2, d3));
}

// -------------------- MOTOR CONTROL --------------------
void moveForward(int spd){
  digitalWrite(STBY, HIGH);
  digitalWrite(AIN_1, HIGH);
  digitalWrite(BIN_1, HIGH);
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
}

void stopCar(){
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(STBY, LOW);
}

void turnLeft90(){
  stopCar(); delay(80);
  digitalWrite(STBY, HIGH);
  digitalWrite(AIN_1, LOW);
  digitalWrite(BIN_1, HIGH);
  analogWrite(ENA, turnSpeed);
  analogWrite(ENB, turnSpeed);
  delay(240);
  stopCar();
}

void turnRight90(){
  stopCar(); delay(80);
  digitalWrite(STBY, HIGH);
  digitalWrite(AIN_1, HIGH);
  digitalWrite(BIN_1, LOW);
  analogWrite(ENA, turnSpeed);
  analogWrite(ENB, turnSpeed);
  delay(240);
  stopCar();
}

void right(){
  digitalWrite(STBY, HIGH);
  digitalWrite(AIN_1, HIGH);
  digitalWrite(BIN_1, LOW);
  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, baseSpeed);
}

void left(){
  digitalWrite(STBY, HIGH);
  digitalWrite(AIN_1, LOW);
  digitalWrite(BIN_1, HIGH);
  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, baseSpeed);
}

// -------------------- END-OF-LINE DETECTOR --------------------
bool isEndOfLine(int L, int C, int R) {
  // matches your real readings:
  // L ~700, C ~450, R ~600 at the T-bar
  return (L > 600 && R > 500 && C > 300);
}

// -------------------- LINE TRACKING --------------------
void lineTrack() {
  int L = analogRead(leftSensor);
  int C = analogRead(centerSensor);
  int R = analogRead(rightSensor);


  int threshold = 300; //based on my desk white/black readings

  // -------------------------------
  // END OF LINE
  // -------------------------------
  if (isEndOfLine(L, C, R)) {

    static unsigned long tStart = 0;

    if (!mazeTransitionStarted) {
      mazeTransitionStarted = true;
      tStart = millis();
      stopCar();
      Serial.println("POSSIBLE END...");
    }

    if (millis() - tStart >= 350) {
      stopCar();
      Serial.println("END CONFIRMED → STOP 3s");

      delay(3000); // required 3-second stop

      // Move into ramp → maze
      climbRamp = true;
      rampStart = millis();
      mazeTransitionStarted = false;

      moveForward(baseSpeed + 25);
      return;
    }
  }

  else {
    mazeTransitionStarted = false;
  }

  // -------------------------------
  // NORMAL LINE TRACKING
  // -------------------------------
  bool L_black = L > threshold;
  bool C_black = C > threshold;
  bool R_black = R > threshold;

  if (C_black && !L_black && !R_black) moveForward(baseSpeed);
  else if (L_black && !C_black) right();
  else if (R_black && !C_black) left();
  else moveForward(baseSpeed);
}


// -------------------- MAZE NAVIGATION (LONGEST PATH) --------------------
void mazeNavigation() {

  long emergency = filteredDistance();
  if (emergency < 15) {
    stopCar();
    delay(120);
  }

  // Scan front
  servoSlowWrite(90);
  long frontDist = filteredDistance();

  // Scan left
  servoSlowWrite(135);
  long leftDist = filteredDistance();

  // Scan right
  servoSlowWrite(45);
  long rightDist = filteredDistance();

  servoSlowWrite(90);

  // Decide longest path
  long biggest = frontDist;
  String dir = "front";

  if (leftDist > biggest) { biggest = leftDist; dir = "left"; }
  if (rightDist > biggest){ biggest = rightDist; dir = "right"; }

  if (dir == "left") {
    Serial.println(">>> Turn LEFT (longest path)");
    turnLeft90();
  }
  else if (dir == "right") {
    Serial.println(">>> Turn RIGHT (longest path)");
    turnRight90();
  }
  else {
    Serial.println(">>> FORWARD");
  }

  // Move forward safely
  unsigned long start = millis();
  while (millis() - start < 400) {
    long d = filteredDistance();
    if (d < 18) {
      stopCar();
      Serial.println(">>> WALL TOO CLOSE — RESCAN");
      delay(120);
      return;
    }
    moveForward(baseSpeed);
  }

  stopCar();
  delay(80);
}

// -------------------- SETUP --------------------
void setup(){
  Serial.begin(9600);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(AIN_1, OUTPUT);
  pinMode(BIN_1, OUTPUT);
  pinMode(STBY, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  servoMotor.attach(SERVO_PIN);
  servoMotor.write(90);

  Serial.println("Robot Started → LINE TRACKING MODE");
  delay(1000);
}

// -------------------- LOOP --------------------
void loop(){

  // 1) LINE TRACKING
  if (!inMaze && !climbRamp){
    lineTrack();
  }

  // 2) RAMP (≈1.2s)
  else if (climbRamp && !inMaze){

    if (millis() - rampStart < 1200){
      moveForward(baseSpeed + 25);
    }
    else {
      climbRamp = false;
      inMaze = true;
      Serial.println("ENTERING MAZE MODE");
    }
  }

  // 3) MAZE
  else if (inMaze){
    mazeNavigation();
  }
}

// -------------------- LOOP --------------------
void loop(){
  mazeNavigation();
}
