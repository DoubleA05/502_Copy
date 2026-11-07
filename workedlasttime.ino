#include <Servo.h>

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

Servo servoMotor;

int baseSpeed = 55;
int turnSpeed = 80;
bool inMaze = false;
unsigned long mazeStartTime = 0;
bool mazeTransitionStarted = false;

// -------------------- SERVO --------------------
void servoSlowWrite(int target){
  int current = servoMotor.read();
  if(current < target) {
    for(int i=current;i<=target;i++){
      servoMotor.write(i);
      delay(7);
    }
  } else {
    for(int i=current;i>=target;i--){
      servoMotor.write(i);
      delay(7);
    }
  }
}

// -------------------- ULTRASONIC --------------------
long getDistance(){
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin,HIGH,30000);
  long d = duration*0.034/2;
  if(d==0||d>350) return 350;
  return d;
}

long filteredDistance(){
  long d1=getDistance(); delay(20);
  long d2=getDistance(); delay(20);
  long d3=getDistance(); 
  return min(d1,min(d2,d3));
}

// -------------------- MOTORS --------------------
void moveForward(int spd){
  digitalWrite(STBY,HIGH);
  digitalWrite(AIN_1,HIGH);
  digitalWrite(BIN_1,HIGH);
  analogWrite(ENA,spd);
  analogWrite(ENB,spd);
}

void stopCar(){
  analogWrite(ENA,0);
  analogWrite(ENB,0);
  digitalWrite(STBY,LOW);
}

void turnLeft90(){
  stopCar(); 
  delay(100); 
  digitalWrite(STBY,HIGH); 
  digitalWrite(AIN_1,LOW); 
  digitalWrite(BIN_1,HIGH); 
  analogWrite(ENA,turnSpeed); 
  analogWrite(ENB,turnSpeed); 
  delay(260); 
  stopCar();
}

void turnRight90(){
  stopCar(); 
  delay(100); 
  digitalWrite(STBY,HIGH); 
  digitalWrite(AIN_1,HIGH); 
  digitalWrite(BIN_1,LOW); 
  analogWrite(ENA,turnSpeed); 
  analogWrite(ENB,turnSpeed); 
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

  // Define threshold for black/white detection (adjust based on your sensors)
  int threshold = 500; // Typical value for analog sensors - adjust as needed

  bool L_black = L > threshold;
  bool C_black = C > threshold;
  bool R_black = R > threshold;

  // Debug output
  Serial.print("L=");
  Serial.print(L);
  Serial.print(" C=");
  Serial.print(C);
  Serial.print(" R=");
  Serial.println(R);

  // Centered on line
  if (C_black && !L_black && !R_black) {
    moveForward(baseSpeed);
  } 
  // Need to go right (line is to the left)
  else if (L_black && !C_black && !R_black) {
    right();
  } 
  // Need to go left (line is to the right)
  else if (R_black && !C_black && !L_black) {
    left();
  } 
  // All sensors see black - this triggers maze transition
  else if (L_black && C_black && R_black) {
    if (!mazeTransitionStarted) {
      stopCar();
      mazeTransitionStarted = true;
      mazeStartTime = millis();
      Serial.println("Black line detected - starting 3 second countdown");
    }
    
    // Wait for 3 seconds
    if (millis() - mazeStartTime >= 3000) {
      inMaze = true;
      mazeTransitionStarted = false;
      Serial.println("Transitioning to maze navigation");
      moveForward(baseSpeed); // Start moving into maze
    }
  }
  // No line detected - stop and wait
  else if (!L_black && !C_black && !R_black) {
    stopCar();
  }
  // Otherwise continue forward (for cases like partial detection)
  else {
    moveForward(baseSpeed);
  }
}

// -------------------- MAZE NAVIGATION --------------------
void mazeNavigation(){
  // Scan front and left
  servoSlowWrite(90);
  long frontDist = filteredDistance();
  servoSlowWrite(135);
  long leftDist = filteredDistance();
  servoSlowWrite(90); // Return to center

  bool frontBlocked = frontDist < 18;
  bool leftOpen = leftDist > 22;

  Serial.print("Front: ");
  Serial.print(frontDist);
  Serial.print("cm, Left: ");
  Serial.print(leftDist);
  Serial.println("cm");

  if(leftOpen && frontBlocked){ // Left path open and front blocked
    stopCar(); delay(150);
    turnLeft90();
    return;
  }

  if(frontBlocked){ // Obstacle in front
    stopCar(); delay(150);
    turnLeft90(); // Turn left if blocked
    return;
  }

  // Dead end? walls on front and left
  if(!leftOpen && frontDist < 18){
    stopCar(); delay(150);
    turnRight90();
    return;
  }

  // Otherwise keep moving forward
  moveForward(baseSpeed);
}

// -------------------- SETUP --------------------
void setup(){
  Serial.begin(9600);
  pinMode(ENA,OUTPUT); 
  pinMode(ENB,OUTPUT); 
  pinMode(AIN_1,OUTPUT); 
  pinMode(BIN_1,OUTPUT); 
  pinMode(STBY,OUTPUT);
  pinMode(trigPin,OUTPUT); 
  pinMode(echoPin,INPUT);
  servoMotor.attach(SERVO_PIN); 
  servoMotor.write(90);

  inMaze = false; 
  mazeTransitionStarted = false;
  delay(1000);
  Serial.println("Robot started - Line following mode");
}

// -------------------- LOOP --------------------
void loop(){
  if(!inMaze) {
    lineTrack();
  } else {
    mazeNavigation();
  }
}
