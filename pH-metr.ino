/* #######################################################################
pH метр на базе Arduino Uno ver: 1.0
Разработчики: Иванов Р.Н., Семеняченко А.А.
https://github.com/Ushuiski/pH-meter
#########################################################################*/

#include <LiquidCrystal.h>     // библиотека для работы с LCD 1602
#include <OneWire.h>           // библиотека для работы с протоколом 1-Wire
#include <DallasTemperature.h> // библиотека для работы с датчиком DS18B20

#define BUZZER_PIN 13          // пин с пищалкой
#define NUM_ELT 3              // количество элементов меню
#define OPERATING_VOLTAGE 5.0  // порное напряжения для АЦП
#define SENSOR 2               // пин АЦП для pH щупа
#define ONE_WIRE_BUS 2         // сигнальный провод датчика температуры

// Коэффициенты перевода напряжения в концентрацию pH
#define K1 -5.95
#define K2 21.9

OneWire oneWire(ONE_WIRE_BUS); // создаём объект для работы с библиотекой OneWire
 
DallasTemperature sensor(&oneWire); // создадим объект для работы с библиотекой DallasTemperature
 
// инициализируем объект-экран, передаём использованные 
// для подключения контакты на Arduino в порядке:
// RS, E, DB4, DB5, DB6, DB7
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//Пример создание символа
byte hui[8] = {
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b11111,
    0b11111,
    0b11111
};

// Мелодия Марио
int notes1[] = {
     // 1318, 1318, 1318, 1046, 1318, 1568, 784,
     1046, 784, 659, 880, 987, 932, 880, 784,
     1318, 1568, 1750, 1396, 1568, 1318, 1046, 1174, 987,
     /* 1046, 784, 659, 880, 987, 932, 880,
     784, 1318, 1568, 1750, 1396, 1568, 1318, 1046, 1174, 987,
     1568, 1480, 1396, 1244, 1318, 830, 880, 1046, 880, 1046, 1174,
     0, 1568, 1480, 1396, 1244, 1318, 2093, 2093, 2093,
     1568, 1480, 1396, 1244, 1318, 830, 880, 1046, 880, 1046, 1174, 1244, 1174, 1046, */
};

int times1[] = {
    // 150, 300, 150, 150, 300, 600, 600,
    450, 150, 300, 300, 150, 150, 300, 210,
    210, 150, 300, 150, 150, 300, 150, 150, 450,
    /* 450, 150, 300, 300, 150, 150, 300,
    210, 210, 150, 300, 150, 150, 300, 150, 150, 450,
    150, 150, 150, 300, 150, 150, 150, 150, 150, 150, 150,
    0, 150, 150, 150, 300, 150, 300, 150, 600,
    150, 150, 150, 300, 150, 150, 150, 150, 150, 150, 150, 300, 450, 600, */
};

int delays1[] = {
    // 150, 300, 300, 150, 300, 600, 600,
    450, 450, 450, 300, 300, 150, 300, 210,
    210, 150, 300, 150, 300, 300, 150, 150, 450,
    /* 450, 450, 450, 300, 300, 150, 300,
    210, 210, 150, 300, 150, 300, 300, 150, 150, 600,
    150, 150, 150, 300, 300, 150, 150, 300, 150, 150, 150,
    300, 150, 150, 150, 300, 300, 300, 150, 600,
    150, 150, 150, 300, 300, 150, 150, 300, 150, 150, 450, 450, 450, 1200, */
};

// Мелодия Имперский марш
int notes[] = {
    392, 392, 392, 311, 466, 392, 311, 466, 392 /*,
    587, 587, 587, 622, 466, 369, 311, 466, 392,
    784, 392, 392, 784, 739, 698, 659, 622, 659,
    415, 554, 523, 493, 466, 440, 466,
    311, 369, 311, 466, 392 */
};

int times[] = {
    350, 350, 350, 250, 100, 350, 250, 100, 700 /*,
    350, 350, 350, 250, 100, 350, 250, 100, 700,
    350, 250, 100, 350, 250, 100, 100, 100, 450,
    150, 350, 250, 100, 100, 100, 450,
    150, 350, 250, 100, 750 */
};

int button; //Переменная для значения кнопок
float x;    //Переменная для хранения значений pH
float t;    //Переменная для хранения температуры

// Структура для элемента меню
struct myStruc{
  byte Select;
  char Next;
  char Previous;
  char Parent;
  char Child;
  String Text;
};

myStruc menu[NUM_ELT]; // Создаём массив структур
myStruc menuNow;       // Создаём структуру menuNow

// Функция вывода ОК! для теста
void ok(void){
  lcd.clear();
  lcd.print ("OK!");
  delay(3000);
  lcd.clear();
  }

// Функция чтения pH щупа
float phMeasure(){
   float pHSum = 0;
   int adcSensor = 0;
   float voltageSensor = 0;
   float pHSensor = 0;
   
   for (int i = 0; i < 3; i++){
     adcSensor = analogRead(SENSOR);                       // Считываем аналоговое значение с датчика кислотности жидкости
     voltageSensor = adcSensor * OPERATING_VOLTAGE / 1023; // Переводим данные сенсора в напряжение
     pHSensor = K1 * voltageSensor + K2;                   // Конвертируем напряжение в концентрацию pH
     pHSum += pHSensor;
     }
   return pHSum/3;
  }

//Функция чтения термодатчика
float tempMeasure(){
   float tempSum = 0;
   float temperature;
   for (int i = 0; i < 3; i++){
     sensor.requestTemperatures();                       // отправляем запрос на измерение температуры
     temperature = sensor.getTempCByIndex(0);            // считываем данные из регистра датчик
     tempSum += temperature; 
     }
   return tempSum/3;
  }

void setup() {
    // Инициализация перефирии 
    pinMode(BUZZER_PIN, OUTPUT);                        //Конфигурируем ножку МК на выход
    lcd.begin(16, 2);                                   // устанавливаем размер (количество столбцов и строк) экрана
    lcd.createChar(0, hui);                             // create a new character
  
  
    lcd.setCursor(3, 0);
    lcd.print("Avgust Crop");
    lcd.setCursor(3, 1);  
    lcd.print("Protection");                           
    delay(3000);
    lcd.clear();
    
    menu[0] = {0x00, 1, 2, 0, 0, "pH measurment"};
    menu[1] = {0x01, 2, 0, 0, 0, "Calibration"};
    menu[2] = {0x02, 0, 1, 0, 0, "INFO"};
    
    menuNow = menu[0];

    lcd.begin(16, 2);
    lcd.setCursor(0, 0);

    Serial.begin(9600);                                 // инициализируем работу Serial-порта
    
    sensor.begin();                                     // начинаем работу с датчиком
    sensor.setResolution(12);                           // устанавливаем разрешение датчика от 9 до 12 бит
}
void loop() {
   lcd.setCursor(2, 0);   
   lcd.print(menuNow.Text);
   delay(100);
   button = analogRead (0);

   if (button < 60) { 
       lcd.clear();
       lcd.setCursor(0, 1);
       lcd.print ("Right");
       delay(100);
       for (int i = 0; i < 17; i++){
       tone(BUZZER_PIN, notes1[i], times1[i]);
       delay(delays1[i]);
       }
       }
   else if (button < 200){
      menuNow = menu[menuNow.Previous];
      lcd.clear();
      // lcd.print ("Up");
      delay(100);
       }
   else if (button < 400){
      menuNow = menu[menuNow.Next];
      lcd.clear();
      // lcd.print ("Down");
      delay(100);
       }
   else if (button < 600){ 
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print ("Left");
      delay(100);

      for (int i = 0; i < 9; i++){
      tone(BUZZER_PIN, notes[i], times[i]*2);
      delay(times[i]*2);
      noTone(BUZZER_PIN);
      }
      }
   else if (button < 800){
       switch(menuNow.Select){
           case 0x00:
             lcd.clear();
             lcd.print("loading...");
             
               while(1){
                 button = analogRead(0);
                 x = phMeasure();
                 t = tempMeasure();
                 lcd.clear();
                 
                 lcd.print("pH = ");
                 lcd.print(x);
                 lcd.print("\x3B");
                 
                 lcd.setCursor(0, 1);

                 lcd.print("T = ");
                 lcd.print(t);
                 lcd.print(" \xDF\x43\x3B");
                 
                 if (button < 600) { 
                   lcd.clear();
                   delay(300);
                   break;
                 }
                 //char lcd_buffer[16]; // Массив для вывода
                 //sprintf(lcd_buffer,"DSdebug=%u",ds1820_devices); //запись в буфер текста и значений
                 //lcd.print(lcd_buffer); // Вывод
                }
                break;
           case 0x01:
               ok();
               break;
           case 0x02:
               lcd.clear();
               lcd.setCursor(0, 1);
               lcd.print("Semenachko LOH ");
               lcd.write(byte(0));
               delay(1000);
               lcd.clear();
       }
       delay(100);
       } 
       
}
