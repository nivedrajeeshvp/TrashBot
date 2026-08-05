#include <IotWebServer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <ESP32Servo.h>

/* ================= CONFIG ================= */
const char *Wifi_AP_name = "ESP Server";
String apikey = "ECNH7Eqbjpmx";

/* ================= PINS ================= */
int IN[] = {25, 26, 27, 13};
int buzzer = 23;
int button = 14;
int servoPin = 33;
int i2c[] = {21, 22};

/* ================= OBJECTS ================= */
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_VL53L0X lox;
Servo segregator;

/* ================= VARIABLES ================= */
String buttonstat = "0000";
String wasteType = "";
bool wasteDetected = false;
bool robotRunning = false;
bool alertSent = false;

unsigned long randomMoveTimer = 0;
int randomDirection = 0;

int detectThreshold = 25;

/* ================= BUZZER ================= */
void beep(int freq, int duration){
  tone(buzzer, freq, duration);
  delay(duration);
  noTone(buzzer);
}

/* ================= MOTOR ================= */

void moveForward(){
  digitalWrite(IN[0], HIGH);
  digitalWrite(IN[1], LOW);
  digitalWrite(IN[2], HIGH);
  digitalWrite(IN[3], LOW);
}

void moveBackward(){
  digitalWrite(IN[0], LOW);
  digitalWrite(IN[1], HIGH);
  digitalWrite(IN[2], LOW);
  digitalWrite(IN[3], HIGH);
}

void moveLeft(){
  digitalWrite(IN[0], LOW);
  digitalWrite(IN[1], HIGH);
  digitalWrite(IN[2], HIGH);
  digitalWrite(IN[3], LOW);
}

void moveRight(){
  digitalWrite(IN[0], HIGH);
  digitalWrite(IN[1], LOW);
  digitalWrite(IN[2], LOW);
  digitalWrite(IN[3], HIGH);
}

void stopRobot(){
  for(int i=0;i<4;i++)
    digitalWrite(IN[i],LOW);
}

/* ================= SETUP ================= */

void setup(){

  Serial.begin(115200);
  delay(3000);

  Wire.begin(i2c[0], i2c[1]);

  pinMode(button, INPUT);
  pinMode(buzzer, OUTPUT);

  for(int i=0;i<4;i++)
    pinMode(IN[i], OUTPUT);

  /* Servo Setup */
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);

  segregator.setPeriodHertz(50);
  segregator.attach(servoPin, 500, 2400);
  segregator.write(30);

  lcd.init();
  lcd.backlight();
  lcd.print("TrashBot Ready");
  delay(1500);

  if (!lox.begin()) {
    lcd.clear();
    lcd.print("ToF Error");
    while (1);
  }

  set_wifiAP_name(Wifi_AP_name);
  set_update_timeout(2000);
  set_debug(false);

  if (digitalRead(button)) startAP();
  else startWiFi();

  lcd.clear();
  lcd.print("Waiting Start");

  randomSeed(analogRead(34));
}

/* ================= LOOP ================= */

void loop(){

  buttonstat = updatebutton(apikey);

  /* START COMMAND */
  if(buttonstat[0]=='1' && !robotRunning){
    robotRunning = true;
    wasteDetected = false;
    alertSent = false;
    randomMoveTimer = millis();
  }

  /* STOP COMMAND */
  if(buttonstat[1]=='1'){
    robotRunning = false;
    stopRobot();
    lcd.clear();
    lcd.print("Stopped");
    return;
  }

  /* READ DISTANCE */
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  int distance_cm = 0;

  if(measure.RangeStatus != 4)
    distance_cm = measure.RangeMilliMeter / 10;

  /* RANDOM SEARCH MODE */

  if(robotRunning && !wasteDetected){

    if(millis() - randomMoveTimer > 2000){

      randomMoveTimer = millis();

      randomDirection = random(0,4);

      if(randomDirection==0) moveForward();
      else if(randomDirection==1) moveBackward();
      else if(randomDirection==2) moveLeft();
      else moveRight();
    }

    lcd.setCursor(0,0);
    lcd.print("Searching...");

    lcd.setCursor(0,1);
    lcd.print("Dist:");
    lcd.print(distance_cm);
    lcd.print("cm   ");
  }

  /* OBJECT DETECTION */

  if(robotRunning && distance_cm>0 && distance_cm<=detectThreshold){

    stopRobot();

    lcd.clear();
    lcd.print("Object Found");

    lcd.setCursor(0,1);
    lcd.print(distance_cm);
    lcd.print(" cm");

    beep(1500,100);

    Serial.println("CHECK_OBJECT");

    unsigned long waitTimer = millis();

    while(millis() - waitTimer < 5000){

      if(Serial.available()){

        char received = Serial.read();

        if(received=='A'){
          wasteType="BIO";
          wasteDetected=true;
          break;
        }

        else if(received=='B'){
          wasteType="NON";
          wasteDetected=true;
          break;
        }

        /* OBSTACLE */
        else if(received=='C'){

          lcd.clear();
          lcd.print("Obstacle");

          beep(1000,100);

          moveBackward();
          delay(400);

          moveRight();
          delay(600);

          stopRobot();

          wasteDetected=false;
          randomMoveTimer = millis();

          break;
        }
      }
    }
  }

  /* SEGREGATION */

  if(wasteDetected){

    lcd.clear();
    lcd.print(wasteType + " Waste");

    beep(2500,200);

    if(wasteType=="BIO")
      segregator.write(0);
    else
      segregator.write(180);

    delay(2000);

    segregator.write(30);

    if(!alertSent){

      String alert="Object found: "+wasteType+" | Dist:"+String(detectThreshold)+"cm";

      updateSensor_now(apikey, alert, detectThreshold);

      alertSent=true;
    }

    wasteDetected=false;
    robotRunning=false;

    lcd.clear();
    lcd.print("Waiting Start");
  }

  checkwificonnection();
}