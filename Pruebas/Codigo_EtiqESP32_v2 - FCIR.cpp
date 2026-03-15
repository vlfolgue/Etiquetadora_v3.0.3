#include <Arduino.h>
// Codigo adaptado de Arduino UNO a ESP32
// Equivalencias de pines actualizadas

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);  // LCD I2C 20x4

int muestras = 10;
int muestras_largo = 10;
int valor_corte_FC_min = 2750;
int valor_corte_FC2_max = 3500;  // Umbral para FC2 con sensor IR.

//----------------> VALORES A PARAMETRIZAR
unsigned long delay_botella_actuador = 0;
unsigned long delay_botella_etiqueta = 300;
unsigned long delay_etiqueta_contra = 0;
unsigned long tiempo_parada_actuador = 400;
unsigned long inicio_motor_etiqueta = 0;
unsigned long inicio_motor_contra = 0;
float tiempo_etiquetado;
//----------------> FIN DE VALORES A PARAMETRIZAR

//----------------> INTERRUPTOR Y POTENCIOMETRO
int boton = 25;  // Selector "colocar contra o no"
int pot1 = 0;
int pot2 = 0;
long variar_actuador;
long variar_contra;

//---------> Sensor IR y deteccion Botella
const int pin_SensorIR = 17;
bool botella = false;
bool detectada_botella = false;

//---------> Actuador y motor
bool actuador_fuera = false;
int pin_actuador = 16;  // Relé del actuador (SSR)

//---------> FOTOCELULAS
const int pin_LDR = 34;
const int pin_LDR2 = 35;
int valor_FC;
int valor_FC2;

int pin_estado_FC = 18;
int pin_estado_FC2 = 19;

//---------> MOTORES
int pin_Motor_Etiquetas = 12;  // Transistor 1
int pin_Motor_Contras = 14;    // Transistor 2

//---------> TIEMPOS
unsigned long llegada_botella;
unsigned long salida_etiqueta;
unsigned long parada_etiqueta;
unsigned long etiqueta_colocada;
unsigned long contra_colocada;
unsigned long parada_contra;
unsigned long salida_botella;
unsigned long pasando_botella;

//---------> Llaves
bool mover1;
bool mover2;
bool contra = true;

//---------> TIEMPO CONTADOR
unsigned long inicio_actuador;
unsigned long tiempo_botella;
unsigned long tiempo_etiquetar;

bool etiquetapuesta = false;
bool contrapuesta = false;

int botellas_etiquetadas = 0;

//Funcion para poder filtrar los valores de los potenciometeros y montrar en la LCD.
#define HIST_POT_SIZE 5
int hist_pot1[HIST_POT_SIZE] = {0};
int hist_pot2[HIST_POT_SIZE] = {0};
unsigned long ultimo_refresco_lcd = 0;
const unsigned long intervalo_lcd_idle = 500;

int filtrar_pot(int nuevo, int* hist) {
  static int index_hist = 0;
  hist[index_hist] = nuevo;
  index_hist = (index_hist + 1) % HIST_POT_SIZE;
  int suma = 0;
  for (int i = 0; i < HIST_POT_SIZE; i++) suma += hist[i];
  return suma / HIST_POT_SIZE;
}

void setup() {
  //---> Paro el Serial apra qeu no use tiempo de ejecucion

  // 🟢 Inicializa el bus I2C en GPIO22 (SDA), GPIO21 (SCL)
  Wire.begin(22, 21);

  // 🟢 Inicializa LCD 20x4 correctamente
  lcd.begin(20, 4);     // En lugar de lcd.init();
  lcd.backlight();
  lcd.clear();

lcd.setCursor(0, 0);
lcd.print("Iniciando sistema");

int val_fc1 = analogRead(pin_LDR);
int val_fc2 = analogRead(pin_LDR2);

lcd.setCursor(0, 1);
lcd.print("FC1: ");
lcd.print(val_fc1);

lcd.setCursor(10, 1);
lcd.print("FC2: ");
lcd.print(val_fc2);


  pinMode(boton, INPUT_PULLUP);
  pinMode(pin_LDR, INPUT);
  pinMode(pin_LDR2, INPUT);
  pinMode(pin_estado_FC, OUTPUT);
  pinMode(pin_estado_FC2, OUTPUT);
  pinMode(pin_actuador, OUTPUT);
  digitalWrite(pin_actuador, LOW);
  pinMode(pin_Motor_Etiquetas, OUTPUT);
  pinMode(pin_Motor_Contras, OUTPUT);
  digitalWrite(pin_Motor_Etiquetas, LOW);
  digitalWrite(pin_Motor_Contras, LOW);
  pinMode(pin_SensorIR, INPUT);

//Serial.begin(115200);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
void loop() {

//-----> Leemos la posicion del final de carrera.
//Este estara en HIGH cuando llegue una botella.
botella = digitalRead(pin_SensorIR); 
 if ( botella == LOW && detectada_botella == false) //Esta normalmente en High. Cuando detecte, el if pone en true la llave.
 {
    llegada_botella = millis();  //Usamos la variable "llegada_botella" para almacenar el momento de llegada de la botella
    while (botella == LOW){
            botella = digitalRead(pin_SensorIR);
            pasando_botella = millis();
            if ( pasando_botella - llegada_botella >= 3 ){               
                 mover1=true; mover2=true; etiquetapuesta=false; contrapuesta=false;   
                 detectada_botella=true; 
                 llegada_botella = millis(); // Enscribimos este momento al temrinar la comprobacion como puntod e lelgada de la botella para evitar ruido.               
            }        
    }
 }

/*Comprobamos si se solicita la colocación de la contra etiqueta o no.
 * En caso de no requerir contra, se cierran de entrada las llaves que dirigen el "while" de la contra
 */
contra = digitalRead(boton);
if (contra == LOW){//activa en LOW
   mover2=false;
   contrapuesta=true; 
}

/*El actuador debe activarse cuando haya llegado la botella y haya pasado el tiempo de delay entre la llegada de botella y 
 * el momento que queremos que salga el actuador. Para que salga el actuador por tanto debe cumplirse:
 * 1. llegada_botella != 0
 * 2. mover1=true
 * 3. millis()>llegada_botella + delay_botella_actuador
 */
 
if (detectada_botella && !actuador_fuera && (millis() > llegada_botella + delay_botella_actuador)) {
    digitalWrite(pin_actuador, HIGH);  
    actuador_fuera = true;
    inicio_actuador = millis();
}

//-----> Si ha llegado botella y ha pasado el delay requerido entre botella y actuador. Iniciamos actuador y motor0.
////// ETIQUETA
while (mover1 && actuador_fuera && (millis() > inicio_actuador + delay_botella_etiqueta) && llegada_botella != 0) {
    unsigned long ahora = millis();

    digitalWrite(pin_Motor_Etiquetas, HIGH);  

    if (inicio_motor_etiqueta == 0) {
        inicio_motor_etiqueta = ahora;
    }

    // Lectura filtrada del sensor FC con delay entre muestras
    int suma = 0;
    for (int i = 0; i < muestras; i++) {
        suma += analogRead(pin_LDR);
        delay(20);  // Permite estabilizar la lectura ADC
    }
    valor_FC = suma / muestras;

    // LED de estado FC
    digitalWrite(pin_estado_FC, valor_FC > valor_corte_FC_min ? HIGH : LOW);

    // Nueva lógica: esperar tiempo mínimo y detectar si estamos en el "valle" (valor alto)
    if ((ahora > inicio_motor_etiqueta + 300) && !etiquetapuesta) {
        if (valor_FC > valor_corte_FC_min) {  // "Valle" porque el sensor está invertido
            etiquetapuesta = true;
        }
    }

    // Parar motor si la etiqueta ha sido puesta
    if (etiquetapuesta) {
        mover1 = false;
        digitalWrite(pin_Motor_Etiquetas, LOW);
        etiqueta_colocada = ahora;
        inicio_motor_etiqueta = 0;
    }

    delay(20);  // Controla la frecuencia del bucle
}

////// CONTRAETIQUETA
while (mover2 && etiquetapuesta && (millis() > etiqueta_colocada + delay_etiqueta_contra) && llegada_botella != 0) {
    unsigned long ahora = millis();

    digitalWrite(pin_Motor_Contras, HIGH);  

    if (inicio_motor_contra == 0) {
        inicio_motor_contra = ahora;
    }

    // Lectura filtrada del sensor FC2 con delay entre muestras
    int suma2 = 0;
    for (int i = 0; i < muestras; i++) {
        suma2 += analogRead(pin_LDR2);
        delay(20);  // Permite estabilizar la lectura ADC
    }
    valor_FC2 = suma2 / muestras;


    // ---> DEBUG: Imprimir estado del motor 2 y valor de FC2
//Serial.print("Motor 2: ");
//Serial.print(digitalRead(pin_Motor_Contras) == HIGH ? "ON" : "OFF");
//Serial.print(" | FC2: ");
//Serial.print(valor_FC2);
//Serial.print(" | Estado FC2: ");
//Serial.println(valor_FC2 < valor_corte_FC2_max ? "VALLE (etiqueta colocada)" : "ALTO");


    // LED de estado FC2
    digitalWrite(pin_estado_FC2, valor_FC2 < valor_corte_FC2_max ? HIGH : LOW);

    // Nueva lógica: esperar tiempo mínimo y detectar si volvemos al valle (valor promedio bajo)
    if ((ahora > inicio_motor_contra + 300) && !contrapuesta) {
        if (valor_FC2 < valor_corte_FC2_max) {  // Puedes ajustar este umbral según calibración
            contrapuesta = true;
        }
    }

    // Finalización si contraetiqueta ha sido puesta
    if (contrapuesta) {
        mover2 = false;
        digitalWrite(pin_Motor_Contras, LOW);
        contra_colocada = ahora;
        inicio_motor_contra = 0;
        tiempo_etiquetado = (contra_colocada - llegada_botella) / 1000.0;
    }

    delay(20);  // Controla la frecuencia del bucle
}
        
/* Una vez que la etiqueta ha salido y estamos entre etiquetas, el motor1 estará parado.
 * Determinamos ahora el momento en el que se parará el actuador que dependerád el momento en el que salio al botella. 
 * Se añaden unos delays ajustables en la aprte inicial del código para
 * poder variar el tiempo en el que termina la etiqueta.         * 
 */
// ESTA PARTE DEL CODIGO  PARA EL ACTUADOR CUANDO SE HAN PUESTO LAS ETIQUETAS NECESARIAS.
//-----> Sacamos actuador y dejamos de girar la botella


if (contrapuesta==true && etiquetapuesta==true && (millis() > tiempo_parada_actuador + contra_colocada)&&(millis() > tiempo_parada_actuador + etiqueta_colocada) ) 
        {    
        digitalWrite(pin_actuador, LOW); // Paramos el actuador
        actuador_fuera = false; //El actuador ha entrado (el piston no está fuera)
        detectada_botella = false; // Vaciamos la deteccion de botella.
        llegada_botella = 0 ;
        pasando_botella = 0;
        inicio_actuador = 0 ;
        etiqueta_colocada = 0 ;
        contra_colocada = 0;
        inicio_motor_etiqueta = 0;      
        inicio_motor_contra = 0;
        botellas_etiquetadas = botellas_etiquetadas +1;
        etiquetapuesta=false;
        contrapuesta=false;
        }
 
if (mover1==false && mover2==false ){ // Definimos esta interrupción para que solo entre cuando no está en marcha.

      //---> FOTOCELULAS Y LEDS DE ESTADO    
      //--> Leemos la fotocélula de las etiquetas varias veces, según muestras_largo y definimos el estado del LED.
      valor_FC = 0;
      for (int i = 0; i < muestras_largo; i++) { // Mediremos el número de veces definido en muestras el sensor de la FC.
        valor_FC += analogRead(pin_LDR) / muestras_largo;
      }
      if (valor_FC > valor_corte_FC_min) {
        digitalWrite(pin_estado_FC, HIGH);
      } else {
        digitalWrite(pin_estado_FC, LOW);
      }

      //--> Leemos la fotocélula de las contras varias veces, según muestras_largo y definimos el estado del LED.
    // Lectura filtrada del sensor FC2 con delay entre muestras
      int suma2 = 0;
      for (int i = 0; i < muestras; i++) {
          suma2 += analogRead(pin_LDR2);
          delay(1);  // Permite estabilizar la lectura ADC
      }
      valor_FC2 = suma2 / muestras;

      if (valor_FC2 < valor_corte_FC2_max) {
          digitalWrite(pin_estado_FC2, HIGH);
      } else {
          digitalWrite(pin_estado_FC2, LOW);
      }
      
      //---> POTENCIOMETROS
      int lectura_pot1 = analogRead(33);
      int lectura_pot2 = analogRead(32);
      pot1 = filtrar_pot(lectura_pot1, hist_pot1);
      pot2 = filtrar_pot(lectura_pot2, hist_pot2);

      // Mapeamos los valores leídos en los potenciómetros a milisegundos.
      variar_actuador = map(pot2, 0, 4095, 0, 1000);
      variar_contra   = map(pot1, 0, 4095, 0, 1000);

      //----> Ajustamos los tiempos del Actuador y contra según potenciómetros
      delay_botella_actuador = variar_actuador;
      delay_etiqueta_contra  = variar_contra;

      //---> LCD (refresco controlado cada 500 ms, sin parpadeo)
      if (millis() - ultimo_refresco_lcd >= intervalo_lcd_idle) {
        ultimo_refresco_lcd = millis();

        // Línea 0
        lcd.setCursor(0, 0);
        lcd.print("Botellas:");
        lcd.setCursor(10, 0);
        lcd.print("          ");  // Limpia valor anterior
        lcd.setCursor(10, 0);
        lcd.print(botellas_etiquetadas);

        // Línea 1
        lcd.setCursor(0, 1);
        lcd.print("FC1:");
        lcd.setCursor(4, 1);
        lcd.print("      ");
        lcd.setCursor(4, 1);
        lcd.print(valor_FC);

        lcd.setCursor(10, 1);
        lcd.print("FC2:");
        lcd.setCursor(14, 1);
        lcd.print("      ");
        lcd.setCursor(14, 1);
        lcd.print(valor_FC2);

        // Línea 2
        lcd.setCursor(0, 2);
        lcd.print("TAct:");
        lcd.setCursor(5, 2);
        lcd.print("      ");
        lcd.setCursor(5, 2);
        lcd.print(delay_botella_actuador);

        lcd.setCursor(10, 2);
        lcd.print("TCotE:");
        lcd.setCursor(16, 2);
        lcd.print("      ");
        lcd.setCursor(16, 2);
        lcd.print(delay_etiqueta_contra);

        // Línea 3
        lcd.setCursor(0, 3);
        lcd.print("TTotal:");
        lcd.setCursor(8, 3);
        lcd.print("        ");
        lcd.setCursor(8, 3);
        lcd.print(tiempo_etiquetado);
      }

}


} // Cierre void loop