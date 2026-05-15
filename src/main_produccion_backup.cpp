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

const unsigned long MOTOR_IGNITION_MS     = 200;  // ventana de ignición para detectar flanco (era 200 ms)
const unsigned long DELAY_POST_ACTUADOR_MS = 50; // espera mínima tras extender actuador

/* =========================
   LECTURA ESTABLE FC (anti-ruido 5 ms)
   ========================= */
const unsigned long STABLE_MS = 5;           // tiempo que debe mantenerse sin variar
const unsigned int  SAMPLE_US  = 200;        // periodo de muestreo durante la ventana
const unsigned long STABLE_IR_MS = 20;  // ms que el IR debe mantenerse en LOW para confirmar botella real (anti-ruido)


inline bool isStableLevel(int pin, int targetLevel, unsigned long stable_ms = STABLE_MS) {
  unsigned long t0 = millis();
  while ((millis() - t0) < stable_ms) {
    if (digitalRead(pin) != targetLevel) return false;
    delayMicroseconds(SAMPLE_US);
    yield();
  }
  return true;
}
inline bool isStableHigh(int pin, unsigned long stable_ms = STABLE_MS) { return isStableLevel(pin, HIGH, stable_ms); }
inline bool isStableLow (int pin, unsigned long stable_ms = STABLE_MS) { return isStableLevel(pin, LOW , stable_ms);  }

/* =========================
   ESTADO / TIEMPOS
   ========================= */
unsigned long inicio_motor_etiqueta = 0;
unsigned long inicio_motor_contra   = 0;
unsigned long llegada_botella       = 0;
unsigned long etiqueta_colocada     = 0;
unsigned long contra_colocada       = 0;
unsigned long tiempo_actuador_fuera = 0;

// Máquina de estados: rastrear si hemos visto ETIQUETA en este ciclo
bool fc1_vio_etiqueta = false;
bool fc2_vio_etiqueta = false;

bool detectada_botella = false;
bool botella_detectada_previa = false;
bool actuador_fuera = false;

bool mover1 = false, mover2 = false;
bool etiquetapuesta = false, contrapuesta = false;
bool FCentreetiquetas = false, FCentrecontras = false;

int   botellas_etiquetadas = 0;
float tiempo_etiquetado    = 0.0;

/* =========================
   TIMEOUTS Y ERRORES
   ========================= */
const unsigned long TIMEOUT_MOTOR_MS = 10000;  // 10 segundos máximo por motor
bool error_fc1_timeout = false;  // FC Etiquetas falló (timeout)
bool error_fc2_timeout = false;  // FC Contras falló (timeout)

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
int   last_p26_state = -1;  // Estado anterior de P26 para actualizar LCD

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
        // NUEVO: Si hay error, limpiar al presionar botón AJUSTES
        if (error_fc1_timeout || error_fc2_timeout) {
          // Limpiar flags de error
          error_fc1_timeout = false;
          error_fc2_timeout = false;
          // Resetear estado del ciclo completamente
          detectada_botella = false;
          botella_detectada_previa = false;
          mover1 = mover2 = false;
          etiquetapuesta = contrapuesta = false;
          actuador_fuera = false;
          fc1_vio_etiqueta = false;
          fc2_vio_etiqueta = false;
          llegada_botella = etiqueta_colocada = contra_colocada = 0;
          tiempo_actuador_fuera = 0;
          inicio_motor_etiqueta = inicio_motor_contra = 0;
          need_full_redraw = true;
        } else {
          ajustes_activos = true;
          need_full_redraw = true;
        }
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
    delay_contra_preview   = map(pot1_preview, 0, 4095, 0, 1500);
    delay_actuador_preview = map(pot2_preview, 0, 4095, 0, 1500);
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

  // Si hay error de timeout en FC, mostrar alerta
  if (error_fc1_timeout || error_fc2_timeout) {
    lcd.setCursor(0,0); lcd.print("Botellas:");
    lcd.setCursor(0,1); lcd.print("FC1: ");
    lcd.setCursor(5,1); lcd.print(error_fc1_timeout ? "FAIL" : "OK  ");
    lcd.setCursor(10,1); lcd.print("FC2: ");
    lcd.setCursor(15,1); lcd.print(error_fc2_timeout ? "FAIL" : "OK  ");

    lcd.setCursor(0,2);
    if (error_fc1_timeout) {
      lcd.print("ERROR: FC Etiquetas");
    } else {
      lcd.print("ERROR: FC Contras");
    }

    lcd.setCursor(0,3); lcd.print("Presiona AJUSTES");
    return;
  }

  // MODO NORMAL: Pantalla sin errores
  lcd.setCursor(0,0); lcd.print("Botellas:");
  lcd.setCursor(0,1); lcd.print("FC1:");
  lcd.setCursor(10,1); lcd.print("FC2:");
  if (set_mode) {
    // MODO AJUSTES: Desplazados a la derecha para evitar píxeles dañados
    lcd.setCursor(0,2); lcd.print("[SET] TAct:");
    lcd.setCursor(0,3); lcd.print("[SET] TCtE:");
  } else {
    // MODO NORMAL: Desplazados a la derecha para evitar píxeles dañados (cols 11-14)
    lcd.setCursor(0,2);  lcd.print("TAct:");
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
  digitalWrite(PIN_MOTOR_ETI, LOW);  // DOBLE GARANTÍA: P26 DEBE estar en LOW
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

  // Mostrar estado de P26 en la esquina superior derecha
  int p26_init = digitalRead(PIN_MOTOR_ETI) == HIGH ? 1 : 0;
  lcd.setCursor(18,0);
  lcd.print(p26_init);
  last_p26_state = p26_init;

  // Lecturas estables iniciales para mostrar en LCD
  int fc1 = isStableHigh(PIN_FC1) ? 1 : (isStableLow(PIN_FC1) ? 0 : (digitalRead(PIN_FC1)==HIGH));
  int fc2 = isStableHigh(PIN_FC2) ? 1 : (isStableLow(PIN_FC2) ? 0 : (digitalRead(PIN_FC2)==HIGH));
  lcd_print_padded(4,1,  fc1==1 ? "HIGH" : "LOW ", 4);
  lcd_print_padded(14,1, fc2==1 ? "HIGH" : "LOW ", 4);
  last_fc1 = fc1; last_fc2 = fc2;

  // Desplazados a la derecha para evitar píxeles dañados
  lcd_print_int(15,2, delay_botella_actuador, 4);
  last_TAct = delay_botella_actuador;
  lcd_print_float(15,3, tiempo_etiquetado, 7, 2);
  last_TTotal = tiempo_etiquetado;
}

/* =========================
   LOOP
   ========================= */
void loop() {
const unsigned long now = millis();

// 1) Detección botella con lectura ESTABLE (anti-ruido)
// Solo usar isStableLow cuando sea necesario (primer contacto)
bool ir_low_stable = false;
static bool ir_prev_high = true;  // Asumir que empieza en HIGH
bool ir_current = digitalRead(PIN_IR_BOTELLA) == HIGH;

if (!ir_current && !botella_detectada_previa) {
  // IR bajó (botella detectada) - confirmar con lectura estable
  if (isStableLow(PIN_IR_BOTELLA, STABLE_IR_MS)) {
    ir_low_stable = true;
    botella_detectada_previa = true;
  }
}

// Cuando IR sube (botella sale) - permitir nueva detección
if (ir_current && botella_detectada_previa && ir_prev_high == false) {
  botella_detectada_previa = false;
}

ir_prev_high = ir_current;

// Arranque de ciclo únicamente si la detección estable está presente
if (!detectada_botella && ir_low_stable) {
  mover1 = mover2 = true;
  etiquetapuesta = contrapuesta = false;
  detectada_botella = true;
  llegada_botella = now;

  // Estado "entre" inicial: HOME SIEMPRE ES HIGH (no capturar estado actual)
  // FC1: HOME = HIGH, ETIQUETA = LOW
  // FC2: HOME = HIGH, ETIQUETA = LOW
  FCentreetiquetas = true;   // HOME es HIGH para FC1
  FCentrecontras   = true;   // HOME es HIGH para FC2

  // Inicializar máquina de estados: si arrancamos con FC en ETIQUETA (LOW), ya la vimos
  // Lectura rápida para saber estado actual
  bool fc1_actual = (digitalRead(PIN_FC1) == HIGH);
  bool fc2_actual = (digitalRead(PIN_FC2) == HIGH);
  fc1_vio_etiqueta = !fc1_actual;  // Si está en LOW (no HOME), ya vimos etiqueta
  fc2_vio_etiqueta = !fc2_actual;  // Si está en LOW (no HOME), ya vimos etiqueta
}


  // 2) Selector contras
  if (digitalRead(PIN_BTN_CONTRAS) == LOW) { mover2 = false; contrapuesta = true; }

  // 3) Actuador
  if (mover1 && !actuador_fuera && llegada_botella &&
      now > (llegada_botella + delay_botella_actuador)) {
    digitalWrite(PIN_ACTUADOR, HIGH);
    actuador_fuera = true;
    tiempo_actuador_fuera = now;
  }

  // 4) Motor etiquetas (entre=HIGH)
  // GARANTÍA: P26 solo puede ir a HIGH si hay botella detectada
  if (!detectada_botella) {
    digitalWrite(PIN_MOTOR_ETI, LOW);
  } else if (mover1 && actuador_fuera && llegada_botella &&
             millis() > (tiempo_actuador_fuera + DELAY_POST_ACTUADOR_MS)) {
    // while convertido a if para evitar bloqueos
    // TRIPLE GARANTÍA: Verificar estado ANTES de subir
    if (detectada_botella && mover1) {
      digitalWrite(PIN_MOTOR_ETI, HIGH);
    } else {
      // Si algo está mal, FORZAR LOW inmediatamente
      digitalWrite(PIN_MOTOR_ETI, LOW);
      mover1 = false;
    }
    if (inicio_motor_etiqueta == 0 && digitalRead(PIN_MOTOR_ETI) == HIGH) inicio_motor_etiqueta = millis();

    // NUEVO: Verificar timeout (5 segundos máximo)
    if ((millis() - inicio_motor_etiqueta) > TIMEOUT_MOTOR_MS) {
      digitalWrite(PIN_MOTOR_ETI, LOW);
      digitalWrite(PIN_ACTUADOR, LOW);
      actuador_fuera = false;
      detectada_botella = false;
      mover1 = false;
      etiquetapuesta = false;
      contrapuesta = false;
      fc1_vio_etiqueta = false;
      fc2_vio_etiqueta = false;
      error_fc1_timeout = true;
      need_full_redraw = true;
    } else {
      // Continuar con detección si no hay timeout

    // UNA sola lectura digital rápida
    bool fc1_high = (digitalRead(PIN_FC1) == HIGH);
    bool fc1_en_etiqueta = (fc1_high != FCentreetiquetas);
    bool fc1_en_home = (fc1_high == FCentreetiquetas);

    // PASO 1: Registrar si vimos la etiqueta (cambio respecto a HOME)
    if (!fc1_vio_etiqueta && fc1_en_etiqueta) {
      fc1_vio_etiqueta = true;
    }

      // PASO 2: Parar cuando: vimos ETIQUETA + ahora estamos en HOME estable
      if (fc1_vio_etiqueta && fc1_en_home && isStableLevel(PIN_FC1, FCentreetiquetas ? HIGH : LOW)) {
        mover1 = false;
        etiquetapuesta = true;
        digitalWrite(PIN_MOTOR_ETI, LOW);
        etiqueta_colocada = millis();
        inicio_motor_etiqueta = 0;
      }
    }
  }

  // 5) Motor contras (entre=HIGH)
  // GARANTÍA: P27 solo puede ir a HIGH si hay botella detectada
  if (!detectada_botella) {
    digitalWrite(PIN_MOTOR_CON, LOW);
  } else if (mover2 && etiquetapuesta && llegada_botella &&
             millis() > (etiqueta_colocada + delay_etiqueta_contra)) {
    // TRIPLE GARANTÍA: Verificar estado ANTES de subir
    if (detectada_botella && mover2) {
      digitalWrite(PIN_MOTOR_CON, HIGH);
    } else {
      // Si algo está mal, FORZAR LOW inmediatamente
      digitalWrite(PIN_MOTOR_CON, LOW);
      mover2 = false;
    }
    if (inicio_motor_contra == 0) inicio_motor_contra = millis();

    // NUEVO: Verificar timeout (5 segundos máximo)
    if ((millis() - inicio_motor_contra) > TIMEOUT_MOTOR_MS) {
      digitalWrite(PIN_MOTOR_CON, LOW);
      digitalWrite(PIN_ACTUADOR, LOW);
      actuador_fuera = false;
      detectada_botella = false;
      mover2 = false;
      etiquetapuesta = false;
      contrapuesta = false;
      fc1_vio_etiqueta = false;
      fc2_vio_etiqueta = false;
      error_fc2_timeout = true;
      need_full_redraw = true;
    } else {
      // Continuar con detección si no hay timeout
      // UNA sola lectura digital rápida
      bool fc2_high = (digitalRead(PIN_FC2) == HIGH);
      bool fc2_en_etiqueta = (fc2_high != FCentrecontras);
      bool fc2_en_home = (fc2_high == FCentrecontras);

      // PASO 1: Registrar si vimos la contraetiqueta (cambio respecto a HOME)
      if (!fc2_vio_etiqueta && fc2_en_etiqueta) {
        fc2_vio_etiqueta = true;
      }

      // PASO 2: Parar cuando: vimos CONTRAETIQUETA + ahora estamos en HOME estable
      if (fc2_vio_etiqueta && fc2_en_home && isStableLevel(PIN_FC2, FCentrecontras ? HIGH : LOW)) {
        mover2 = false;
        contrapuesta = true;
        digitalWrite(PIN_MOTOR_CON, LOW);
        contra_colocada = millis();
        inicio_motor_contra = 0;
        tiempo_etiquetado = (contra_colocada - llegada_botella) / 1000.0;
      }
    }
  }

  // 6) Parar actuador
  if (etiquetapuesta && contrapuesta &&
      now > etiqueta_colocada + tiempo_parada_actuador &&
      now > contra_colocada + tiempo_parada_actuador) {

    // Asegurar tiempo_total aunque no haya contra
    unsigned long fin_ciclo = (contra_colocada != 0) ? contra_colocada : etiqueta_colocada;
    tiempo_etiquetado = (fin_ciclo - llegada_botella) / 1000.0;

    digitalWrite(PIN_ACTUADOR, LOW);
    actuador_fuera = false;
    detectada_botella = false;
    botella_detectada_previa = false;
    llegada_botella = etiqueta_colocada = contra_colocada = 0;
    tiempo_actuador_fuera = 0;
    inicio_motor_etiqueta = inicio_motor_contra = 0;
    etiquetapuesta = contrapuesta = false;
    fc1_vio_etiqueta = false;
    fc2_vio_etiqueta = false;
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

    // Mostrar estado estable en LCD (bloquea ~10 ms como máximo)
    int fc1v = isStableHigh(PIN_FC1) ? 1 : (isStableLow(PIN_FC1) ? 0 : (digitalRead(PIN_FC1)==HIGH));
    int fc2v = isStableHigh(PIN_FC2) ? 1 : (isStableLow(PIN_FC2) ? 0 : (digitalRead(PIN_FC2)==HIGH));
    if (fc1v != last_fc1) { lcd_print_padded(4,1,  fc1v ? "HIGH" : "LOW ", 4); last_fc1 = fc1v; }
    if (fc2v != last_fc2) { lcd_print_padded(14,1, fc2v ? "HIGH" : "LOW ", 4); last_fc2 = fc2v; }

    // Mostrar estado de P26 en esquina superior derecha
    int p26v = digitalRead(PIN_MOTOR_ETI) == HIGH ? 1 : 0;
    if (p26v != last_p26_state) {
      lcd.setCursor(18,0);
      lcd.print(p26v);
      last_p26_state = p26v;
    }

    if (ajustes_activos) {
      // MODO SET: Valores desplazados a la derecha (col 16) para evitar píxeles dañados
      if (delay_actuador_preview != last_TAct) {
        lcd_print_int(15, 2, delay_actuador_preview, 4);
        last_TAct = delay_actuador_preview;
      }
      if (delay_contra_preview   != last_TCtE) {
        lcd_print_int(15, 3, delay_contra_preview, 4);
        last_TCtE = delay_contra_preview;
      }
    } else {
      // MODO NORMAL: Valores desplazados a la derecha para evitar píxeles dañados (cols 11-14)
      if (delay_botella_actuador != last_TAct) {
        lcd_print_int(15, 2, delay_botella_actuador, 4);
        last_TAct = delay_botella_actuador;
      }
      if (fabs(tiempo_etiquetado - last_TTotal) > 0.009f) {
        lcd_print_float(15, 3, tiempo_etiquetado, 7, 2);
        last_TTotal = tiempo_etiquetado;
      }
    }
  }
}
