#include <Wire.h>
#include <SoftwareSerial.h>
#include <MeMCore.h>

//Motors, LED, LineSensor, UltrasonicSensor, LightSensor, LED, Buzzer
MeDCMotor MotorL(M1);
MeDCMotor MotorR(M2);
MeLineFollower LineSensor(PORT_2);
MeUltrasonicSensor UltrasonicSensor(PORT_3);
MeLightSensor LightSensor(PORT_6);
MeRGBLed LED(PORT_7);
MeBuzzer Buzzer(8);

//PORT_4 of mCore IR sensors
#define IR1 A0  // Left
#define IR2 A1  // Right

//Movement variables
int moveSpeed = 200;
int minSpeed = 48;

//Motor Functions
void Forward(){
  MotorL.run(-moveSpeed*1.09);
  MotorR.run(moveSpeed);
}

void Backward(){
  MotorL.run(moveSpeed); 
  MotorR.run(-moveSpeed);
}

void TurnLeft(){
  MotorL.run(moveSpeed);
  MotorR.run(moveSpeed);
}

void TurnRight(){
  MotorL.run(-moveSpeed);
  MotorR.run(-moveSpeed);
}

void Stop(){
  MotorL.run(0);
  MotorR.run(0);
}

//Colour Arrays
float colourArray[] = {0,0,0};
float whiteArray[] = {420,325,400};  //lab conditions for WHITE
float blackArray[] = {30,30,30};   //lab conditions for BLACK
float greyDiff[] = {390,295,370};  //difference between white and black array
float rgbArr[3][3] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};

//DELAYS
#define RGBWAIT 100
#define LDRWAIT 10

int getAvgReading(int times){   //average reading from LDR
  int reading;
  int total = 0;
  for(int i = 0;i < times;i++){
     reading = LightSensor.read();
     total = reading + total;
     delay(LDRWAIT);
  }
  return total/times;
}

int detect_colour(){  //maps colours to integer values
  for(int i = 0; i < 3; i++){
    LED.setColor(rgbArr[i][0],rgbArr[i][1],rgbArr[i][2]);
    LED.show();
    for(int j=0; j < 20; j++){ //value stabalised
      colourArray[i] = (LightSensor.read()-blackArray[i])/(greyDiff[i])*255;
      delay(10);
    }
    LED.setColor(0,0,0);
    LED.show();
    Serial.println(colourArray[i]);  //used to determine thresholds for each colour
  }
  if (colourArray[0] > 140 && colourArray[1] < 70 && colourArray[2] < 70){ //RGB thresholds for RED
    Serial.println("RED DETECTED!");
    return 0;
  }
  else if (colourArray[0] < 70 && colourArray[1] >100 && colourArray[2] < 70){ //RGB thresholds for GREEN
    Serial.println("GREEN DETECTED!");
    return 1;
  }
  else if (colourArray[0] > 200 && colourArray[1] > 200 && colourArray[2] > 200){ // NO COLOUR DETECTED //CHECK BEFORE BLUE AS THRESHOLD FOR BLUE COINCIDE
    return 6;
  }
  else if (colourArray[0] > 100 && colourArray[1] >150 && colourArray[2] > 150){ //RGB thresholds for BLUE
    Serial.println("BLUE DETECTED!");
    return 2;
  }
  else if (colourArray[0] > 200 && colourArray[1] > 130 && colourArray[2] < 100){//RGB thresholds for YELLOW
    Serial.println("YELLOW DETECTED!");
    return 3;
  }
  else if (colourArray[0] > 100 && colourArray[1] > 100 && colourArray[2] > 150){//RGB thresholds for PURPLE
    Serial.println("PURPLE DETECTED!");
    return 4;
  }
  else if (colourArray[0] < 50 && colourArray[1] < 50 && colourArray[2] < 50){//RGB thresholds for BLACK
    Serial.println("BLACK DETECTED!");
    return 5;
  }
  else{ //SAFETY NET to try again
    return 6;
  }
}


// frequencies for required notes
#define NOTE_B4  494
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_FS5 740
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_B5  988
#define REST      0

void celebrate(){ //TAKE ON ME melody
  int melody[] = {
    NOTE_FS5, NOTE_FS5, NOTE_D5, NOTE_B4, NOTE_B4, NOTE_E5, 
    NOTE_E5, NOTE_E5, NOTE_GS5, NOTE_GS5, NOTE_A5, NOTE_B5, 
    NOTE_A5, NOTE_A5, NOTE_A5, NOTE_E5, NOTE_D5, NOTE_FS5, 
    NOTE_FS5, NOTE_FS5, NOTE_E5, NOTE_E5, NOTE_FS5, NOTE_E5
  };
  int durations[] = {
    8, 8, 8, 4, 4, 4, 
    4, 5, 8, 8, 8, 8, 
    8, 8, 8, 4, 4, 4, 
    4, 5, 8, 8, 8, 8
  };
  int songLength = sizeof(melody)/sizeof(melody[0]);
  for (int thisNote = 0; thisNote < songLength; thisNote++){
    int duration = 1000/ durations[thisNote];
    Buzzer.tone(8, melody[thisNote], duration);
    int pause = duration * 1.3;
    delay(pause);
    Buzzer.noTone(8);
  }
}


void setup() {
  Serial.begin(9600);
}

void loop() {
  int sensor_state = LineSensor.readSensors();
  int front_distance = UltrasonicSensor.distanceCm();
  int left_distance = analogRead(IR1);
  int right_distance = analogRead(IR2);
  if (sensor_state < 3 || front_distance <= 7){ //black line or wall
    Stop();
    int cmd = detect_colour();  //determine colour and perform corresponding turn
    if (cmd == 0){ //RED
      TurnLeft();
      delay(300);
      Stop();
      delay(500);
    }
    else if (cmd == 1){ //GREEN
      TurnRight();
      delay(300);
      Stop();
      delay(500);
    }
    else if (cmd == 2){ //BLUE
      TurnRight();
      delay(300);
      Stop();
      delay(500);
      if(UltrasonicSensor.distanceCm() <= 35) { //explained in report
        while(1) {
          if (UltrasonicSensor.distanceCm() < 9) {
            Stop();
            break;
          } else {
            Forward();
          }
         }
        } else {
        Forward();
        delay(700);
      }
      Stop();
      delay(500);
      TurnRight();
      delay(300);
      Stop();
      delay(500);
    }
    else if (cmd == 3){ //YELLOW
      Stop();
      delay(300);
      Serial.println(analogRead(IR2));
      if (analogRead(IR2) > 750){ //there is space to pivot right
        TurnRight();
        delay(300);
        Stop();
        delay(500);
        TurnRight();
        delay(300);
        Stop();
        delay(500);
      }
      else if (analogRead(IR1) > 610){ //there is space to pivot left
        TurnLeft();
        delay(300);
        Stop();
        delay(500);
        TurnLeft();
        delay(300);
        Stop();
        delay(500);
      }
      else{  //worst case scenario just turn right
        TurnRight();
        delay(300);
        Stop();
        delay(500);
        TurnRight();
        delay(300);
        Stop();
        delay(500);
      }
    }
    else if (cmd == 4){ //PURPLE
      TurnLeft();
      delay(300);
      Stop();
      delay(500);
      if(UltrasonicSensor.distanceCm() <= 35) {
        while(1) {
          if (UltrasonicSensor.distanceCm() < 9) {
            Stop();
            break;
          } else {
            Forward();
          }
         }
        } else {
        Forward();
        delay(700);
      }
      Stop();
      delay(500);
      TurnLeft();
      delay(300);
      Stop();
      delay(500);
    }
    else if (cmd == 5){ //BLACK
      Stop();
      celebrate();
      delay(5000);
    }
    else if (cmd == 6){ //try again
      Stop();
      delay(500);
    }
  }
  else{
    Forward();
    if (left_distance < 590){ //around 3 cm away from left wall
      Serial.println(left_distance);
      Serial.println("Turn right");
      TurnRight();
      delay(15);
    }
    if (right_distance < 600){ //around 3 cm away from right wall
      Serial.println(right_distance);
      Serial.println("Turn left");
      TurnLeft();
      delay(15);
    }
  }
}
