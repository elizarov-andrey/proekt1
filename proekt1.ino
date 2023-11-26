#include <GyverTimers.h>


float err = 0;
float integral = 0;
float prevErr = 0;
float D = 0;
long PIDout = 0;
int dimmer = (constrain(9700,9700,300));
int RPM = 0;
long lastflash = 0;
int dim = (constrain(100,100,2700));
byte minets = (constrain(1,0,99));
byte seconds = (constrain(0,0,59));
boolean batton_flag = false;
boolean void_flag = false;
float kp = 0.7;
float ki = 0.8;
float kd = 0.04;
float dt = 0.01;



void interrupt_3(){
  RPM = 60 / ((float)(micros()-lastflash)/1000000);
  lastflash = micros();
}

#define ZERO_PIN 2

#define INT_NUM 0

#define DIMMER_PIN 4



#include <Wire.h>
#include <LCD_1602_RUS.h>

LCD_1602_RUS lcd(0x27,16,2);

  ISR(TIMER2_A) {
  digitalWrite(DIMMER_PIN, 1);
  Timer2.stop();
  }



long temps10=0 ;

void computePID() {
  err = dim - RPM;
  integral = (constrain((integral + (err * dt) * ki),0,3000));
  D = (err - prevErr) / dt;
  prevErr = err;
  PIDout = (constrain(((err * kp + integral) + D * kd),0,3000));
  dimmer = map(PIDout, 0, 3000, 9700, 300);
}

void isr() {

static int lastDim;

digitalWrite(DIMMER_PIN, 0);

if (lastDim != dimmer) Timer2.setPeriod(lastDim = dimmer);

else Timer2.restart();

}


void setup() {
  attachInterrupt(digitalPinToInterrupt(3),interrupt_3,FALLING);
  pinMode(ZERO_PIN, INPUT_PULLUP);

  pinMode(DIMMER_PIN, OUTPUT);

  attachInterrupt(INT_NUM, isr, RISING);

  Timer2.enableISR();



  lcd.init();
 lcd.backlight();

  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);

}

void loop() {
    if (micros() - lastflash > 1000000) {
      RPM = 0;
    }
    if (!digitalRead(7)) {
      dim = dim + 10;
    }
    if (!digitalRead(8)) {
      dim = dim - 10;
    }
    if (!digitalRead(12)) {
      minets = minets + 1;
    }
    if (!digitalRead(13)) {
      minets = minets - 1;
    }
    if (!digitalRead(5) && batton_flag == false) {
      batton_flag = true;
      void_flag = !void_flag;
    }
    if (void_flag == true) {
      if ((millis()-temps10)>=10) {
        temps10=millis();
        computePID();
      }
      lcd.setCursor(0, 0);
       lcd.print((String(String(RPM)) + String(" Оборотов                 ")));
       lcd.setCursor(0, 1);
       lcd.print((String(String(dim)) + String(" Таймер           ")));
       }
    if (void_flag == false) {
      lcd.setCursor(0, 0);
       lcd.print((String(String(dim)) + String(" Обороты         ")));
       lcd.setCursor(0, 1);
       lcd.print((String(String(minets)) + String(":") + String(String(seconds)) + String(" Время             ")));
       }

}
