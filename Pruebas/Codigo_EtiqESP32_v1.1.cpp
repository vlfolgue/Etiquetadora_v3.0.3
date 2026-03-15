#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  // LCD I2C 20x4

int muestras = 15;
int muestras_largo = 15;

// Parámetros ajustables
unsigned long delay_botella_actuador = 0;
unsigned long delay_botella_etiqueta = 200;
unsigned long delay_etiqueta_contra = 0;
unsigned long tiempo_parada_actuador = 200;
unsigned long inicio_motor_etiqueta = 0;
unsigned long inicio_motor_contra = 0;
float tiempo_etiquetado;

int boton = 25;
int pot1 = 0, pot2 = 0;
long variar_actuador, variar_contra;

const int pin_SensorIR = 17;
bool botella = false, detectada_botella = false, botella_detectada_previa = false;

bool actuador_fuera = false;
int pin_actuador = 16;

const int pin_LDR = 34;
const int pin_LDR2 = 27;
int valor_FC, valor_FC2;
int valor_corte_FC_min = 2800;
int valor_corte_FC2_min = 3000;
int pin_estado_FC = 18;
int pin_estado_FC2 = 26;

int pin_Motor_Etiquetas = 12;
int pin_Motor_Contras = 14;

unsigned long llegada_botella, etiqueta_colocada, contra_colocada, pasando_botella;
unsigned long inicio_actuador;

bool mover1, mover2;
bool contra = true;
bool etiquetapuesta = false, contrapuesta = false;
bool FCentreetiquetas, FCentrecontras;
int botellas_etiquetadas = 0;

// LCD y lectura
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

int leer_fotocelula(int pin, int muestras) {
  int total = 0;
  for (int i = 0; i < muestras; i++) total += analogRead(pin);
  return total / muestras;
}

int leer_fotocelula_rapido(int pin) {
  return (analogRead(pin) + analogRead(pin)) / 2;
}

void actualizar_tiempos() {
  int lectura_pot1 = analogRead(33);
  int lectura_pot2 = analogRead(32);
  pot1 = filtrar_pot(lectura_pot1, hist_pot1);
  pot2 = filtrar_pot(lectura_pot2, hist_pot2);
  variar_actuador = map(pot2, 0, 4095, 0, 1000);
  variar_contra = map(pot1, 0, 4095, 0, 1000);
  delay_botella_actuador = variar_actuador;
  delay_etiqueta_contra = variar_contra;
}

void setup() {
  Wire.begin(22, 21);
  lcd.begin(20, 4);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando sistema");

  lcd.setCursor(0, 1);
  lcd.print("FC1: ");
  lcd.print(analogRead(pin_LDR));
  lcd.setCursor(10, 1);
  lcd.print("FC2: ");
  lcd.print(analogRead(pin_LDR2));

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

void loop() {
  unsigned long now = millis();

  // 🟢 1. Detección de botella no bloqueante
  botella = digitalRead(pin_SensorIR);
  if (botella == LOW && !botella_detectada_previa) {
    llegada_botella = now;
    botella_detectada_previa = true;
  }
  if (botella == HIGH) botella_detectada_previa = false;

  if (!detectada_botella && botella == LOW && now - llegada_botella >= 5) {
    mover1 = true;
    mover2 = true;
    etiquetapuesta = false;
    contrapuesta = false;
    detectada_botella = true;
    llegada_botella = now;

    valor_FC = leer_fotocelula(pin_LDR, muestras);
    FCentreetiquetas = valor_FC > valor_corte_FC_min;
    valor_FC2 = leer_fotocelula(pin_LDR2, muestras);
    FCentrecontras = valor_FC2 > valor_corte_FC2_min;
  }

  // 🟢 2. Selección contra
  contra = digitalRead(boton);
  if (contra == LOW) {
    mover2 = false;
    contrapuesta = true;
  }

  // 🟢 3. Activar actuador
  if (mover1 && !actuador_fuera && llegada_botella != 0 && now > llegada_botella + delay_botella_actuador) {
    digitalWrite(pin_actuador, HIGH);
    inicio_actuador = now;
    actuador_fuera = true;
  }

  // 🟢 4. Motor de etiquetas
  if (mover1 && actuador_fuera && now > inicio_actuador + delay_botella_etiqueta) {
    if (inicio_motor_etiqueta == 0) {
      digitalWrite(pin_Motor_Etiquetas, HIGH);
      inicio_motor_etiqueta = now;
    }

    valor_FC = leer_fotocelula_rapido(pin_LDR);
    if (now > inicio_motor_etiqueta + 200 && !etiquetapuesta) {
      if ((FCentreetiquetas && valor_FC < valor_corte_FC_min) ||
          (!FCentreetiquetas && valor_FC > valor_corte_FC_min)) {
        etiquetapuesta = true;
      }
    }

    if (valor_FC > valor_corte_FC_min) {
      digitalWrite(pin_estado_FC, HIGH);
      if (etiquetapuesta) {
        digitalWrite(pin_Motor_Etiquetas, LOW);
        mover1 = false;
        etiqueta_colocada = now;
        inicio_motor_etiqueta = 0;
      }
    } else {
      digitalWrite(pin_estado_FC, LOW);
    }
  }

  // 🟢 5. Motor de contras
  if (mover2 && etiquetapuesta && now > etiqueta_colocada + delay_etiqueta_contra) {
    if (inicio_motor_contra == 0) {
      digitalWrite(pin_Motor_Contras, HIGH);
      inicio_motor_contra = now;
    }

    valor_FC2 = leer_fotocelula_rapido(pin_LDR2);
    if (now > inicio_motor_contra + 200 && !contrapuesta) {
      if ((FCentrecontras && valor_FC2 < valor_corte_FC2_min) ||
          (!FCentrecontras && valor_FC2 > valor_corte_FC2_min)) {
        contrapuesta = true;
      }
    }

    if (valor_FC2 > valor_corte_FC2_min) {
      digitalWrite(pin_estado_FC2, HIGH);
      if (contrapuesta) {
        digitalWrite(pin_Motor_Contras, LOW);
        mover2 = false;
        contra_colocada = now;
        inicio_motor_contra = 0;
        tiempo_etiquetado = (contra_colocada - llegada_botella) / 1000.0;
      }
    } else {
      digitalWrite(pin_estado_FC2, LOW);
    }
  }

  // 🟢 6. Parar actuador
  if (etiquetapuesta && contrapuesta &&
      now > etiqueta_colocada + tiempo_parada_actuador &&
      now > contra_colocada + tiempo_parada_actuador) {
    digitalWrite(pin_actuador, LOW);
    actuador_fuera = false;
    detectada_botella = false;
    botella_detectada_previa = false;
    llegada_botella = 0;
    etiqueta_colocada = 0;
    contra_colocada = 0;
    inicio_motor_etiqueta = 0;
    inicio_motor_contra = 0;
    etiquetapuesta = false;
    contrapuesta = false;
    botellas_etiquetadas++;
  }

  // 🟢 7. Lecturas idle
  if (!mover1 && !mover2) {
    valor_FC = 0; valor_FC2 = 0;
    for (int i = 0; i < muestras_largo; i++) {
      valor_FC += analogRead(pin_LDR);
      valor_FC2 += analogRead(pin_LDR2);
    }
    valor_FC /= muestras_largo;
    valor_FC2 /= muestras_largo;
    digitalWrite(pin_estado_FC, valor_FC > valor_corte_FC_min ? HIGH : LOW);
    digitalWrite(pin_estado_FC2, valor_FC2 > valor_corte_FC2_min ? HIGH : LOW);

    actualizar_tiempos();

    if (now - ultimo_refresco_lcd >= intervalo_lcd_idle) {
      ultimo_refresco_lcd = now;

      lcd.setCursor(0, 0); lcd.print("Botellas:       ");
      lcd.setCursor(10, 0); lcd.print(botellas_etiquetadas);
      lcd.setCursor(0, 2); lcd.print("TAct:       ");
      lcd.setCursor(5, 2); lcd.print(delay_botella_actuador);
      lcd.setCursor(10, 2); lcd.print("TCotE:     ");
      lcd.setCursor(16, 2); lcd.print(delay_etiqueta_contra);
      lcd.setCursor(0, 1); lcd.print("FC1:     ");
      lcd.setCursor(4, 1); lcd.print(valor_FC);
      lcd.setCursor(10, 1); lcd.print("FC2:     ");
      lcd.setCursor(14, 1); lcd.print(valor_FC2);
      lcd.setCursor(0, 3); lcd.print("TTotal:        ");
      lcd.setCursor(8, 3); lcd.print(tiempo_etiquetado);
    }
  }
}
