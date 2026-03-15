#include <Arduino.h>
// Codigo adaptado de Arduino UNO a ESP32
// Equivalencias de pines actualizadas

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);  // LCD I2C 20x4

int muestras = 7;
int muestras_largo = 7;

//----------------> VALORES A PARAMETRIZAR
unsigned long delay_botella_actuador = 0;
unsigned long delay_botella_etiqueta = 200;
unsigned long delay_etiqueta_contra = 0;
unsigned long tiempo_parada_actuador = 300;
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
int valor_corte_FC_min = 2500;
int valor_corte_FC2_min = 2900;
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
bool FCentreetiquetas;
bool FCentrecontras;
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
  //---> PAro el Serial apra qeu no use tiempo de ejecucion
  //Serial.begin(115200);

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
            if ( pasando_botella - llegada_botella >= 5 ){               
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

//-----> LECTURA DE LLEGADA DE BOTELLA Y ACTIVACION DE LLAVES
/*Una vez que se detecta una botella, se activa la llave "detectada_botella", con lo que activamos las llaves de movimiento de 
* los motores "mover1" y "mover2"
* Además no hemos puesto aun etiqueta ni contra con lo que ponemos en false "etiquetapuesta" y "contrapuesta".
* Comprobamos donde está la LDR si entre etiquetas o en medio de la etiqueta para luego determinar donde debe parar cada motor.
* En funcion de donde esta, determinamos que tiene que cambiar ese estado como requisito para determinar que la etiqueta se ha puesto
* Si iniciamos entre etiquetas, en algun momento tenemos que ver la etiqueta. Si iniciamos en la etiqueta, tiene que llegar el entreetiquetas.
* Por tanto, cuando FCentreetiquetas es true, valorFC en el bucle tiene que mostrarse < valor_corteFC_min en algun momento y vicebersa */
if ( detectada_botella == true ){ //El sensor ha detectado una botella
     valor_FC =0; // Vaciamos valor de valor FC.
     for (int i=0; i< muestras; i++){ //Mediremos el numero de veces definido en muestras el sensor de la FC.
      valor_FC += analogRead(pin_LDR)/muestras;
     }

     if (valor_FC > valor_corte_FC_min){ //Comprobamos en base a la lectura de la FC si está entre etiquetas o encima de la etiqueta   
       FCentreetiquetas = true;}      
       else{    
       FCentreetiquetas = false;
     }
     
     valor_FC2 =0; // Vaciamos valor de valor FC2.  
     for (int i=0; i< muestras; i++){ //Mediremos el numero de veces definido en muestras el sensor de la FC2.
      valor_FC2 += analogRead(pin_LDR2)/muestras;
     }
          
     if (valor_FC2 > valor_corte_FC2_min){ //Comprobamos en base a la lectura de la FC si está entre contra o encima de la contra       
       FCentrecontras = true;}      
       else{        
       FCentrecontras = false;
     }
     
}

/*El actuador debe activarse cuando haya llegado la botella y haya pasado el tiempo de delay entre la llegada de botella y 
 * el momento que queremos que salga el actuador. Para que salga el actuador por tanto debe cumplirse:
 * 1. llegada_botella != 0
 * 2. mover1=true
 * 3. millis()>llegada_botella + delay_botella_actuador
 */
 
//-----> Si ha llegado botella y ha pasado el delay requerido entre botella y actuador. Iniciamos actuador y motor0.
if (mover1==true && (millis() > llegada_botella + delay_botella_actuador) && detectada_botella ==true && actuador_fuera == false && llegada_botella != 0 ) 
{
 digitalWrite(pin_actuador, HIGH); // Activamos el actuador
 inicio_actuador = millis();  //Almacenamos el timestamp.
 actuador_fuera = true; //Llave para no repetir esta interrupcion.
}
        
//-----> Mientras mover1 sea true, haya llegado botella y haya pasado el tiempo entre la llegada de la botella 
//y el tiempo de espera para lanzar etiqueta.
while (mover1==true && actuador_fuera==true && (millis() > inicio_actuador + delay_botella_etiqueta) && llegada_botella != 0 ) 
   {    
        digitalWrite(pin_Motor_Etiquetas,HIGH);//Activamos el motor_etiquetas  
                  
        if (inicio_motor_etiqueta == 0)
        {
        inicio_motor_etiqueta = millis(); //almacenamos tiempo inicio motor etiqueta 
        }
        valor_FC = 0;
        for (int i=0; i< muestras; i++){ //Mediremos el numero de veces definido en muestras el sensor de la FC.
        valor_FC += analogRead(pin_LDR)/muestras;
        }

        /* Puede darse el caso de que arranquemos el motor "entreetiquetas" y que por tanto al instante de activarlo, automaticamente
         *  se detecte en la FC un valor alto. Para evitar que ese valor alto nos dé señal de parada, comprobamos que se mueve la etiqueta
         *  y por tanto la llave etiquetapuesta se activará solo cuando cambie de estado la fotocelula.   
         No empezamos a comprobar hasta los 200ms desde que arranco el motor  */
        if (millis() >  inicio_motor_etiqueta + 200 && etiquetapuesta == false)
        {  
            if (FCentreetiquetas == true && valor_FC < valor_corte_FC_min){ //Si arranco entreetiquetas, ahora deberiamos leer etiqueta de nuevo        
                  etiquetapuesta = true;
            }         
            if (FCentreetiquetas == false && valor_FC > valor_corte_FC_min){ //Si arranco en medio de la etiqueta, nada mas que leamos entreetiquetas estará colocada.
                  etiquetapuesta = true;
            }
        }
               
        if (valor_FC >  valor_corte_FC_min ){ //Si el valor de la FC es mas alto que el de corte, estamos entre etiquetas.
            digitalWrite(pin_estado_FC,HIGH); // El led verde se enciende.
            
            if ( etiquetapuesta==true){ // Dentro de este if, para que ademas se pare el motor, ha tenido que cumplir el requisito "etiquetapuesta".
                mover1=false; // Paramos el motor de las etiquetas
                digitalWrite(pin_Motor_Etiquetas,LOW);//Paramos el motor_etiquetas 
                etiqueta_colocada = millis();
                inicio_motor_etiqueta = 0;
            }
        }     
        else{   
            digitalWrite(pin_estado_FC,LOW);   //El Led verde se apaga.
        }  
        
}//End While

////// CONTRAETIQUETA

while (mover2==true && etiquetapuesta==true && (millis() > etiqueta_colocada + delay_etiqueta_contra) && llegada_botella != 0 ) 
   {    
        digitalWrite(pin_Motor_Contras,HIGH);//Activamos el motor_contras  
                  
        if (inicio_motor_contra == 0)
        {
        inicio_motor_contra = millis(); //almacenamos tiempo inicio motor contra 
        }
        valor_FC2 = 0;
        for (int i=0; i< muestras; i++){ //Mediremos el numero de veces definido en muestras el sensor de la FC.
        valor_FC2 += analogRead(pin_LDR2)/muestras;
        }

        /* Puede darse el caso de que arranquemos el motor "entreetiquetas" y que por tanto al instante de activarlo, automaticamente
         *  se detecte en la FC un valor alto. Para evitar que ese valor alto nos dé señal de parada, comprobamos que se mueve la etiqueta
         *  y por tanto la llave etiquetapuesta se activará solo cuando cambie de estado la fotocelula.   
         No empezamos a comprobar hasta los 200ms desde que arranco el motor  */
        if (millis() >  inicio_motor_contra + 200 && contrapuesta == false)
        {  
            if (FCentrecontras == true && valor_FC2 < valor_corte_FC2_min){ //Si arranco entreetiquetas, ahora deberiamos leer etiqueta de nuevo        
                  contrapuesta = true;
            }         
            if (FCentrecontras == false && valor_FC2 > valor_corte_FC2_min){ //Si arranco en medio de la etiqueta, nada mas que leamos entreetiquetas estará colocada.
                  contrapuesta = true;
            }
        }
               
        if (valor_FC2 >  valor_corte_FC2_min ){ //Si el valor de la FC es mas alto que el de corte, estamos entre contras.
            digitalWrite(pin_estado_FC2,HIGH); // El led verde se enciende.
            
            if ( contrapuesta==true){ // Dentro de este if, para que ademas se pare el motor, ha tenido que cumplir el requisito "contrapuesta".
                mover2=false; // Paramos el motor de las contras
                digitalWrite(pin_Motor_Contras,LOW);//Paramos el motor_etiquetas 
                contra_colocada = millis();
                inicio_motor_contra = 0;
                tiempo_etiquetado = ((contra_colocada-llegada_botella)/1000.00);
            }
        }     
        else{   
            digitalWrite(pin_estado_FC2,LOW);   //El Led verde se apaga.
        }  
        
}//End While
         
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
      valor_FC2 = 0;
      for (int i = 0; i < muestras_largo; i++) { // Mediremos el número de veces definido en muestras el sensor de la FC.
        valor_FC2 += analogRead(pin_LDR2) / muestras_largo;
      }
      if (valor_FC2 > valor_corte_FC2_min) {
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

      //---> LCD (refresco controlado cada 500 ms)
      if (millis() - ultimo_refresco_lcd >= intervalo_lcd_idle) {
        ultimo_refresco_lcd = millis();

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Botellas:");
        lcd.setCursor(10, 0);
        lcd.print("    ");
        lcd.setCursor(10, 0);
        lcd.print(botellas_etiquetadas);

        lcd.setCursor(0, 2);
        lcd.print("TAct:");
        lcd.setCursor(5, 2);
        lcd.print("    ");
        lcd.setCursor(5, 2);
        lcd.print(delay_botella_actuador);

        lcd.setCursor(10, 2);
        lcd.print("TCotE:");
        lcd.setCursor(16, 2);
        lcd.print("    ");
        lcd.setCursor(16, 2);
        lcd.print(delay_etiqueta_contra);

        // Medida de las lecturas de las FC
        lcd.setCursor(0, 1);
        lcd.print("FC1:");
        lcd.setCursor(4, 1);
        lcd.print("    ");
        lcd.setCursor(4, 1);
        lcd.print(valor_FC);

        lcd.setCursor(10, 1);
        lcd.print("FC2:");
        lcd.setCursor(14, 1);
        lcd.print("    ");
        lcd.setCursor(14, 1);
        lcd.print(valor_FC2);

        // Tiempo total etiquetado
        lcd.setCursor(0, 3);
        lcd.print("TTotal:");
        lcd.setCursor(8, 3);
        lcd.print("        ");
        lcd.setCursor(8, 3);
        lcd.print(tiempo_etiquetado);
      }
}

/*
// 🟢 MONITORIZACIÓN CONTINUA DE SENSORES
Serial.println("===== ESTADO DE SENSORES =====");

Serial.print("Sensor IR (pin ");
Serial.print(pin_SensorIR);
Serial.print("): ");
Serial.println(digitalRead(pin_SensorIR) == LOW ? "OBJETO DETECTADO (LOW)" : "LIBRE (HIGH)");

Serial.print("Boton (pin ");
Serial.print(boton);
Serial.print("): ");
Serial.println(digitalRead(boton) == LOW ? "PULSADO (LOW)" : "NO PULSADO (HIGH)");

// Fotocélula etiquetas
int fc1_val = analogRead(pin_LDR);
Serial.print("FC1 (LDR pin ");
Serial.print(pin_LDR);
Serial.print("): ");
Serial.println(fc1_val);

// Fotocélula contras
int fc2_val = analogRead(pin_LDR2);
Serial.print("FC2 (LDR pin ");
Serial.print(pin_LDR2);
Serial.print("): ");
Serial.println(fc2_val);

// Potenciómetros
int pot1_val = analogRead(33);
int pot2_val = analogRead(32);
Serial.print("Potenciómetro 1 (GPIO 33): ");
Serial.println(pot1_val);
Serial.print("Potenciómetro 2 (GPIO 32): ");
Serial.println(pot2_val);

// Estado de salidas (actuadores y motores)
Serial.print("Actuador OUT (pin ");
Serial.print(pin_actuador);
Serial.print("): ");
Serial.println(digitalRead(pin_actuador));

Serial.print("Motor Etiquetas (pin ");
Serial.print(pin_Motor_Etiquetas);
Serial.print("): ");
Serial.println(digitalRead(pin_Motor_Etiquetas));

Serial.print("Motor Contras (pin ");
Serial.print(pin_Motor_Contras);
Serial.print("): ");
Serial.println(digitalRead(pin_Motor_Contras));

Serial.println("================================");
delay(100);  // Refresca cada 500 ms

*/

} // Cierre void loop