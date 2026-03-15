#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

/* =========================
   HARDWARE / PINOUT
   ========================= */
LiquidCrystal_I2C lcd(0x27, 20, 4);

const int PIN_SDA = 21, PIN_SCL = 22;
const int PIN_IR_BOTELLA  = 17;
const int PIN_FC1         = 34;
const int PIN_FC2         = 35;
const int PIN_BTN_CONTRAS = 25;
const int PIN_BTN_AJUSTES = 19;
const int PIN_POT1        = 36;
const int PIN_POT2        = 39;
const int PIN_MOTOR_ETI   = 26;
const int PIN_MOTOR_CON   = 27;
const int PIN_ACTUADOR    = 16;

/* =========================
   PARÁMETROS AJUSTABLES
   ========================= */
unsigned long delay_botella_actuador = 0;
unsigned long delay_botella_etiqueta = 200;
unsigned long delay_etiqueta_contra  = 0;
unsigned long tiempo_parada_actuador = 300;

/* =========================
   ESTADO / TIEMPOS
   ========================= */
unsigned long inicio_motor_etiqueta = 0;
unsigned long inicio_motor_contra   = 0;
unsigned long llegada_botella       = 0;
unsigned long etiqueta_colocada     = 0;
unsigned long contra_colocada       = 0;

bool detectada_botella = false;
bool botella_detectada_previa = false;
bool actuador_fuera = false;

bool mover1 = false, mover2 = false;
bool etiquetapuesta = false, contrapuesta = false;
bool FCentreetiquetas = false, FCentrecontras = false;

int   botellas_etiquetadas = 0;
float tiempo_etiquetado    = 0.0;

/* =========================
   POTs + MODO AJUSTES
   ========================= */
#define HIST_POT_SIZE 5
int hist_pot1[HIST_POT_SIZE] = {0}, hist_pot2[HIST_POT_SIZE] = {0};
int idx_hist1 = 0, idx_hist2 = 0;

unsigned long ultimo_refresco_lcd = 0;
const unsigned long intervalo_lcd_idle = 200;

const unsigned long DEBOUNCE_MS = 30;
bool ajustes_btn_state = false;
bool ajustes_btn_last  = false;
unsigned long ajustes_last_change = 0;
bool ajustes_activos = false;

int  pot1_preview = 0, pot2_preview = 0;
long delay_contra_preview = 0, delay_actuador_preview = 0;

bool  last_ajustes_activos = false;
int   last_botellas = -1;
int   last_fc1 = -1, last_fc2 = -1;
long  last_TAct = -1, last_TCtE = -1;
float last_TTotal = -1.0f;
bool  need_full_redraw = true;

int filtrar_pot(int nuevo, int* hist, int &idx) {
  hist[idx] = nuevo;
  idx = (idx + 1) % HIST_POT_SIZE;
  int suma = 0;
  for (int i = 0; i < HIST_POT_SIZE; i++) suma += hist[i];
  return suma / HIST_POT_SIZE;
}

void gestionar_ajustes() {
  bool lectura = (digitalRead(PIN_BTN_AJUSTES) == HIGH);
  unsigned long now = millis();

  if (lectura != ajustes_btn_last) {
    ajustes_last_change = now;
    ajustes_btn_last = lectura;
  }
  if ((now - ajustes_last_change) > DEBOUNCE_MS) {
    if (lectura != ajustes_btn_state) {
      ajustes_btn_state = lectura;
      if (ajustes_btn_state) {
        ajustes_activos = true;
        need_full_redraw = true;
      } else {
        delay_etiqueta_contra  = delay_contra_preview;
        delay_botella_actuador = delay_actuador_preview;
        ajustes_activos = false;
        need_full_redraw = true;
      }
    }
  }

  if (ajustes_activos) {
    pot1_preview = filtrar_pot(analogRead(PIN_POT1), hist_pot1, idx_hist1);
    pot2_preview = filtrar_pot(analogRead(PIN_POT2), hist_pot2, idx_hist2);
    delay_contra_preview   = map(pot1_preview, 0, 4095, 0, 1000);
    delay_actuador_preview = map(pot2_preview, 0, 4095, 0, 1000);
  }
}

/* =========================
   LCD HELPERS
   ========================= */
void lcd_print_padded(int col, int row, const String &txt, int width) {
  lcd.setCursor(col, row);
  String out = txt;
  if ((int)out.length() > width) {
    out = out.substring(0, width);
  }
  lcd.print(out);
  for (int i = out.length(); i < width; ++i) {
    lcd.print(' ');
  }
}

void lcd_print_int(int col, int row, long val, int width) {
  lcd_print_padded(col, row, String(val), width);
}

void lcd_print_float(int col, int row, float val, int width, int decimals=2) {
  String s = String(val, decimals);
  lcd_print_padded(col, row, s, width);
}

void lcd_draw_static_labels(bool set_mode) {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Botellas:");
  lcd.setCursor(0,1); lcd.print("FC1:");
  lcd.setCursor(10,1); lcd.print("FC2:");
  if (set_mode) {
    // Dejo un espacio tras [SET] y muevo el valor a la col 12 (ver loop)
    lcd.setCursor(0,2); lcd.print("[SET] TAct:");
    lcd.setCursor(0,3); lcd.print("[SET] TCtE:");
  } else {
    lcd.setCursor(0,2);  lcd.print("TAct:");
    lcd.setCursor(10,2); lcd.print("TCtE:");
    lcd.setCursor(0,3);  lcd.print("TTotal:");
  }
}

/* =========================
   SETUP
   ========================= */
void setup() {
  Wire.begin(PIN_SDA, PIN_SCL);
  lcd.begin(20, 4);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Iniciando sistema");

  pinMode(PIN_IR_BOTELLA,  INPUT);
  pinMode(PIN_FC1,         INPUT);
  pinMode(PIN_FC2,         INPUT);
  pinMode(PIN_BTN_CONTRAS, INPUT);
  pinMode(PIN_BTN_AJUSTES, INPUT);

  pinMode(PIN_ACTUADOR, OUTPUT);  digitalWrite(PIN_ACTUADOR, LOW);
  pinMode(PIN_MOTOR_ETI, OUTPUT); digitalWrite(PIN_MOTOR_ETI, LOW);
  pinMode(PIN_MOTOR_CON, OUTPUT); digitalWrite(PIN_MOTOR_CON, LOW);

  // Lectura inicial potenciómetros
  for (int i=0; i<10; ++i) {
    filtrar_pot(analogRead(PIN_POT1), hist_pot1, idx_hist1);
    filtrar_pot(analogRead(PIN_POT2), hist_pot2, idx_hist2);
    delay(5);
  }
  int p1_init = filtrar_pot(analogRead(PIN_POT1), hist_pot1, idx_hist1);
  int p2_init = filtrar_pot(analogRead(PIN_POT2), hist_pot2, idx_hist2);
  delay_etiqueta_contra  = map(p1_init, 0, 4095, 0, 1000);
  delay_botella_actuador = map(p2_init, 0, 4095, 0, 1000);

  lcd_draw_static_labels(false);
  last_ajustes_activos = false;
  need_full_redraw = false;

  lcd_print_int(10,0, botellas_etiquetadas, 4);
  last_botellas = botellas_etiquetadas;
  int fc1 = (digitalRead(PIN_FC1) == HIGH) ? 1 : 0;
  int fc2 = (digitalRead(PIN_FC2) == HIGH) ? 1 : 0;
  lcd_print_padded(4,1, fc1==1 ? "HIGH" : "LOW ", 4);
  lcd_print_padded(14,1, fc2==1 ? "HIGH" : "LOW ", 4);
  last_fc1 = fc1; last_fc2 = fc2;
  lcd_print_int(5,2,  delay_botella_actuador, 4);
  lcd_print_int(15,2, delay_etiqueta_contra, 4);
  last_TAct = delay_botella_actuador;
  last_TCtE = delay_etiqueta_contra;
  lcd_print_float(8,3, tiempo_etiquetado, 8, 2);
  last_TTotal = tiempo_etiquetado;
}

/* =========================
   LOOP
   ========================= */
void loop() {
  const unsigned long now = millis();

  // 1) Detección botella
  bool botella = (digitalRead(PIN_IR_BOTELLA) == LOW);
  if (botella && !botella_detectada_previa) {
    llegada_botella = now;
    botella_detectada_previa = true;
  }
  if (!botella) botella_detectada_previa = false;

  if (!detectada_botella && botella && (now - llegada_botella >= 5)) {
    mover1 = mover2 = true;
    etiquetapuesta = contrapuesta = false;
    detectada_botella = true;
    llegada_botella = now;
    FCentreetiquetas = (digitalRead(PIN_FC1) == HIGH);
    FCentrecontras   = (digitalRead(PIN_FC2) == HIGH);
  }

  // 2) Selector contras
  if (digitalRead(PIN_BTN_CONTRAS) == LOW) { mover2 = false; contrapuesta = true; }

  // 3) Actuador
  if (mover1 && !actuador_fuera && llegada_botella &&
      now > (llegada_botella + delay_botella_actuador)) {
    digitalWrite(PIN_ACTUADOR, HIGH);
    actuador_fuera = true;
  }

  // 4) Motor etiquetas (entre=HIGH)
  while (mover1 && actuador_fuera && llegada_botella &&
         millis() > (llegada_botella + delay_botella_etiqueta)) {
    digitalWrite(PIN_MOTOR_ETI, HIGH);
    if (inicio_motor_etiqueta == 0) inicio_motor_etiqueta = millis();
    bool fc1_entre = (digitalRead(PIN_FC1) == HIGH);
    if ((millis() > inicio_motor_etiqueta + 200) && !etiquetapuesta) {
      if ((FCentreetiquetas && !fc1_entre) || (!FCentreetiquetas && fc1_entre)) {
        etiquetapuesta = true;
      }
    }
    if (fc1_entre && etiquetapuesta) {
      mover1 = false;
      digitalWrite(PIN_MOTOR_ETI, LOW);
      etiqueta_colocada = millis();
      inicio_motor_etiqueta = 0;
    }
  }

  // 5) Motor contras (entre=HIGH)
  while (mover2 && etiquetapuesta && llegada_botella &&
         millis() > (etiqueta_colocada + delay_etiqueta_contra)) {
    digitalWrite(PIN_MOTOR_CON, HIGH);
    if (inicio_motor_contra == 0) inicio_motor_contra = millis();
    bool fc2_entre = (digitalRead(PIN_FC2) == HIGH);
    if ((millis() > inicio_motor_contra + 200) && !contrapuesta) {
      if ((FCentrecontras && !fc2_entre) || (!FCentrecontras && fc2_entre)) {
        contrapuesta = true;
      }
    }
    if (fc2_entre && contrapuesta) {
      mover2 = false;
      digitalWrite(PIN_MOTOR_CON, LOW);
      contra_colocada = millis();
      inicio_motor_contra = 0;
      tiempo_etiquetado = (contra_colocada - llegada_botella) / 1000.0;
    }
  }

  // 6) Parar actuador
  if (etiquetapuesta && contrapuesta &&
      now > etiqueta_colocada + tiempo_parada_actuador &&
      now > contra_colocada + tiempo_parada_actuador) {
    digitalWrite(PIN_ACTUADOR, LOW);
    actuador_fuera = false;
    detectada_botella = false;
    botella_detectada_previa = false;
    llegada_botella = etiqueta_colocada = contra_colocada = 0;
    inicio_motor_etiqueta = inicio_motor_contra = 0;
    etiquetapuesta = contrapuesta = false;
    botellas_etiquetadas++;
  }

  // 7) Ajustes + LCD
  gestionar_ajustes();
  if (need_full_redraw || last_ajustes_activos != ajustes_activos) {
    lcd_draw_static_labels(ajustes_activos);
    need_full_redraw = false;
    last_ajustes_activos = ajustes_activos;
    last_botellas = -1; last_fc1 = -1; last_fc2 = -1;
    last_TAct = -1; last_TCtE = -1; last_TTotal = -1.0f;
  }

  if (now - ultimo_refresco_lcd >= intervalo_lcd_idle) {
    ultimo_refresco_lcd = now;

    if (botellas_etiquetadas != last_botellas) {
      lcd_print_int(10,0, botellas_etiquetadas, 4);
      last_botellas = botellas_etiquetadas;
    }

    int fc1v = (digitalRead(PIN_FC1) == HIGH) ? 1 : 0;
    int fc2v = (digitalRead(PIN_FC2) == HIGH) ? 1 : 0;
    if (fc1v != last_fc1) { lcd_print_padded(4,1,  fc1v ? "HIGH" : "LOW ", 4); last_fc1 = fc1v; }
    if (fc2v != last_fc2) { lcd_print_padded(14,1, fc2v ? "HIGH" : "LOW ", 4); last_fc2 = fc2v; }

    if (ajustes_activos) {
      // ⬇⬇⬇ VALORES EN MODO SET: columna 12 (deja 1 espacio) ⬇⬇⬇
      if (delay_actuador_preview != last_TAct) {
        lcd_print_int(12, 2, delay_actuador_preview, 4);
        last_TAct = delay_actuador_preview;
      }
      if (delay_contra_preview   != last_TCtE) {
        lcd_print_int(12, 3, delay_contra_preview, 4);
        last_TCtE = delay_contra_preview;
      }
    } else {
      if (delay_botella_actuador != last_TAct) {
        lcd_print_int(5, 2, delay_botella_actuador, 4);
        last_TAct = delay_botella_actuador;
      }
      if (delay_etiqueta_contra  != last_TCtE) {
        lcd_print_int(15, 2, delay_etiqueta_contra, 4);
        last_TCtE = delay_etiqueta_contra;
      }
      if (fabs(tiempo_etiquetado - last_TTotal) > 0.009f) {
        lcd_print_float(8, 3, tiempo_etiquetado, 8, 2);
        last_TTotal = tiempo_etiquetado;
      }
    }
  }
}
