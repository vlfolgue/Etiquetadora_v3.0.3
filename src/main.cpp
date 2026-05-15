#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/* =========================
   PRUEBA DE VARIADORES
   ========================= */
// Mismos pines que producción
const int PIN_SDA         = 21;
const int PIN_SCL         = 22;
const int PIN_MOTOR_ETI   = 26;   // Variador etiquetas
const int PIN_MOTOR_CON   = 27;   // Variador contras
const int PIN_ACTUADOR    = 16;
const int PIN_BTN_AJUSTES = 19;   // Botón A → controla motor ETI
const int PIN_BTN_CONTRAS = 25;   // Botón B → controla motor CON

LiquidCrystal_I2C lcd(0x27, 20, 4);

/* =========================
   ESTADO
   ========================= */
bool motor_eti_on  = false;
bool motor_con_on  = false;

// Debounce simple para cada botón
struct Boton {
  int pin;
  bool estado;
  bool ultimo_leido;
  unsigned long ultimo_cambio;
};

Boton btn_eti = {PIN_BTN_AJUSTES, false, false, 0};
Boton btn_con = {PIN_BTN_CONTRAS, false, false, 0};

const unsigned long DEBOUNCE_MS = 30;

bool leer_boton(Boton &b) {
  bool lectura = (digitalRead(b.pin) == HIGH);
  unsigned long now = millis();
  if (lectura != b.ultimo_leido) {
    b.ultimo_cambio = now;
    b.ultimo_leido = lectura;
  }
  if ((now - b.ultimo_cambio) > DEBOUNCE_MS && lectura != b.estado) {
    b.estado = lectura;
    return true;  // cambio confirmado
  }
  return false;
}

void actualizar_lcd() {
  lcd.setCursor(0, 0); lcd.print("== PRUEBA VARIADORES==");
  lcd.setCursor(0, 1); lcd.print("BtnA=ETI  BtnB=CON  ");
  lcd.setCursor(0, 2); lcd.print("ETI P26:");
  lcd.setCursor(9, 2); lcd.print(motor_eti_on ? " ON  " : " OFF ");
  lcd.setCursor(0, 3); lcd.print("CON P27:");
  lcd.setCursor(9, 3); lcd.print(motor_con_on ? " ON  " : " OFF ");
}

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);
  lcd.begin(20, 4);
  lcd.backlight();
  lcd.clear();

  // Pines de salida — arranca siempre en LOW
  pinMode(PIN_MOTOR_ETI, OUTPUT); digitalWrite(PIN_MOTOR_ETI, LOW);
  pinMode(PIN_MOTOR_CON, OUTPUT); digitalWrite(PIN_MOTOR_CON, LOW);
  pinMode(PIN_ACTUADOR,  OUTPUT); digitalWrite(PIN_ACTUADOR,  LOW);

  // Pines de entrada
  pinMode(PIN_BTN_AJUSTES, INPUT);
  pinMode(PIN_BTN_CONTRAS, INPUT);

  actualizar_lcd();
  Serial.println("== PRUEBA VARIADORES ==");
  Serial.println("BtnA (pin 19) -> Motor ETI (pin 26)");
  Serial.println("BtnB (pin 25) -> Motor CON (pin 27)");
  Serial.println("Pulsa para ON, vuelve a pulsar para OFF");
}

void loop() {
  // Botón A → toggle motor ETI
  if (leer_boton(btn_eti) && btn_eti.estado) {
    motor_eti_on = !motor_eti_on;
    digitalWrite(PIN_MOTOR_ETI, motor_eti_on ? HIGH : LOW);
    Serial.printf("Motor ETI (P26): %s\n", motor_eti_on ? "ON" : "OFF");
    actualizar_lcd();
  }

  // Botón B → toggle motor CON
  if (leer_boton(btn_con) && btn_con.estado) {
    motor_con_on = !motor_con_on;
    digitalWrite(PIN_MOTOR_CON, motor_con_on ? HIGH : LOW);
    Serial.printf("Motor CON (P27): %s\n", motor_con_on ? "ON" : "OFF");
    actualizar_lcd();
  }
}
