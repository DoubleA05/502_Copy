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

int baseSpeed = 60;
int turnSpeed = 150;

bool inMaze = false;
bool climbRamp = false;
bool mazeTransitionStarted = false;

unsigned long rampStart = 0;

// -------------------- SERVO --------------------
void servoSlowWrite(int target){
  servoMotor.write(target);
  delay(200);   // slower, more stable scanning
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
  delay(260);
  stopCar();
}

void turnRight90(){
  stopCar(); delay(80);
  digitalWrite(STBY, HIGH);
  digitalWrite(AIN_1, HIGH);
  digitalWrite(BIN_1, LOW);
  analogWrite(ENA, turnSpeed);
  analogWrite(ENB, turnSpeed);
  delay(260);
  stopCar();
}

void right(){
  digitalWrite(STBY, HIGH);
  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, baseSpeed);
  digitalWrite(AIN_1, HIGH);
  digitalWrite(BIN_1, LOW);
}

void left(){
  digitalWrite(STBY, HIGH);
  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, baseSpeed);
  digitalWrite(AIN_1, LOW);
  digitalWrite(BIN_1, HIGH);
}

// -------------------- LINE TRACKING --------------------
void lineTrack(){
  int L = analogRead(leftSensor);
  int C = analogRead(centerSensor);
  int R = analogRead(rightSensor);

  int threshold = 700;

  bool L_black = L > threshold;
  bool C_black = C > threshold;
  bool R_black = R > threshold;

  if (C_black && !L_black && !R_black) moveForward(baseSpeed);
  else if (L_black && !C_black && !R_black) right();
  else if (R_black && !C_black && !L_black) left();

  // ----------- Detect Line B (ALL BLACK) -----------
  else if (L_black && C_black && R_black) {

    static unsigned long blackStart = 0;

    // First detection
    if (!mazeTransitionStarted) {
      mazeTransitionStarted = true;
      blackStart = millis();
      stopCar();
      Serial.println("Line B detected - holding...");
    }

    stopCar();  // hold still

    // Require stable all-black for 500 ms
    if (millis() - blackStart >= 500) {

      Serial.println("LINE B CONFIRMED → FULL STOP 3s");
      stopCar();
      delay(3000);   // FULL STOP

      // Begin ramp
      climbRamp = true;
      rampStart = millis();
      mazeTransitionStarted = false;

      moveForward(baseSpeed + 20);
      return;
    }
  }

  else {
    mazeTransitionStarted = false;
    moveForward(baseSpeed);
  }
}

// -------------------- LONGEST PATH MAZE NAVIGATION --------------------
void mazeNavigation() {

  // --- LIVE EMERGENCY STOP ---
  long liveDist = filteredDistance();
  if (liveDist < 15) {
    stopCar();
    Serial.println("EMERGENCY STOP: Wall too close!");
    delay(150);
  }

  // --- SCAN FRONT ---
  servoSlowWrite(90);
  long frontDist = filteredDistance();

  // --- SCAN LEFT (wider) ---
  servoSlowWrite(150);
  long leftDist = filteredDistance();

  // --- SCAN RIGHT (wider) ---
  servoSlowWrite(30);
  long rightDist = filteredDistance();

  // Return to center
  servoSlowWrite(90);

  Serial.print("Front: "); Serial.print(frontDist);
  Serial.print(" | Left: "); Serial.print(leftDist);
  Serial.print(" | Right: "); Serial.println(rightDist);

  // --- Choose the longest path ---
  long biggest = frontDist;
  String direction = "front";

  if (leftDist > biggest) {
    biggest = leftDist;
    direction = "left";
  }
  if (rightDist > biggest) {
    biggest = rightDist;
    direction = "right";
  }

  // --- Turn accordingly ---
  if (direction == "left") {
    Serial.println(">>> Turning LEFT (longest path)");
    stopCar(); delay(150);
    turnLeft90();
  }
  else if (direction == "right") {
    Serial.println(">>> Turning RIGHT (longest path)");
    stopCar(); delay(150);
    turnRight90();
  }
  else {
    Serial.println(">>> Going FORWARD");
  }

  // --- SAFETY FORWARD MOTION ---
  unsigned long moveStart = millis();
  while (millis() - moveStart < 450) {

    long d = filteredDistance();
    if (d < 18) {
      stopCar();
      Serial.println(">>> WALL DURING MOTION — RESCANNING");
      delay(150);
      return;
    }

    moveForward(baseSpeed);
  }

  stopCar();
  delay(100);
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

  pinMode(leftSensor, INPUT);
  pinMode(centerSensor, INPUT);
  pinMode(rightSensor, INPUT);

  servoMotor.attach(SERVO_PIN);
  servoMotor.write(90);

  Serial.println("Robot Started → LINE TRACKING MODE");
  delay(1000);
}

// -------------------- LOOP --------------------
void loop(){

  // 1) LINE FOLLOWING MODE
  if (!inMaze && !climbRamp) {
    lineTrack();
  }

  // 2) RAMP CLIMB MODE
  else if (climbRamp && !inMaze) {

    if (millis() - rampStart < 1200) {
      moveForward(baseSpeed + 20);
    }
    else {
      climbRamp = false;
      inMaze = true;
      Serial.println("ENTERING MAZE MODE");
    }
  }

  // 3) MAZE MODE
  else if (inMaze) {
    mazeNavigation();
  }
}

