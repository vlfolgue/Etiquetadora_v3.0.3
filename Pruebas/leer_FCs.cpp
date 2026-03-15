#include <Arduino.h>

// Pines actuales de las fotocélulas
const int pin_LDR1 = 34;
const int pin_LDR2 = 35;

// Control de lectura
const int num_muestras = 10000;
int datos_FC1[num_muestras];
int datos_FC2[num_muestras];
int indice = 0;

unsigned long ultima_lectura = 0;
const unsigned long intervalo_lectura = 5; // cada 5 ms

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(pin_LDR1, INPUT);
  pinMode(pin_LDR2, INPUT);

  Serial.println("FC1,FC2");  // Cabecera para el Plotter
}

void loop() {
  if (millis() - ultima_lectura >= intervalo_lectura && indice < num_muestras) {
    ultima_lectura = millis();

    int lectura1 = analogRead(pin_LDR1);
    int lectura2 = analogRead(pin_LDR2);

    datos_FC1[indice] = lectura1;
    datos_FC2[indice] = lectura2;

    // Imprimir lecturas sin filtrar
    Serial.print(lectura1);
    Serial.print(",");
    Serial.println(lectura2);

    indice++;
  }

  if (indice >= num_muestras) {
    Serial.println("✅ Lectura finalizada.");
    while (true);
  }
}
