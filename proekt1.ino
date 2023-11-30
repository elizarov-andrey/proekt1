#include <GyverTimers.h>


int RPM = 0; //датчик оборотов двигателя
long lastflash = 0;
int dimmer = (constrain(9700,9700,300)); //временной интервал вкл. симистора
int dim = (constrain(100,100,2700)); //значение заданных оборотов
float err = 0; //пид
float integral = 0; //пид
float D = 0; //пид
long PIDout = 0; //пид
byte minets = 3; //переменная минут
byte seconds = 0; //переменная секунд
byte batton_flag = 0; //флаг кнопки пин 5
boolean void_flag = false; //флаг включения вункчий работы 
float kp = 0.7; //пид
float ki = 0.8; //пид
float kd = 0.04; //пид
float dt = 0.01; //пид
int disp_flag; //флаг отображения информации дисплея
float prevErr = 0; //пид


//функция рассчета оборотов сдатчика на 3 пине
void interrupt_3(){
  RPM = 60 / ((float)(micros()-lastflash)/1000000);
  lastflash = micros();
}

#define ZERO_PIN 2 //прерывание точки нуля синусоиды АС

#define INT_NUM 0

#define DIMMER_PIN 4 //управление пином симистора



#include <Wire.h>
#include <LCD_1602_RUS.h>

LCD_1602_RUS lcd(0x27,16,2);
//функция таймера включения симистора
  ISR(TIMER2_A) {
  if (void_flag == true) {
  digitalWrite(DIMMER_PIN, 1);
  Timer2.stop();
  }
  }

long temps500=0 ;
long temps10=0 ;
long temps1=0 ;
//пид регулятор
void computePID() {
  err = dim - RPM;
  integral = (constrain((integral + (err * dt) * ki),0,3000));
  D = (err - prevErr) / dt;
  prevErr = err;
  PIDout = (constrain(((err * kp + integral) + D * kd),0,3000));
  dimmer = map(PIDout, 0, 3000, 9700, 300);
}
//функция выключения симмистора и сравнения заданного интервала времени
void isr() {

static int lastDim;

digitalWrite(DIMMER_PIN, 0);

if (lastDim != dimmer) Timer2.setPeriod(lastDim = dimmer);

else Timer2.restart();

}


void setup() {
  attachInterrupt(digitalPinToInterrupt(3),interrupt_3,FALLING);// прерывание датчика оборотов
  pinMode(ZERO_PIN, INPUT_PULLUP); 

  pinMode(DIMMER_PIN, OUTPUT); //симистор

  attachInterrupt(INT_NUM, isr, RISING);

  Timer2.enableISR();



  lcd.init();
 lcd.backlight();

  pinMode(7, INPUT_PULLUP); //кнопка значения +10 оборото
  pinMode(8, INPUT_PULLUP); //кнопка значения -10 оборото
  pinMode(12, INPUT_PULLUP);  //кнопка значения +1 минута таймера
  pinMode(13, INPUT_PULLUP); //кнопка значения -1 минута таймера
  pinMode(5, INPUT_PULLUP);  //кнопка старт-стоп

}

void loop() {
    if (micros() - lastflash > 1000000) { //если с датчика нет сигнала больше сикунды отображать ноль
      RPM = 0;
    }
    if (!digitalRead(7)) { //+ обороты
      dim = dim + 10;
    }
    if (!digitalRead(8)) { //- обороты
      dim = dim - 10;
    }
    if (!digitalRead(12)) { //+ минуты
      minets = minets + 1;
    }
    if (!digitalRead(13)) { //- минуты
      minets = minets - 1;
    }
    if (!digitalRead(5) && batton_flag == false) { //старт-стоп
      batton_flag = true;
      void_flag = !void_flag;
    } else {
      if ((millis()-temps500)>=500) {
        temps500=millis();
        batton_flag = false;
      }
    }
    if (void_flag == true) { //если члаг истина вызываем функцию пид каждые 10 милисикунд
      disp_flag = 1;
      if ((millis()-temps10)>=10) {
        temps10=millis();
        computePID();
      }
      if ((millis()-temps1)>=1*1000) { //таймер времени
        temps1=millis();
        if (seconds <= 0) {
          seconds = 59;
          minets = minets - 1;
          if (minets < 0) {
            minets = 0;
            if (seconds == 0 & minets == 0) {
              void_flag = false;
              disp_flag = 2; //отображение информации на дисплее о завершении
            }
          }
        }
      }
    }
    switch (disp_flag) {
    case 0: // отображение на дисплее задания параметров
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
     case 1: //отображение на дисплее оборотов с датчика и таймера во время работы
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
     case 2: //отображени на дисплее об окончании процесса работы
      lcd.setCursor(0, 0);
       lcd.print("Процесс");
       lcd.setCursor(0, 1);
       lcd.print("завершон!");
       delay(10000);
      disp_flag = 0;
      break;
     case 3: //отображение ошибки о незакрытой крышки
      lcd.setCursor(0, 0);
       lcd.print("Не закрыта ");
       lcd.setCursor(0, 1);
       lcd.print("крышка!");
         break;
     case 4: //отображение ошибки о вибрации двигателя
      lcd.setCursor(0, 0);
       lcd.print("Дизбаланс");
       lcd.setCursor(0, 1);
       lcd.print("ротора!");
         break;
    }

}
