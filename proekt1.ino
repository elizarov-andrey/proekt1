#include <GyverTimers.h>


int RPM = 0;
long lastflash = 0;
long timeflash = 0;
int dimmer = (constrain(9700,9700,300));
int dim = 100;
float err = 0;
float integral = 0;
float D = 0;
long PIDout = 0;
int minets = 5;
int seconds = 0;
boolean batton_flag = false;
boolean void_flag = false;
byte disp_flag = 0;
int s = 1;
int m = 0;
boolean timers = false;
boolean batton_flag2 = false;
boolean error_flag = false;
float prevErr = 0;
float kp = 0.7;
float ki = 0.8;
float kd = 0.04;
float dt = 0.01;



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

void interrupt_3(){
  RPM = 60 / ((float)(micros()-lastflash)/1000000);
  lastflash = micros();
}

long temps400=0 ;
long temps3=0 ;
long temps800=0 ;
long temps10=0 ;

void computePID() {
  err = dim - RPM;
  integral = (constrain((integral + (err * dt) * ki),0,3000));
  D = (err - prevErr) / dt;
  prevErr = err;
  PIDout = (constrain(((err * kp + integral) + D * kd),0,3000));
  dimmer = map(PIDout, 0, 3000, 9700, 300);
}

void PIN10() {
  pinMode(10, OUTPUT);
   digitalWrite(10, 1);
  delay(150);
  pinMode(10, OUTPUT);
   digitalWrite(10, 0);
}

void isr() {

static int lastDim;

digitalWrite(DIMMER_PIN, 0);

if (lastDim != dimmer) Timer2.setPeriod(lastDim = dimmer);

else Timer2.restart();

}


void setup() {
  pinMode(ZERO_PIN, INPUT_PULLUP);

  pinMode(DIMMER_PIN, OUTPUT);

  attachInterrupt(INT_NUM, isr, RISING);

  Timer2.enableISR();



  lcd.init();
 lcd.backlight();

  attachInterrupt(digitalPinToInterrupt(3),interrupt_3,FALLING);
  pinMode(6, INPUT_PULLUP);
  pinMode(9, OUTPUT);
  pinMode(5, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);

}

void loop() {
    if (analogRead(A0) > 900) {
      error_flag = true;
      disp_flag = 4;
      void_flag = false;
    } else if ((analogRead(A0) > 600 && analogRead(A0) < 900) && disp_flag != 5) {
      error_flag = true;
      disp_flag = 3;
      void_flag = false;
    } else {
      if (disp_flag != 4) {
        error_flag = false;
      }
      if ((millis()-temps400)>=400) {
        temps400=millis();
        if (disp_flag != 5) {
          disp_flag = 0;
        }
      }
    }
    if (RPM >= dim) {
      timers = true;
    }
    if (micros() - lastflash > 1000000) {
      RPM = 0;
    }
    if (disp_flag == 1 && timers == true) {
      if (micros() - timeflash > 1000000) {
        timeflash = micros();
        s = s - 1;
        if (s < 0) {
          s = 59;
          m = m - 1;
          if (m < 0) {
            m = 0;
          }
        }
      }
    }
    if (m == 0 && s == 0) {
      disp_flag = 2;
      timers = false;
      void_flag = false;
      s = 1;
    }
    if (!digitalRead(6) && batton_flag == false) {
      batton_flag = true;
      dimmer = 9700;
      if (void_flag == true) {
        disp_flag = 5;
        void_flag = false;
        timers = false;
      } else if (disp_flag == 4) {
        disp_flag = 0;
      }
      pinMode(9, OUTPUT);
       digitalWrite(9, 1);
      PIN10();
    } else {
      if ((millis()-temps3)>=3*1000) {
        temps3=millis();
        batton_flag = false;
        disp_flag = 0;
         analogWrite(9, 0);
      }
    }
    if (!digitalRead(5) && batton_flag == false) {
      batton_flag = true;
      dimmer = 9700;
      if (disp_flag != 4) {
        void_flag = !void_flag;
        s = seconds;
        m = minets;
      } else if (disp_flag == 4) {
        disp_flag = 0;
      }
    } else {
      if ((millis()-temps800)>=800) {
        temps800=millis();
        batton_flag = false;
      }
    }
    if (void_flag == true) {
      disp_flag = 1;
      if ((millis()-temps10)>=10) {
        temps10=millis();
        computePID();
      }
    }
    switch (disp_flag) {
    case 0:
      if ((!digitalRead(7) && batton_flag2 == false) && error_flag == false) {
        batton_flag = true;
        dim = dim + 10;
      } else {
        if ((millis()-temps400)>=400) {
          temps400=millis();
          batton_flag = false;
        }
      }
      if ((!digitalRead(8) && batton_flag2 == false) && error_flag == false) {
        batton_flag = true;
        dim = dim - 10;
      } else {
        if ((millis()-temps400)>=400) {
          temps400=millis();
          batton_flag = false;
        }
      }
      if ((!digitalRead(11) && batton_flag2 == false) && error_flag == false) {
        batton_flag = true;
        minets = minets + 1;
      } else {
        if ((millis()-temps400)>=400) {
          temps400=millis();
          batton_flag = false;
        }
      }
      if ((!digitalRead(12) && batton_flag2 == false) && error_flag == false) {
        batton_flag = true;
        minets = minets - 1;
      } else {
        if ((millis()-temps400)>=400) {
          temps400=millis();
          batton_flag = false;
        }
      }
      timers = false;
      if (dim < 100) {
        dim = 100;
      } else if (dim > 2700) {
        dim = 2700;
      }
      lcd.setCursor(0, 0);
       lcd.print((String((constrain(dim,100,2700))) + String(" Обороты         ")));
       if (minets < 1) {
        minets = 99;
      } else if (minets > 99) {
        minets = 1;
      }
      if (minets < 10) {
        lcd.setCursor(0, 1);
         lcd.print("0");
         lcd.setCursor(1, 1);
         lcd.print((String((constrain(minets,0,59)))));
         } else {
        lcd.setCursor(0, 1);
         lcd.print((String((constrain(minets,0,99)))));
         }
      lcd.setCursor(2, 1);
       lcd.print(":");
       if (seconds < 10) {
        lcd.setCursor(3, 1);
         lcd.print("0");
         lcd.setCursor(4, 1);
         lcd.print((String((constrain(seconds,0,59)))));
         } else {
        lcd.setCursor(3, 1);
         lcd.print((String((constrain(seconds,0,59)))));
         }
      lcd.setCursor(5, 1);
       lcd.print((String(" Таймер   ")));
         break;
     case 1:
      if (void_flag == false && (((disp_flag != 2 && disp_flag != 3) && disp_flag != 4) && disp_flag != 5)) {
        disp_flag = 0;
      }
      lcd.setCursor(0, 0);
       lcd.print((String(RPM) + String(" Оборотов                 ")));
       if (m < 10) {
        lcd.setCursor(0, 1);
         lcd.print("0");
         lcd.setCursor(1, 1);
         lcd.print((String(m)));
         } else {
        lcd.setCursor(0, 1);
         lcd.print((String(m)));
         }
      lcd.setCursor(2, 1);
       lcd.print(":");
       if (s < 10) {
        lcd.setCursor(3, 1);
         lcd.print("0");
         lcd.setCursor(4, 1);
         lcd.print((String(s)));
         } else {
        lcd.setCursor(3, 1);
         lcd.print((String(s)));
         }
      lcd.setCursor(5, 1);
       lcd.print(" Время     ");
         break;
     case 2:
      pinMode(9, OUTPUT);
       digitalWrite(9, 1);
      if ((millis()-temps3)>=3*1000) {
        temps3=millis();
        pinMode(9, OUTPUT);
         digitalWrite(9, 0);
      }
      PIN10();
      lcd.setCursor(0, 0);
       lcd.print("Процесс         ");
       lcd.setCursor(0, 1);
       lcd.print("завершон!       ");
       disp_flag = 0;
      break;
     case 3:
      lcd.setCursor(0, 0);
       lcd.print("Не закрыта      ");
       lcd.setCursor(0, 1);
       lcd.print("крышка!         ");
         break;
     case 4:
      pinMode(9, OUTPUT);
       digitalWrite(9, 1);
      if ((millis()-temps3)>=3*1000) {
        temps3=millis();
        pinMode(9, OUTPUT);
         digitalWrite(9, 0);
      }
      lcd.setCursor(0, 0);
       lcd.print("Дизбаланс       ");
       lcd.setCursor(0, 1);
       lcd.print("ротора!         ");
         break;
     case 5:
      lcd.setCursor(0, 0);
       lcd.print("Процесс         ");
       lcd.setCursor(0, 1);
       lcd.print("прерван!        ");
         break;
    }

}
