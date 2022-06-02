
////////////////////////////////////////////////////////////////////////////////////////
//      |LIBRERIAS|
////////////////////////////////////////////////////////////////////////////////////////

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Fonts\Stark_Italic7pt7b.h>
#include <PWM.h>

////////////////////////////////////////////////////////////////////////////////////////
//      |NOTAS|
////////////////////////////////////////////////////////////////////////////////////////

#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_C7  2093

////////////////////////////////////////////////////////////////////////////////////////
//      |PINES|
////////////////////////////////////////////////////////////////////////////////////////

#define MOSFET         0    // pin mosfet ventilador
#define PULSADOR1      1    // pulsador en pin 2 pagina++
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
//                     13   // pin 13 tft

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
//      |VARIABLES GRAFICADORAS|
////////////////////////////////////////////////////////////////////////////////////////

int RawValue = 0;
int LastPercent = 0;
int RawValue1 = 0;
int LastPercent1 = 0;
int RawValue2 = 0;
int LastPercent2 = 0;

int graficoMax = 83;
int grafico1 = 0;
int grafico2 = 0;

////////////////////////////////////////////////////////////////////////////////////////
//      |VARIABLES GLOBALES|
////////////////////////////////////////////////////////////////////////////////////////

float V5 = 5; //voltaje entre 5v y gnd con cable violeta de la fuente aliemntando el arduino
int pagina = 0;

float voltaje = 0.0;
float Idc = 0.0;
float potencia = 0.0;
float voltajefinal = 0.0;
int vdecimal = 0;
float iva;
float sens;
int frecuencia  = 1;
int duty = 1;
int dutyPag = 1;

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
  pulsadores();
  temperatura();
  

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
    tft.setCursor(134, 117);
    tft.print(" ");
  }else if(temperature < 100){
    tft.setCursor(128, 117);
    tft.print(" ");
  }
  
    tft.setTextColor(ST7735_WHITE, warnColor);
    tft.setTextSize(1);
    tft.setCursor(cursorA, 117);
    tft.println(temperature, 0);
    tft.setCursor(145, 117);
    tft.println(" C");

    tft.setCursor(115, 117);
    tft.println(pagina);

    delay(10);
  }

void pulsadores() {
    if(digitalRead(PULSADOR1) == LOW){     // si se ha presionado el pulsador
      for (int i = 0; i < 1; i++) {     // bucle repite 1 veces
      int duracion = 1000 / duraciones[i];    // duracion de la nota en milisegundos
      tone(BUZZER_PASIVO, NOTE_C7, duracion);  // ejecuta el tono con la duracion
      int pausa = duracion * 1.30;      // calcula pausa
      delay(pausa);         // demora con valor de pausa
      noTone(BUZZER_PASIVO);        // detiene reproduccion de tono

      tft.setFont();
      pagina++;

      if(pagina > 4){
      pagina=0;
      }

      if(pagina == 4 || pagina == 0){
        tft.fillScreen(ST7735_BLACK);
      }
    }
      
  }else if(digitalRead(PULSADOR2) == LOW){     // si se ha presionado el pulsador
      for (int i = 0; i < 1; i++) {     // bucle repite 1 veces
      int duracion = 1000 / duraciones[i];    // duracion de la nota en milisegundos
      tone(BUZZER_PASIVO, NOTE_C7, duracion);  // ejecuta el tono con la duracion
      int pausa = duracion * 1.30;      // calcula pausa
      delay(pausa);         // demora con valor de pausa
      noTone(BUZZER_PASIVO);        // detiene reproduccion de tono

      tft.setFont();
      pagina--;

      if(pagina < 0){
      pagina=4;
      }

      if(pagina == 4 || pagina == 3){
        tft.fillScreen(ST7735_BLACK);
      }
    }
  }

}

void recuadrosWT() {

  tft.setRotation(1);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);

  float scre = 0;
  scre = 110/voltajefinal;

    tft.drawRect(106, 9, 9, 85, ST7735_WHITE); //recuadros blancos
    tft.drawRect(125, 9, 9, 85, ST7735_WHITE);
    tft.drawRect(144, 9, 9, 85, ST7735_WHITE);

    tft.setFont();
    
    tft.setCursor(108, 99);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.println("V");
    
    tft.setCursor(127, 99);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.println("I");
    
    tft.setCursor(146, 99);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.println("P");

}

void graficoV(){

  tft.setRotation(1);

  grafico1 = 2.76*voltajefinal;

  if(grafico2 > 90)grafico2 = 1;
  else  grafico2 = graficoMax - grafico1;

  tft.fillRect(107, 92, 7, -grafico1, ST7735_RED);
  tft.fillRect(107, 10, 7, grafico2, ST7735_BLACK);

}

void graficoA(){

  tft.setRotation(1);

  grafico1 = 8.3*Idc;

  if(grafico2 > 90)grafico2 = 1;
  else  grafico2 = graficoMax - grafico1;

  tft.fillRect(126, 92, 7, -grafico1, ST7735_RED);
  tft.fillRect(126, 10, 7, grafico2, ST7735_BLACK);

}

void graficoP(){

  tft.setRotation(1);

  grafico1 = 0.332*potencia;

  if(grafico2 > 90)grafico2 = 1;
  else  grafico2 = graficoMax - grafico1;

  tft.fillRect(145, 92, 7, -grafico1, ST7735_RED);
  tft.fillRect(145, 10, 7, grafico2, ST7735_BLACK);
  
  }

void logo(){
    tft.setFont(&Stark_Italic7pt7b);
    tft.setTextWrap(true);

    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setRotation(1);
    tft.setCursor(5, 123);
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
  duty = (255 / 100) * dutyPag;

  tft.setRotation(1);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.setFont();
  
  tft.setCursor(10, 10);
  tft.println("PWM");

  tft.setCursor(75, 10);  // posicion (x,y)
  tft.println("V:");
    
  char string[10];
  tft.setCursor(105, 10);  // posicion (x,y)
  dtostrf(voltajefinal, 2, 4, string); //declara como string el valor en memoria de char
  tft.println(voltajefinal, (vdecimal-1)); // imprime texto o valor

  tft.setCursor(75, 45);  // posicion (x,y)
  tft.println("F:");

  char string1[10];
  tft.setCursor(105, 45);  // posicion (x,y)
  dtostrf(frecuencia, 2, 4, string1); //declara como string el valor en memoria de char
  tft.println(frecuencia); // imprime texto o valor

  tft.setCursor(75, 80);  // posicion (x,y)
  tft.println("D:");

  char string2[10];
  tft.setCursor(105, 80);  // posicion (x,y)
  dtostrf(dutyPag, 2, 4, string2); //declara como string el valor en memoria de char
  tft.println(dutyPag); // imprime texto o valor

  tft.setCursor(140, 80);  // posicion (x,y)
  tft.println("%");

  for(int i = 1; i <= 5; i++) {

  tft.drawRect(10 * i, 45, 11, 11, ST7735_WHITE);
  tft.drawRect(10 * i, 55, 11, 11, ST7735_WHITE);
  tft.drawRect(10 * i, 65, 11, 11, ST7735_WHITE);
  tft.drawRect(10 * i, 75, 11, 11, ST7735_WHITE);
  tft.drawRect(10 * i, 85, 11, 11, ST7735_WHITE);

  }

  int dLine = 50 - (30 / 30);

  tft.drawLine(10, 85, 20, 85, ST7735_GREEN);
  tft.drawLine(50, 85, 60, 85, ST7735_GREEN);
  tft.drawLine(20, 85, 20, 55, ST7735_GREEN);
  
  tft.drawLine(20, 85, dLine, 85, ST7735_GREEN);
  tft.drawLine(dLine+30, 55, dLine+30, 85, ST7735_GREEN);
  tft.drawLine(50, 55, dLine, 55, ST7735_GREEN);

  

/*

  tft.setCursor(10, 45);  // posicion (x,y)
  tft.println("|");

  tft.setCursor(60, 95);  // posicion (x,y)
  tft.println("|");

*/

}

void voltajeFunction(){

  int sum = 0;                    // suma de muestras hechas
  unsigned char sample_count = 0; // numero de la muestra
  int vvca;

  int warnColor;

  tft.setFont();

   if(pagina == 0){
    vvca = A1;
    voltajefinal = voltaje; // salida de 3.3
  }else if(pagina == 1){
    vvca = A2;
    voltajefinal = (voltaje / 0.8722); //(voltaje) / (R2 / (R2 + R1)) salida de 5
  }else if(pagina == 2){
    vvca = A3;
    voltajefinal = (voltaje / 0.3190); //(voltaje) / (R2 / (R2 + R1)) salida de 12
  }else if(pagina == 3){
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

  char string[10];
  tft.setCursor(40, 10);  // posicion (x,y)
  dtostrf(voltajefinal, 2, 4, string); //declara como string el valor en memoria de char
  tft.println(voltajefinal, vdecimal); // imprime texto o valor

  tft.setCursor(10, 10);  // posicion (x,y)
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

  char string[10];
  tft.setCursor(40, 80);  // posicion (x,y)
  dtostrf(potencia, 3, 4, string); //declara como string el valor en memoria de char
  tft.println(potencia, pdecimal); // imprime texto o valor

  tft.setCursor(10, 80);  // posicion (x,y)
  tft.println("P:");

  
}

  