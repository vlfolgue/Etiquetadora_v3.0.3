#include <Arduino.h>
// Codigo adaptado de Arduino UNO a ESP32
// Equivalencias de pines actualizadas

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);  // LCD I2C 20x4

int muestras = 10;
int muestras_largo = 10;
int valor_corte_FC_min = 2500;
int valor_corte_FC2_max = 3500;
int valor_corte_FC2_min = 2900;

//----------------> VALORES A PARAMETRIZAR
unsigned long delay_botella_actuador = 0;
unsigned long delay_botella_etiqueta = 300;
unsigned long delay_etiqueta_contra = 0;
unsigned long tiempo_parada_actuador = 400;
unsigned long inicio_motor_etiqueta = 0;
unsigned long inicio_motor_contra = 0;
float tiempo_etiquetado;
//----------------

int boton = 25;
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
int pin_actuador = 16;

//---------> FOTOCELULAS
const int pin_LDR = 34;
const int pin_LDR2 = 35;
int valor_FC;
int valor_FC2;

int pin_estado_FC = 18;
int pin_estado_FC2 = 19;

//---------> MOTORES
int pin_Motor_Etiquetas = 12;
int pin_Motor_Contras = 14;

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

unsigned long inicio_actuador;
unsigned long tiempo_botella;
unsigned long tiempo_etiquetar;

bool etiquetapuesta = false;
bool contrapuesta = false;

int botellas_etiquetadas = 0;

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
  Wire.begin(22, 21);
  lcd.begin(20, 4);
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

  Serial.begin(115200);
}

void loop() {
  botella = digitalRead(pin_SensorIR);
  if (botella == LOW && detectada_botella == false) {
    llegada_botella = millis();
    while (botella == LOW) {
      botella = digitalRead(pin_SensorIR);
      pasando_botella = millis();
      if (pasando_botella - llegada_botella >= 3) {
        mover1 = true;
        mover2 = true;
        etiquetapuesta = false;
        contrapuesta = false;
        detectada_botella = true;
        llegada_botella = millis();
      }
    }
  }

  contra = digitalRead(boton);
  if (contra == LOW) {
    mover2 = false;
    contrapuesta = true;
  }

  if (detectada_botella && !actuador_fuera && (millis() > llegada_botella + delay_botella_actuador)) {
    digitalWrite(pin_actuador, HIGH);
    actuador_fuera = true;
    inicio_actuador = millis();
  }

  while (mover1 && actuador_fuera && (millis() > inicio_actuador + delay_botella_etiqueta) && llegada_botella != 0) {
    unsigned long ahora = millis();
    digitalWrite(pin_Motor_Etiquetas, HIGH);
    if (inicio_motor_etiqueta == 0) {
      inicio_motor_etiqueta = ahora;
    }

    int suma = 0;
    for (int i = 0; i < muestras; i++) {
      suma += analogRead(pin_LDR);
      delayMicroseconds(500);
      yield();
    }
    valor_FC = suma / muestras;
    digitalWrite(pin_estado_FC, valor_FC > valor_corte_FC_min ? HIGH : LOW);

    if ((ahora > inicio_motor_etiqueta + 300) && !etiquetapuesta) {
      if (valor_FC > valor_corte_FC_min) {
        etiquetapuesta = true;
      }
    }

    if (etiquetapuesta) {
      mover1 = false;
      digitalWrite(pin_Motor_Etiquetas, LOW);
      etiqueta_colocada = ahora;
      inicio_motor_etiqueta = 0;
    }

    delayMicroseconds(500);
    yield();

    Serial.print("Motor 1: ");
    Serial.print(mover1 ? "ON " : "OFF ");
    Serial.print("| FC1: ");
    Serial.print(valor_FC);
    Serial.print(" | Estado FC1: ");
    Serial.println(valor_FC > valor_corte_FC_min ? "ALTO" : "BAJO");
  }

  while (mover2 && etiquetapuesta && (millis() > etiqueta_colocada + delay_etiqueta_contra) && llegada_botella != 0) {
    unsigned long ahora = millis();
    digitalWrite(pin_Motor_Contras, HIGH);
    if (inicio_motor_contra == 0) {
      inicio_motor_contra = ahora;
    }

    int suma2 = 0;
    for (int i = 0; i < muestras; i++) {
      suma2 += analogRead(pin_LDR2);
      delayMicroseconds(500);
      yield();
    }
    valor_FC2 = suma2 / muestras;
    digitalWrite(pin_estado_FC2, valor_FC2 > valor_corte_FC2_min ? HIGH : LOW);

    if ((ahora > inicio_motor_contra + 300) && !contrapuesta) {
      if (valor_FC2 > valor_corte_FC2_min) {
        contrapuesta = true;
      }
    }

    if (contrapuesta) {
      mover2 = false;
      digitalWrite(pin_Motor_Contras, LOW);
      contra_colocada = ahora;
      inicio_motor_contra = 0;
      tiempo_etiquetado = (contra_colocada - llegada_botella) / 1000.0;
    }

    delayMicroseconds(500);
    yield();

    Serial.print("Motor 2: ");
    Serial.print(mover2 ? "ON " : "OFF ");
    Serial.print("| FC2: ");
    Serial.print(valor_FC2);
    Serial.print(" | Estado FC2: ");
    Serial.println(valor_FC2 > valor_corte_FC2_min ? "ALTO" : "BAJO");
  }

  if (contrapuesta && etiquetapuesta &&
      (millis() > tiempo_parada_actuador + contra_colocada) &&
      (millis() > tiempo_parada_actuador + etiqueta_colocada)) {
    digitalWrite(pin_actuador, LOW);
    actuador_fuera = false;
    detectada_botella = false;
    llegada_botella = 0;
    pasando_botella = 0;
    inicio_actuador = 0;
    etiqueta_colocada = 0;
    contra_colocada = 0;
    inicio_motor_etiqueta = 0;
    inicio_motor_contra = 0;
    botellas_etiquetadas++;
    etiquetapuesta = false;
    contrapuesta = false;
  }

  if (!mover1 && !mover2) {
    valor_FC = 0;
    for (int i = 0; i < muestras_largo; i++) {
      valor_FC += analogRead(pin_LDR);
      delayMicroseconds(500);
      yield();
    }
    valor_FC /= muestras_largo;
    digitalWrite(pin_estado_FC, valor_FC > valor_corte_FC_min ? HIGH : LOW);

    int suma2 = 0;
    for (int i = 0; i < muestras; i++) {
      suma2 += analogRead(pin_LDR2);
      delayMicroseconds(500);
      yield();
    }
    valor_FC2 = suma2 / muestras;
    digitalWrite(pin_estado_FC2, valor_FC2 > valor_corte_FC2_min ? HIGH : LOW);

    int lectura_pot1 = analogRead(33);
    int lectura_pot2 = analogRead(32);
    pot1 = filtrar_pot(lectura_pot1, hist_pot1);
    pot2 = filtrar_pot(lectura_pot2, hist_pot2);

    variar_actuador = map(pot2, 0, 4095, 0, 1000);
    variar_contra = map(pot1, 0, 4095, 0, 1000);

    delay_botella_actuador = variar_actuador;
    delay_etiqueta_contra = variar_contra;

    if (millis() - ultimo_refresco_lcd >= intervalo_lcd_idle) {
      ultimo_refresco_lcd = millis();

      lcd.setCursor(0, 0);
      lcd.print("Botellas:");
      lcd.setCursor(10, 0);
      lcd.print("        ");
      lcd.setCursor(10, 0);
      lcd.print(botellas_etiquetadas);

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

      lcd.setCursor(0, 3);
      lcd.print("TTotal:");
      lcd.setCursor(8, 3);
      lcd.print("        ");
      lcd.setCursor(8, 3);
      lcd.print(tiempo_etiquetado);
    }
  }
}
#include <Arduino.h>
// Codigo adaptado de Arduino UNO a ESP32
// Equivalencias de pines actualizadas

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);  // LCD I2C 20x4

int muestras = 2;
int muestras_largo = 2;
int valor_corte_FC_min = 2500;
int valor_corte_FC2_min = 2900;

//----------------> VALORES A PARAMETRIZAR
unsigned long delay_botella_actuador = 0;
unsigned long delay_botella_etiqueta = 300;
unsigned long delay_etiqueta_contra = 0;
unsigned long tiempo_parada_actuador = 400;
unsigned long inicio_motor_etiqueta = 0;
unsigned long inicio_motor_contra = 0;
float tiempo_etiquetado;
//----------------

int boton = 25;
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
int pin_actuador = 16;

//---------> FOTOCELULAS
const int pin_LDR = 34;
const int pin_LDR2 = 35;
int valor_FC;
int valor_FC2;

int pin_estado_FC = 18;
int pin_estado_FC2 = 19;

//---------> MOTORES
int pin_Motor_Etiquetas = 12;
int pin_Motor_Contras = 14;

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

unsigned long inicio_actuador;
unsigned long tiempo_botella;
unsigned long tiempo_etiquetar;

bool etiquetapuesta = false;
bool contrapuesta = false;

int botellas_etiquetadas = 0;

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
  Wire.begin(22, 21);
  pinMode(pin_Motor_Etiquetas, OUTPUT);
  pinMode(pin_Motor_Contras, OUTPUT);
  digitalWrite(pin_Motor_Etiquetas, LOW);
  digitalWrite(pin_Motor_Contras, LOW);
  lcd.begin(20, 4);
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
  pinMode(pin_SensorIR, INPUT);

 ////// ---> Calculamos la calibracion de las FC:
   // ---> Calibración inicial de fotocélulas (20s)
  unsigned long tiempo_inicio_calibracion = millis();
  unsigned long duracion_calibracion = 20000;  // 20 segundos
  unsigned long total_FC1 = 0;
  unsigned long total_FC2 = 0;
  unsigned long contador_muestras = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Asegurate que durante");
  lcd.setCursor(0, 1);
  lcd.print("20 segundos leemos");
  lcd.setCursor(0, 2);
  lcd.print("el liner...");

  while (millis() - tiempo_inicio_calibracion < duracion_calibracion) {
    int lectura1 = analogRead(pin_LDR);
    int lectura2 = analogRead(pin_LDR2);
    total_FC1 += lectura1;
    total_FC2 += lectura2;
    contador_muestras++;
    delay(10);  // 100 lecturas por segundo (ajustable)
    yield();
  }

  // Calcular medias y restar 100
  int media_FC1 = total_FC1 / contador_muestras;
  int media_FC2 = total_FC2 / contador_muestras;

  valor_corte_FC_min = media_FC1 - 50;  // --> Se peude ajustar precision bajando a menos de 100.
  valor_corte_FC2_min = media_FC2; // --> Se peude ajustar precision bajando a menos de 100.

  // Mostrar resultados durante 5 segundos
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Umbrales ajustados:");
  lcd.setCursor(0, 1);
  lcd.print("FC1: ");
  lcd.print(valor_corte_FC_min);
  lcd.setCursor(0, 2);
  lcd.print("FC2: ");
  lcd.print(valor_corte_FC2_min);

  delay(5000);
  lcd.clear();

}

void loop() {
  botella = digitalRead(pin_SensorIR);
  if (botella == LOW && detectada_botella == false) {
    llegada_botella = millis();
    while (botella == LOW) {
      botella = digitalRead(pin_SensorIR);
      pasando_botella = millis();
      if (pasando_botella - llegada_botella >= 3) {
        mover1 = true;
        mover2 = true;
        etiquetapuesta = false;
        contrapuesta = false;
        detectada_botella = true;
        llegada_botella = millis();
      }
    }
  }

  contra = digitalRead(boton);
  if (contra == LOW) {
    mover2 = false;
    contrapuesta = true;
  }

  if (detectada_botella && !actuador_fuera && (millis() > llegada_botella + delay_botella_actuador)) {
    digitalWrite(pin_actuador, HIGH);
    actuador_fuera = true;
    inicio_actuador = millis();
  }

  while (mover1 && actuador_fuera && (millis() > inicio_actuador + delay_botella_etiqueta) && llegada_botella != 0) {
    unsigned long ahora = millis();
    digitalWrite(pin_Motor_Etiquetas, HIGH);
    if (inicio_motor_etiqueta == 0) {
      inicio_motor_etiqueta = ahora;
    }

    int contador_muestras_validas = 0;
    for (int i = 0; i < muestras; i++) {
      int lectura = analogRead(pin_LDR);
      if (lectura > valor_corte_FC_min) {
        contador_muestras_validas++;
      } else {
        contador_muestras_validas = 0;
      }
      digitalWrite(pin_estado_FC, lectura > valor_corte_FC_min ? HIGH : LOW);

    }

    
    if ((ahora > inicio_motor_etiqueta + 300) && !etiquetapuesta) {
      if (contador_muestras_validas >= muestras) {
        etiquetapuesta = true;
      }
    }

    if (etiquetapuesta) {
      mover1 = false;
      digitalWrite(pin_Motor_Etiquetas, LOW);
      etiqueta_colocada = ahora;
      inicio_motor_etiqueta = 0;
    }
    delayMicroseconds(500);
    yield();


  }

  while (mover2 && etiquetapuesta && (millis() > etiqueta_colocada + delay_etiqueta_contra) && llegada_botella != 0) {
    unsigned long ahora = millis();
    digitalWrite(pin_Motor_Contras, HIGH);
    if (inicio_motor_contra == 0) {
      inicio_motor_contra = ahora;
    }

        int contador_muestras_validas2 = 0;
        for (int i = 0; i < muestras; i++) {
          int lectura = analogRead(pin_LDR2);
          if (lectura > valor_corte_FC2_min) {
            contador_muestras_validas2++;
          } else {
            contador_muestras_validas2 = 0;
          }

          digitalWrite(pin_estado_FC2, lectura > valor_corte_FC2_min ? HIGH : LOW);
        }


    if ((ahora > inicio_motor_contra + 300) && !contrapuesta) {
      if (contador_muestras_validas2 >= muestras) {
        contrapuesta = true;
      }
    }

    if (contrapuesta) {
      mover2 = false;
      digitalWrite(pin_Motor_Contras, LOW);
      contra_colocada = ahora;
      inicio_motor_contra = 0;
      tiempo_etiquetado = (contra_colocada - llegada_botella) / 1000.0;
    }

    delayMicroseconds(500);
    yield();

  }

  if (contrapuesta && etiquetapuesta &&
      (millis() > tiempo_parada_actuador + contra_colocada) &&
      (millis() > tiempo_parada_actuador + etiqueta_colocada)) {
    digitalWrite(pin_actuador, LOW);
    actuador_fuera = false;
    detectada_botella = false;
    llegada_botella = 0;
    pasando_botella = 0;
    inicio_actuador = 0;
    etiqueta_colocada = 0;
    contra_colocada = 0;
    inicio_motor_etiqueta = 0;
    inicio_motor_contra = 0;
    botellas_etiquetadas++;
    etiquetapuesta = false;
    contrapuesta = false;
  }

  if (!mover1 && !mover2) {
    valor_FC = 0;
    for (int i = 0; i < muestras_largo; i++) {
      valor_FC += analogRead(pin_LDR);
      delayMicroseconds(500);
      yield();
    }
    valor_FC /= muestras_largo;
    digitalWrite(pin_estado_FC, valor_FC > valor_corte_FC_min ? HIGH : LOW);

    int suma2 = 0;
    for (int i = 0; i < muestras; i++) {
      suma2 += analogRead(pin_LDR2);
      delayMicroseconds(500);
      yield();
    }
    valor_FC2 = suma2 / muestras;
    digitalWrite(pin_estado_FC2, valor_FC2 > valor_corte_FC2_min ? HIGH : LOW);

    int lectura_pot1 = analogRead(33);
    int lectura_pot2 = analogRead(32);
    pot1 = filtrar_pot(lectura_pot1, hist_pot1);
    pot2 = filtrar_pot(lectura_pot2, hist_pot2);

    variar_actuador = map(pot2, 0, 4095, 0, 1000);
    variar_contra = map(pot1, 0, 4095, 0, 1000);

    delay_botella_actuador = variar_actuador;
    delay_etiqueta_contra = variar_contra;

    if (millis() - ultimo_refresco_lcd >= intervalo_lcd_idle) {
      ultimo_refresco_lcd = millis();

      lcd.setCursor(0, 0);
      lcd.print("Botellas:");
      lcd.setCursor(10, 0);
      lcd.print("        ");
      lcd.setCursor(10, 0);
      lcd.print(botellas_etiquetadas);

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

      lcd.setCursor(0, 3);
      lcd.print("TTotal:");
      lcd.setCursor(8, 3);
      lcd.print("        ");
      lcd.setCursor(8, 3);
      lcd.print(tiempo_etiquetado);
    }
  }
}
