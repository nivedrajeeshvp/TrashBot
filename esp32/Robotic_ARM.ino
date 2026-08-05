#include <ESP32Servo.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

// ===== PIN DEFINITIONS (UPDATED) =====
#define BASE_PIN      16
#define SHOULDER_PIN  17
#define ELBOW_PIN     18
#define GRIPPER_PIN   19

// create four servo objects
Servo shoulder;
Servo base;
Servo gripper;
Servo elbow;

// SG90 servo pulse width
int minUs = 1000;
int maxUs = 2000;

// Servo limits
int elbow_min = 0;
int elbow_max = 180;
int elbow_pos = 0;

int gripper_min = 0;
int gripper_max = 180;
int gripper_pos = 0;

int shoulder_min = 0;
int shoulder_max = 180;
int shoulder_pos = 0;

int base_min = 0;
int base_max = 180;
int base_pos = 0;

// Rover motor pins
int IN[] = {25, 26, 27, 13};

ESP32PWM pwm;

void setup()
{
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  Serial.begin(115200);

  shoulder.setPeriodHertz(50);
  base.setPeriodHertz(50);
  gripper.setPeriodHertz(50);
  elbow.setPeriodHertz(50);

  SerialBT.begin("Arm Robot");
  Serial.println("Bluetooth Started - Pair with Arm Robot");

  // Attach Servos
  shoulder.attach(SHOULDER_PIN, minUs, maxUs);
  base.attach(BASE_PIN, minUs, maxUs);
  gripper.attach(GRIPPER_PIN, minUs, maxUs);
  elbow.attach(ELBOW_PIN, minUs, maxUs);

  // Rover motor setup
  for(int i=0;i<4;i++){
    pinMode(IN[i], OUTPUT);
    digitalWrite(IN[i], LOW);
  }
}

void loop()
{
  if (SerialBT.available())
  {
    char rx = SerialBT.read();

    if (rx == 'S') rover_control("0000");
    else if (rx == 'R') rover_control("1000");
    else if (rx == 'Q') rover_control("0100");
    else if (rx == 'O') rover_control("0010");
    else if (rx == 'P') rover_control("0001");

    else if (rx == 'C') elbow_pos++;
    else if (rx == 'D') elbow_pos--;

    else if (rx == 'F') shoulder_pos++;
    else if (rx == 'E') shoulder_pos--;

    else if (rx == 'G') gripper_pos++;
    else if (rx == 'H') gripper_pos--;

    else if (rx == 'B') base_pos++;
    else if (rx == 'A') base_pos--;
  }

  // Limit checking
  elbow_pos = constrain(elbow_pos, elbow_min, elbow_max);
  shoulder_pos = constrain(shoulder_pos, shoulder_min, shoulder_max);
  gripper_pos = constrain(gripper_pos, gripper_min, gripper_max);
  base_pos = constrain(base_pos, base_min, base_max);

  // Write servo positions
  elbow.write(elbow_pos);
  shoulder.write(shoulder_pos);
  base.write(base_pos);
  gripper.write(gripper_pos);
}

void rover_control(String buttonstat)
{
  if (buttonstat[0] == '1')
  {
    digitalWrite(IN[0], HIGH);
    digitalWrite(IN[1], LOW);
    digitalWrite(IN[2], HIGH);
    digitalWrite(IN[3], LOW);
  }
  else if (buttonstat[1] == '1')
  {
    digitalWrite(IN[0], LOW);
    digitalWrite(IN[1], HIGH);
    digitalWrite(IN[2], LOW);
    digitalWrite(IN[3], HIGH);
  }
  else if (buttonstat[2] == '1')
  {
    digitalWrite(IN[0], LOW);
    digitalWrite(IN[1], HIGH);
    digitalWrite(IN[2], HIGH);
    digitalWrite(IN[3], LOW);
  }
  else if (buttonstat[3] == '1')
  {
    digitalWrite(IN[0], HIGH);
    digitalWrite(IN[1], LOW);
    digitalWrite(IN[2], LOW);
    digitalWrite(IN[3], HIGH);
  }
  else
  {
    digitalWrite(IN[0], LOW);
    digitalWrite(IN[1], LOW);
    digitalWrite(IN[2], LOW);
    digitalWrite(IN[3], LOW);
  }
}
