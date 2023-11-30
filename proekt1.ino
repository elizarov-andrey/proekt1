#include <GyverTimers.h>


int RPM = 0;
long lastflash = 0;
int dimmer = (constrain(9700,9700,300));
int dim = (constrain(100,100,2700));
float err = 0;
float integral = 0;
float D = 0;
long PIDout = 0;
byte minets = 3;
byte seconds = 0;
byte batton_flag = 0;
boolean void_flag = false;
float kp = 0.7;
float ki = 0.8;
float kd = 0.04;
float dt = 0.01;
int disp_flag;
float prevErr = 0;



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
  if (void_flag == true) {
  digitalWrite(DIMMER_PIN, 1);
  Timer2.stop();
  }
  }

long temps500=0 ;
long temps10=0 ;
long temps1=0 ;

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
    } else {
      if ((millis()-temps500)>=500) {
        temps500=millis();
        batton_flag = false;
      }
    }
    if (void_flag == true) {
      disp_flag = 1;
      if ((millis()-temps10)>=10) {
        temps10=millis();
        computePID();
      }
      if ((millis()-temps1)>=1*1000) {
        temps1=millis();
        if (seconds <= 0) {
          seconds = 59;
          minets = minets - 1;
          if (minets < 0) {
            minets = 0;
            if (seconds == 0 & minets == 0) {
              void_flag = false;
              disp_flag = 2;
            }
          }
        }
      }
    }
    switch (disp_flag) {
    case 0:
      lcd.setCursor(0, 0);
       lcd.print((String(String(dim)) + String(" Обороты         ")));
       if (minets < 10) {
        lcd.setCursor(0, 1);
         lcd.print("0");
         lcd.setCursor(1, 1);
         lcd.print((String(String(minets))));
         } else {
        lcd.setCursor(0, 1);
         lcd.print((String(String(minets))));
         }
      lcd.setCursor(2, 1);
       lcd.print(":");
       if (seconds < 10) {
        lcd.setCursor(3, 1);
         lcd.print("0");
         lcd.setCursor(4, 1);
         lcd.print((String(String(seconds))));
         } else {
        lcd.setCursor(3, 1);
         lcd.print((String(String(seconds))));
         }
      lcd.setCursor(6, 1);
       lcd.print((String("Время")));
         break;
     case 1:
      lcd.setCursor(0, 0);
       lcd.print((String(String(RPM)) + String(" Оборотов                 ")));
       if (minets < 10) {
        lcd.setCursor(0, 1);
         lcd.print("0");
         lcd.setCursor(1, 1);
         lcd.print((String(String(minets))));
         } else {
        lcd.setCursor(0, 1);
         lcd.print((String(String(minets))));
         }
      lcd.setCursor(2, 1);
       lcd.print(":");
       if (seconds < 10) {
        lcd.setCursor(3, 1);
         lcd.print("0");
         lcd.setCursor(4, 1);
         lcd.print((String(String(seconds))));
         } else {
        lcd.setCursor(3, 1);
         lcd.print((String(String(seconds))));
         }
      lcd.setCursor(6, 1);
       lcd.print("Вреля");
       if (void_flag == false & ((disp_flag != 2 & disp_flag != 3) & disp_flag != 4)) {
        disp_flag = 0;
      }
      break;
     case 2:
      lcd.setCursor(0, 0);
       lcd.print("Процесс");
       lcd.setCursor(0, 1);
       lcd.print("завершон!");
       delay(10000);
      disp_flag = 0;
      break;
     case 3:
      lcd.setCursor(0, 0);
       lcd.print("Не закрыта ");
       lcd.setCursor(0, 1);
       lcd.print("крышка!");
         break;
     case 4:
      lcd.setCursor(0, 0);
       lcd.print("Дизбаланс");
       lcd.setCursor(0, 1);
       lcd.print("ротора!");
         break;
    }

  " Время";

}
