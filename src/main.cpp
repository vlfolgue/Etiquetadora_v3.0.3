#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27);  // Definimos la direccion de la LCD

int muestras = 2; // Numero de meustras para filtro de señales. Ajustar poara velocidad/calidad señal.
int muestras_largo = 7; // Numero de muestras para filtro de señales en estado parado. Numero Mayor.
//----------------> VALORES A PARAMETRIZAR
unsigned long delay_botella_actuador = 0; // Delay entre que llega al botella al final de carrera y entra el actuador
unsigned long delay_botella_etiqueta = 200; //Delay en milisegundos entre llegada de botella y salida de etiqueta.
unsigned long delay_etiqueta_contra = 0; // Delay entre que acaba una etiqueta y sale la otra.
unsigned long tiempo_parada_actuador = 200; //Delay desde que termina de salir la etiqueta y el actuador suelta la botella.

unsigned long inicio_motor_etiqueta = 0;
unsigned long inicio_motor_contra = 0;
float tiempo_etiquetado;
//----------------> FIN DE VALORES A PARAMETRIZAR

//----------------> INTERRUPTOR Y POTENCIOMETRO
int boton = 6; // Pin del Interruptor para activar contra o no.
int pot1 = 0; // Variable donde almacenaremos el valor del potenciometro
int pot2 = 0; // Variable donde almacenaremos el valor del potenciometro
long variar_actuador; //Variable mapeada de la variacion de tiempo de actuador
long variar_contra; //Variable mapeada de la variacion de tiempo de salida de contra

//---------> Sensor IR y deteccion Botella
const int pin_SensorIR=7; //Pin digital para final de carrera de botella detectado por el sensor IR
bool botella = false; //Variable que determina si sensor IR detecta botella.
bool detectada_botella = false; //Llave para almacenar lectura del sensor IR por lo que ha pasado una botella.

//---------> Actuador y motor0
bool actuador_fuera = false; //Llave de posicion de actuador

//---------> Actuador
int pin_actuador=8;

//---------> FOTOCELULA 
const int pin_LDR = A2; // Pin analogico de lactura de LDR.
const int pin_LDR2 = A3; // Pin analogico de lactura de LDR2.
int valor_FC; //Variable de lectura del valor de la LDR
int valor_FC2; //Variable de lectura del valor de la LDR2
int valor_corte_FC_min =910; //Definimos el valor en el que corta la FC cuando lee entre etiquetas
int valor_corte_FC2_min =400; //Definimos el valor en el que corta la FC cuando lee entre etiquetas
int pin_estado_FC =5; //Definimos pin para mostrar el estado de lectura de la fotocelula.
int pin_estado_FC2 =4; //Definimos pin para mostrar el estado de lectura de la fotocelula.
//---------> MOTORES y VARIADORES 
int pin_Motor_Etiquetas = 2; // Defino variable para el modulo rele en parte etiquetas
int pin_Motor_Contras = 3; // Defino variable para el modulo rele en parte contras

//---------> TIEMPOS
unsigned long llegada_botella; //Momento en el que llega la botella al final de carrera.
unsigned long salida_etiqueta; //Momento en el que sale la etiqueta
unsigned long parada_etiqueta; //Momento en el que deja de salir la etiqueta.
unsigned long etiqueta_colocada; //Momento en el que se coloca la etiqueta
unsigned long contra_colocada; //Momento en el que se coloca la contra
unsigned long parada_contra; //Momento en el que deja de salir la contra.
unsigned long salida_botella; //Momento en el que sale la botella etiquetada.
unsigned long pasando_botella; // Deteccion de que la botella está pasando

//---------> Llaves
bool mover1; //Llave para activar Motor1
bool mover2; //Llave para activar Motor2
bool contra = true ; // Llave para indicar que se va a poner contra o no. Por defecto True. 

//---------> TIEMPO CONTADOR
unsigned long inicio_actuador;
unsigned long tiempo_botella;
unsigned long tiempo_etiquetar;

//Comprobamos que la etiqueta está puesta y tensamos la etiqueta.
bool etiquetapuesta = false; //Variable que almacena el estado de la etiqueta para la botella actual
bool contrapuesta = false; //Variable que almacena el estado de la contra para la botella actual
bool FCentreetiquetas; //Llave para determinar que ha habido un cambio de estado de lectura de la FC.
bool FCentrecontras; //Llave para determinar que ha habido un cambio de estado de lectura de la FC.

//Contador botellas
int botellas_etiquetadas = 0;


void setup() {
  //Serial.begin(9600);         //iniciar puerto serie
    //-----> INTERRUPTOR
    pinMode(6, INPUT); 
    //-----> LCD
    lcd.begin (20,4);  //INICIAMOS LCD                     
    lcd.backlight();       
    //-----> FOTOCELULA 
    pinMode(pin_LDR, INPUT); //Definimos el Pin_LDR (A0) como Entrada.
    pinMode(pin_LDR2, INPUT); //Definimos el Pin_LDR (A0) como Entrada.
    pinMode(pin_estado_FC, OUTPUT); //Definimos el pin del estado de la FC ---> LED verde de la FC.
    pinMode(pin_estado_FC2, OUTPUT); //Definimos el pin del estado de la FC ---> LED verde de la FC.
    //-----> ACTUADOR 
    pinMode(pin_actuador, OUTPUT); //Definimos el pin que activa el rela del acuador como Salida
    digitalWrite(pin_actuador, LOW); // Paramos el actuador 
    /*Escribimos en el actuador para asegurar que está en Bajo. Revisar en caso de que el Actuador esté conectado al reves
     * en este caso es por el relé concreto usado. */
    //-----> MOTORES ETIQUETAS Y CONTRA
    pinMode(pin_Motor_Etiquetas , OUTPUT);  //definir pin como salida
    pinMode(pin_Motor_Contras , OUTPUT);  //definir pin como salida 
    digitalWrite(pin_Motor_Etiquetas, LOW); // Paramos el motor etiquetas
    digitalWrite(pin_Motor_Contras, LOW); // Paramos el motor contras
         
    //-------> Sensor IR
    pinMode(pin_SensorIR, INPUT); //Definimos el pint del sensor IR como Input.  
} // Cierre setup. 

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
        
if (mover1==false && mover2==false ){ // Definimos esta interrupcion para que solo entre cuando no está en marcha.
        
      //---> FOTOCELULAS Y LEDS DE ESTADO    
      //--> Leemos la fotocelula de las etiquetas varias veces, segun muestras_largo y definimos el estado del led.
      valor_FC = 0;
      for (int i=0; i< muestras_largo; i++){ //Mediremos el numero de veces definido en muestras el sensor de la FC.
      valor_FC += analogRead(pin_LDR)/muestras_largo;
      }           
        if (valor_FC >  valor_corte_FC_min ){   
              digitalWrite(pin_estado_FC,HIGH);
              }   
              else{
              digitalWrite(pin_estado_FC,LOW);   
              }
              
      //--> Leemos la fotocelula de las contras varias veces, segun muestras_largo y definimos el estado del led.
      valor_FC2 = 0;
      for (int i=0; i< muestras_largo; i++){ //Mediremos el numero de veces definido en muestras el sensor de la FC.
      valor_FC2 += analogRead(pin_LDR2)/muestras_largo;
      }
        if (valor_FC2 >  valor_corte_FC2_min ){ 
              digitalWrite(pin_estado_FC2,HIGH);
              }   
              else{
              digitalWrite(pin_estado_FC2,LOW);   
              }
        
     //---> POTENCIOMETROS
      pot1 = 0;
      for (int i=0; i< muestras_largo; i++){ //Mediremos el numero de veces definido en muestras el sensor de la FC.
      pot1 += analogRead(A1)/muestras_largo+1;
      }
      pot2 = 0;
      for (int i=0; i< muestras_largo; i++){ //Mediremos el numero de veces definido en muestras el sensor de la FC.
      pot2 += analogRead(A0)/muestras_largo+1;
      }
      //Mapeamos los valores leidos en lso potenciometros a milisegundos.        
      variar_actuador = map(pot1, 0, 1023, 1000, 0);
      variar_contra = map(pot2, 0, 1023, 0, 1000);
                                         
      //----> Ajustamos los tiempos del Actuador y contra segun potenciometros
      delay_botella_actuador = variar_actuador;
      delay_etiqueta_contra = variar_contra;

     //---> LCD                      
      lcd.setCursor(0,0);
      lcd.print("Botellas:");  
        lcd.setCursor(10,0); 
        lcd.print(botellas_etiquetadas);
      
      lcd.setCursor(0,2);
      lcd.print("TAct: ");  
        lcd.setCursor(5,2); 
        lcd.print("    ");
        lcd.setCursor(5,2);   
        lcd.print(delay_botella_actuador);
      
      lcd.setCursor(10,2);
      lcd.print("TCotE:");  
        lcd.setCursor(16,2); 
        lcd.print("    ");
        lcd.setCursor(16,2);   
        lcd.print(delay_etiqueta_contra);
      
      // Medida de las lecturas de las FC
     lcd.setCursor(0,1);
     lcd.print("FC1:"); 
        lcd.setCursor(4,1);   
        lcd.print(valor_FC);         
     lcd.setCursor(10,1);
     lcd.print("FC2:"); 
        lcd.setCursor(14,1);   
        lcd.print(valor_FC2); 
        
    // Tiempo total etiquetado
        lcd.setCursor(0,3);
      lcd.print("TTotal:");  
        lcd.setCursor(8,3); 
        lcd.print("        ");
        lcd.setCursor(8,3);   
        lcd.print(tiempo_etiquetado);                                              
} 

//Serial.print("botella ");
//Serial.println(botella);

/* 
 *  
 *  Serial.print("valor_FC");
Serial.print(valor_FC);
Serial.print(",");
Serial.print("valor_FC2");
Serial.println(valor_FC2);

Serial.print("contra_colocada ");
Serial.println(contra_colocada);
Serial.print("llegada_botella ");
Serial.println(llegada_botella);
Serial.print("tiempo_etiquetado ");
Serial.println(tiempo_etiquetado);
Serial.print("botella ");
Serial.println(botella);
Serial.print("llegada_botella ");
Serial.println(llegada_botella);
Serial.print("mover0 ");
Serial.println(mover0);
Serial.print("mover1 ");
Serial.println(mover1);
Serial.print("mover2 ");
Serial.println(mover2);
Serial.print("actuador_fuera ");
Serial.println(actuador_fuera);
 */                    
} // Cierre void loop