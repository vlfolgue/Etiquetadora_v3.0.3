#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Dirección I2C habitual: 0x27 o 0x3F
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Wire.begin(22, 21);         // ESP32: SDA = 22, SCL = 21
  lcd.begin(20, 4);           // 20 columnas, 4 filas
  lcd.backlight();            // Activa la luz de fondo
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("LCD 20x4 funcionando");

  lcd.setCursor(0, 1);
  lcd.print("Linea 2");

  lcd.setCursor(0, 2);
  lcd.print("Linea 3");

  lcd.setCursor(0, 3);
  lcd.print("Linea 4");
}

void loop() {
  // Aquí puedes poner lecturas, actualizaciones, etc.
}
