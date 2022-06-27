////////////////////////////////////////////////////////////////////////////////////////
//      |LIBRERIAS|
////////////////////////////////////////////////////////////////////////////////////////

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Fonts\Stark_Italic7pt7b.h>
#include <PWM.h>
#include <ezButton.h>

////////////////////////////////////////////////////////////////////////////////////////
//      |NOTAS|
////////////////////////////////////////////////////////////////////////////////////////

#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_C7  2093

////////////////////////////////////////////////////////////////////////////////////////
//      |PINES|
////////////////////////////////////////////////////////////////////////////////////////

#define MOSFET         15    // pin mosfet ventilador
#define PULSADOR1      16    // pulsador en pin 2 pagina++
#define PULSADOR2      2    // pulsador en pin 3 pagina--
#define mosfetPWM      3    // mosfet controlador PWM
#define BUZZER_PASIVO  4    // buzzer pasivo en pin 4
#define A              5    // Variable de menor peso del mux
#define B              6    // Variable de medio peso del mux
#define C              7    // Variable de mayor peso del mux
#define TFT_RST        8    // pin tft
#define TFT_DC         9    // pin tft
#define TFT_CS         10   // pin tft
//                     11   // pin 11 tft
//                     12   // pin 12 tft
// SCK                 13   // pin 13 tft

////////////////////////////////////////////////////////////////////////////////////////
//      |TFT|
////////////////////////////////////////////////////////////////////////////////////////

#define BACKCOLOR 0x18E3
#define BARCOLOR 0x0620
#define SCALECOLOR 0xFFFF
#define NUM_SAMPLES 20

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

////////////////////////////////////////////////////////////////////////////////////////
//      |MELODIA|
////////////////////////////////////////////////////////////////////////////////////////

int melodia[] = {   // array con las notas de la melodia
  NOTE_E6, NOTE_F6, NOTE_C7
};

int duraciones[] = {    // array con la duracion de cada nota
  20, 20, 20
};

////////////////////////////////////////////////////////////////////////////////////////
//      |PULSADORES|
////////////////////////////////////////////////////////////////////////////////////////

int pagina = 0;

const int SHORT_PRESS_TIME  = 1000; // 1000 milliseconds
const int LONG_PRESS_TIME   = 1000; // 1000 milliseconds

int lastState = LOW;  // the previous state from the input pin
int currentState = LOW;     // the current reading from the input pin
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;
bool isPressing = false;
bool isLongDetected = false;

////////////////////////////////////////////////////////////////

int lastState1 = LOW;  // the previous state from the input pin
int currentState1 = LOW;     // the current reading from the input pin
unsigned long pressedTime1  = 0;
unsigned long releasedTime1 = 0;
bool isPressing1 = false;
bool isLongDetected1 = false;

////////////////////////////////////////////////////////////////

bool btBool1 = false;
bool btBool2 = false;
bool btBool3 = true;

////////////////////////////////////////////////////////////////////////////////////////
//      |VARIABLES GRAFICADORAS|
////////////////////////////////////////////////////////////////////////////////////////

int graficoMax = 83;
int grafico1 = 0;
int grafico2 = 0;

////////////////////////////////////////////////////////////////////////////////////////
//      |VARIABLES GLOBALES|
////////////////////////////////////////////////////////////////////////////////////////

float V5 = 4.37; //voltaje entre 5v y gnd con cable violeta de la fuente aliemntando el arduino

float voltaje = 0.0;
float Idc = 0.0;
float potencia = 0.0;
float voltajefinal = 0.0;
int vdecimal = 0;
float iva;
float sens;
int sum = 0;                    // suma de muestras hechas
unsigned char sample_count = 0; // numero de la muestra
int vvca;

float frecuencia  = 0;
float duty = 0;
float dutyPag = 0;

////////////////////////////////////////////////////////////////////////////////////////
//      |FUNCIONES GENERALES|
////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  pinMode(PULSADOR1, INPUT_PULLUP);  // pin 2 como entrada con resistencia de pull-up
  pinMode(PULSADOR2, INPUT_PULLUP);  // pin 3 como entrada con resistencia de pull-up
  
  pinMode(BUZZER_PASIVO, OUTPUT); // pin 4 como salida de buzzer
  sonido1();
  
  tft.initR(INITR_BLACKTAB); 
  tft.fillScreen(ST7735_BLACK);
  
  pinMode(MOSFET, OUTPUT);
  digitalWrite(MOSFET, LOW);
  
  pinMode(mosfetPWM, OUTPUT);
  InitTimersSafe();
}

void loop() {

  logo();
  temperatura();
  pulsadores();
  
  if(pagina != 4){
  corrienteCalculo();
  voltajeFunction();
  potenciaFunction();
  graficoV();
  graficoA();
  graficoP();
  recuadrosWT();
  }else{
  PWM();
  }
}

////////////////////////////////////////////////////////////////////////////////////////
//      |FUNCIONES PERSONALIZADAS|
////////////////////////////////////////////////////////////////////////////////////////

void temperatura() {

  float cursorA;
  float warnColor;
  float temperature;
  int SENSOR;   // variable almacena valor leido de entrada analogica A0
  float TEMPERATURA;  // valor de temperatura en grados centigrados
  float SUMA;   // valor de la suma de las 5 lecturas de temperatura
  tft.setFont();

  SUMA = 0;         // valor inicial de SUMA en cero
  for (int i=0; i < 5; i++){      // bucle que repite 5 veces
  SENSOR = analogRead(A5);      // lectura de entrada analogica A0  
  TEMPERATURA = ((SENSOR * (V5*1000)) / 1023) / 10;// formula para convertir valor leido
            // de entrada A0 en grados centigrados
  SUMA = TEMPERATURA + SUMA;      // suma de cada lectura de temperatura
  }

  temperature = SUMA/5.0;

  if(temperature >= 75){
    warnColor = ST7735_RED;
    digitalWrite(MOSFET, HIGH);
  }else{
    warnColor = ST7735_BLACK;
    digitalWrite(MOSFET, LOW);
  }

  if(temperature > 99.999){
    cursorA = 128;
  }else{
    cursorA = 134;
  }
  
    if(temperature < 10){
    tft.setCursor(134, 119);
    tft.print(" ");
  }else if(temperature < 100){
    tft.setCursor(128, 119);
    tft.print(" ");
  }
  
    tft.setTextColor(ST7735_WHITE, warnColor);
    tft.setTextSize(1);
    tft.setCursor(cursorA, 119);
    tft.println(temperature, 0);
    tft.setCursor(145, 119);
    tft.println(" C");
  }

void pulsadores() {

  currentState = digitalRead(PULSADOR1);

  if(lastState == HIGH && currentState == LOW) {        // button is pressed
    pressedTime = millis();
    isPressing = true;
    isLongDetected = false;
  } else if(lastState == LOW && currentState == HIGH) { // button is released
    isPressing = false;
    releasedTime = millis();

    long pressDuration = releasedTime - pressedTime;

    if( pressDuration < SHORT_PRESS_TIME ) {

      btBool3 = true;
      sonido2();
      
      if(frecuencia == 10 || frecuencia == 100){
        tft.setTextSize(2);
        tft.setCursor(105, 45);
        tft.println("    ");
      }
      
      if(dutyPag == 10 || dutyPag == 100){
        tft.setTextSize(2);
        tft.setCursor(105, 75);
        tft.println("     ");
      }
      
      if(btBool1 == true){
        if(btBool2 == true){
          dutyPag++;
        }else{
          frecuencia++;
        }
      }else{
      pagina++;
      tft.fillScreen(ST7735_BLACK);
      if(pagina > 4)pagina  = 0; 
      }
     
    }
  }

  if(isPressing == true && isLongDetected == false) {
    long pressDuration = millis() - pressedTime;

    if( pressDuration > LONG_PRESS_TIME && pagina == 4){
      sonido1();
      btBool3 = true;
      btBool2 = !btBool2;
      isLongDetected = true;
    }
  }

  lastState = currentState;

/////////////////////////////////////////////////////////////////////////////

  currentState1 = digitalRead(PULSADOR2);

  if(lastState1 == HIGH && currentState1 == LOW) {        // button is pressed
    pressedTime1 = millis();
    isPressing1 = true;
    isLongDetected1 = false;
  } else if(lastState1 == LOW && currentState1 == HIGH) { // button is released
    isPressing1 = false;
    releasedTime1 = millis();

    long pressDuration1 = releasedTime1 - pressedTime1;

    if( pressDuration1 < SHORT_PRESS_TIME ) {

      btBool3 = true;
      sonido2();
      
      if(frecuencia == 10 || frecuencia == 100){
        tft.setTextSize(2);
        tft.setCursor(105, 45);
        tft.println("    ");
      }
      
      if(dutyPag == 10 || dutyPag == 100){
        tft.setTextSize(2);
        tft.setCursor(105, 75);
        tft.println("     ");
      }
      
      if(btBool1 == true){
        if(btBool2 == true){
          dutyPag--;
        }else{
          frecuencia--;
        }
      }else{
      pagina--;
      tft.fillScreen(ST7735_BLACK);
      if(pagina < 0)  pagina = 4;
      }
     
    }
  }

  if(isPressing1 == true && isLongDetected1 == false) {
    long pressDuration1 = millis() - pressedTime1;

    if( pressDuration1 > LONG_PRESS_TIME && pagina == 4){
      sonido1();
      btBool3 = true;
      btBool1 = !btBool1;
      isLongDetected1 = true;
    }
  }

  lastState1 = currentState1;
 
}

void recuadrosWT() {

  tft.setRotation(1);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);

  float scre = 0;
  scre = 110/voltajefinal;

    tft.drawRect(108, 9, 9, 85, ST7735_WHITE); //recuadros blancos
    tft.drawRect(127, 9, 9, 85, ST7735_WHITE);
    tft.drawRect(146, 9, 9, 85, ST7735_WHITE);

    tft.setFont();
    
    tft.setCursor(110, 99);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.println("V");
    
    tft.setCursor(129, 99);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.println("I");
    
    tft.setCursor(148, 99);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.println("P");

}

void graficoV(){

  tft.setRotation(1);

  grafico1 = 2.76*voltajefinal;

  if(grafico2 > 90)grafico2 = 1;
  else  grafico2 = graficoMax - grafico1;

  tft.fillRect(109, 92, 7, -grafico1, ST7735_RED);
  tft.fillRect(109, 10, 7, grafico2, ST7735_BLACK);

}

void graficoA(){

  tft.setRotation(1);

  grafico1 = 8.3*Idc;

  if(grafico2 > 90)grafico2 = 1;
  else  grafico2 = graficoMax - grafico1;

  tft.fillRect(128, 92, 7, -grafico1, ST7735_RED);
  tft.fillRect(128, 10, 7, grafico2, ST7735_BLACK);

}

void graficoP(){

  tft.setRotation(1);

  grafico1 = 0.332*potencia;

  if(grafico2 > 90)grafico2 = 1;
  else  grafico2 = graficoMax - grafico1;

  tft.fillRect(147, 92, 7, -grafico1, ST7735_RED);
  tft.fillRect(147, 10, 7, grafico2, ST7735_BLACK);
  
  }

void logo(){
    tft.setFont(&Stark_Italic7pt7b);
    tft.setTextWrap(true);

    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setRotation(1);
    tft.setCursor(5, 125);
    tft.setTextSize(1);
    tft.print("Desa Industries");
}

void sonido1(){

    for (int i = 0; i < 3; i++) {     // bucle repite 3 veces
    int duracion = 1000 / duraciones[i];    // duracion de la nota en milisegundos
    tone(BUZZER_PASIVO, melodia[i], duracion);  // ejecuta el tono con la duracion
    int pausa = duracion * 1.30;      // calcula pausa
    delay(pausa);         // demora con valor de pausa
    noTone(BUZZER_PASIVO);        // detiene reproduccion de tono
    }
  
}

void sonido2(){

    for (int i = 0; i < 1; i++) {     // bucle repite 3 veces
    int duracion = 1000 / duraciones[i];    // duracion de la nota en milisegundos
    tone(BUZZER_PASIVO, NOTE_C7, duracion);  // ejecuta el tono con la duracion
    int pausa = duracion * 1.30;      // calcula pausa
    delay(pausa);         // demora con valor de pausa
    noTone(BUZZER_PASIVO);        // detiene reproduccion de tono
    }
  
}

void corrienteCalculo(){

  int numeroMuestras = 500;

  float acs = 0;
  float inte = 0;
  for(int i=0;i<numeroMuestras;i++)
  {
    acs = analogRead(A0) * (V5/1023.0);  //lectura*(vDeArduino/lectura del adc)
    inte = inte+(acs-iva)/sens;               //(acs-2.5)/sensibilidad del modulo
    }
    inte=inte/numeroMuestras;
    if(inte < 0) {
      inte = 0;
    }

    int idecimal = 0;

    int warnColor;
    tft.setFont();

   if(pagina == 0){
    iva = 2.593;
    sens = 0.07413;
    digitalWrite(A, LOW); //0   0
    digitalWrite(B, LOW); //0
    digitalWrite(C, LOW); //0
  }else if(pagina == 1){
    iva = 2.598;
    sens = 0.07413;
    digitalWrite(A, HIGH); //1  1
    digitalWrite(B, LOW); //0
    digitalWrite(C, LOW); //0
  }else if(pagina == 2){
    iva = 2.592;
    sens = 0.07413;
    digitalWrite(A, LOW); //0   2
    digitalWrite(B, HIGH); //1
    digitalWrite(C, LOW); //0
  }else if(pagina == 3){
    iva = 2.5893;
    sens = 0.07413;
    digitalWrite(A, HIGH); //1  3
    digitalWrite(B, HIGH); //1
    digitalWrite(C, LOW); //0
  }

    if(Idc > 99.999) {        //ajustar decimales segun escala
      idecimal = 1;
    }else if(Idc > 9.999){
      idecimal = 2;
    }else{
      idecimal = 3;
      }

  if(Idc >= 8){
    warnColor = ST7735_RED;
  }else{
    warnColor = ST7735_BLACK;
  }
    
  tft.setRotation(1);
  tft.setTextWrap(true);
  tft.setTextColor(ST7735_WHITE, warnColor);
  tft.setTextSize(2);

  tft.drawRoundRect(5, 40, 98, 24, 5, ST7735_WHITE);
    
  Idc=inte; //Idc=calculoCorriente(cantidad de muestras)
  tft.setCursor(40, 45);  // posicion (x,y)
  char string[10];
  dtostrf(Idc, 2, 4, string); //declara como string el valor en memoria de char
  tft.println(Idc, idecimal);  // imprime texto o valor

  tft.setCursor(10, 45);  // posicion (x,y)
  tft.println("I:");
  
}

void PWM() {
  
  SetPinFrequency(mosfetPWM, frecuencia);
  pwmWrite(mosfetPWM, duty);
  
  duty = 2.55 * dutyPag;

  if(frecuencia > 100)frecuencia  = 0;
  if(frecuencia < 0)  frecuencia  = 100;
  if(dutyPag > 100)   dutyPag  = 0;
  if(dutyPag < 0)     dutyPag  = 100;

  int sum = 0;                    // suma de muestras hechas
  unsigned char sample_count = 0; // numero de la muestra
  int vvca;

  if(pagina == 4){
    vvca = A4;
    voltajefinal = (voltaje / 0.09082); //(voltaje) / (R2 / (R2 + R1)) salida regulable
  }
  
     while (sample_count < NUM_SAMPLES) {
        sum += analogRead(vvca);
        sample_count++;
    }

    voltaje = ((float)sum / (float)NUM_SAMPLES * V5) / 1023.0; //voltaje = ((float)sum / (float)NUM_SAMPLES * voltaje de tu arduino) / 1024.0;
 
    sample_count = 0;
    sum = 0;

    if(voltajefinal > 99.999) {         //ajustar decimales segun escala
      vdecimal = 0;
    }else if(voltajefinal > 9.999){
      vdecimal = 1;
    }else{
      vdecimal = 2;
      }

  tft.setRotation(1);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.setFont();

  tft.drawRoundRect(10, 10, 44, 24, 5, ST7735_WHITE);
  tft.setCursor(15, 15);
  tft.println("PWM");

  tft.drawRoundRect(70, 10, 85, 24, 5, ST7735_WHITE);
  tft.setCursor(75, 15);  // posicion (x,y)
  tft.println("V:");
    
  char string[10];
  tft.setCursor(105, 15);  // posicion (x,y)
  dtostrf(voltajefinal, 2, 4, string); //declara como string el valor en memoria de char
  tft.println(voltajefinal, vdecimal); // imprime texto o valor

  tft.drawRoundRect(70, 40, 85, 24, 5, ST7735_WHITE);
  tft.setCursor(75, 45);  // posicion (x,y)
  tft.println("F:");

  char string1[10];
  tft.setCursor(105, 45);  // posicion (x,y)
  dtostrf(frecuencia, 2, 4, string1); //declara como string el valor en memoria de char
  tft.println(frecuencia, 0); // imprime texto o valor

  tft.drawRoundRect(70, 70, 85, 24, 5, ST7735_WHITE);
  tft.setCursor(75, 75);  // posicion (x,y)
  tft.println("D:");

  char string2[10];
  tft.setCursor(105, 75);  // posicion (x,y)
  dtostrf(dutyPag, 2, 4, string2); //declara como string el valor en memoria de char
  tft.println(dutyPag, 0); // imprime texto o valor

  tft.setCursor(140, 75);  // posicion (x,y)
  tft.println("%");

  tft.setTextSize(1);
  tft.setCursor(141, 52);  // posicion (x,y)
  tft.println("Hz");

  if(btBool3 == true){

  tft.setTextSize(2);

  tft.setCursor(5, 45);  // posicion (x,y)
  tft.println("     ");
  tft.setCursor(5, 50);  // posicion (x,y)
  tft.println("     ");
  tft.setCursor(5, 60);  // posicion (x,y)
  tft.println("     ");
  tft.setCursor(5, 70);  // posicion (x,y)
  tft.println("     ");
  tft.setCursor(5, 80);  // posicion (x,y)
  tft.println("     ");
  tft.setCursor(5, 90);  // posicion (x,y)
  tft.println("     ");


  for(int i = 1; i <= 5; i++) {

  tft.drawRect(10 * i, 43, 11, 11, ST7735_RED);
  tft.drawRect(10 * i, 53, 11, 11, ST7735_RED);
  tft.drawRect(10 * i, 63, 11, 11, ST7735_RED);
  tft.drawRect(10 * i, 73, 11, 11, ST7735_RED);
  tft.drawRect(10 * i, 83, 11, 11, ST7735_RED);

  }

  btBool3 = false;

  }

  if(dutyPag == 0){
    tft.drawLine(20, 83, 50, 83, ST7735_GREEN);
  }else if(dutyPag == 100){
    tft.drawLine(20, 53, 50, 53, ST7735_GREEN);
  }else{

  int dLine =  20 + (0.3 * dutyPag);
  
  tft.drawLine(20, 83, 20, 53, ST7735_GREEN);
  tft.drawLine(50, 83, 50, 53, ST7735_GREEN);
  
  tft.drawLine(20, 53, dLine, 53, ST7735_GREEN);
  tft.drawLine(dLine, 53, dLine, 83, ST7735_GREEN);
  tft.drawLine(dLine, 83, 50, 83, ST7735_GREEN);
  
  }

}

void voltajeFunction(){

  int warnColor;

  tft.setFont();

   if(pagina == 0){
    vvca = A1;
    voltajefinal = voltaje; // salida de 3.3
  }else if(pagina == 1){
    vvca = A2;
    voltajefinal = (voltaje / 0.845); //(voltaje) / (R2 / (R2 + R1)) salida de 5
  }else if(pagina == 2){
    vvca = A3;
    voltajefinal = (voltaje / 0.3190); //(voltaje) / (R2 / (R2 + R1)) salida de 12
  }else if(pagina == 3 || pagina == 4){
    vvca = A4;
    voltajefinal = (voltaje / 0.09041); //(voltaje) / (R2 / (R2 + R1)) salida regulable
  }

     while (sample_count < NUM_SAMPLES) {
        sum += analogRead(vvca);
        sample_count++;
    }

    voltaje = ((float)sum / (float)NUM_SAMPLES * V5) / 1024.0; //voltaje = ((float)sum / (float)NUM_SAMPLES * voltaje de tu arduino) / 1024.0;
 
    sample_count = 0;
    sum = 0;

    if(voltajefinal > 99.999) {         //ajustar decimales segun escala
      vdecimal = 1;
    }else if(voltajefinal > 9.999){
      vdecimal = 2;
    }else{
      vdecimal = 3;
      }

  if(voltajefinal >= 25){
    warnColor = ST7735_RED;
  }else{
    warnColor = ST7735_BLACK;
  }
    

  tft.setRotation(1);
  tft.setTextWrap(true);
  tft.setTextColor(ST7735_WHITE, warnColor);
  tft.setTextSize(2);
  
  tft.drawRoundRect(5, 10, 98, 24, 5, ST7735_WHITE);

  char string[10];
  tft.setCursor(40, 15);  // posicion (x,y)
  dtostrf(voltajefinal, 2, 4, string); //declara como string el valor en memoria de char
  tft.println(voltajefinal, vdecimal); // imprime texto o valor

  tft.setCursor(10, 15);  // posicion (x,y)
  tft.println("V:");
  
}

void potenciaFunction(){

  int pdecimal = 0;

  int warnColor;
  tft.setFont();

  potencia = voltajefinal*Idc;


    if(potencia > 99.999) {         //ajustar decimales segun escala
      pdecimal = 1;
    }else if(potencia > 9.999){
      pdecimal = 2;
    }else{
      pdecimal = 3;
    }

  if(potencia >= 100){
    warnColor = ST7735_RED;
  }else{
    warnColor = ST7735_BLACK;
  }

  tft.setRotation(1);
  tft.setTextWrap(true);
  tft.setTextColor(ST7735_WHITE, warnColor);
  tft.setTextSize(2);

  tft.drawRoundRect(5, 70, 98, 24, 5, ST7735_WHITE);

  char string[10];
  tft.setCursor(40, 75);  // posicion (x,y)
  dtostrf(potencia, 3, 4, string); //declara como string el valor en memoria de char
  tft.println(potencia, pdecimal); // imprime texto o valor

  tft.setCursor(10, 75);  // posicion (x,y)
  tft.println("P:");

  
}

  
