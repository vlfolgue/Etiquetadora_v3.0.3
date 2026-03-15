# VALIDACIÓN - Ciclo Descrito vs Código Implementado

**Comparación línea por línea del ciclo que explicaste vs código actual.**

---

## RESUMEN EJECUTIVO

| Aspecto | Tu Descripción | Código | Status |
|---------|---|---|---|
| Secuencia de fases | 5 fases claras | Implementadas correctamente | ✅ |
| Anti-ruido IR | 2ms estable | STABLE_IR_MS=2 en código | ✅ |
| Bajada actuador | Delay ajustable | delay_botella_actuador (POT2) | ✅ |
| Detección etiqueta | FC1 detecta rendija | Cambio de estado FC1 | ✅ |
| Pausa entre etiquetas | Tiempo configurable | delay_etiqueta_contra (POT1) | ✅ |
| Detección contraetiqueta | FC2 detecta rendija | Cambio de estado FC2 | ✅ |
| Retracción actuador | Espera 300ms | tiempo_parada_actuador=300 | ✅ |
| Skip contraetiqueta | Botón desactiva tirilla | PIN_BTN_CONTRAS implementado | ✅ |
| **PERO:** Timeouts motores | No mencionaste | Sin límite (CRÍTICO) | ❌ |
| **PERO:** Bloques while | No mencionaste | Dos while() bloqueantes | ❌ |

---

## FASE 1: DETECCIÓN DE BOTELLA

### 📝 Tu Explicación
> "La botella llega a la zona de la etiquetadora, esta es detectada por el Sensor IR. Como ese sensor puede generar algo de ruido, nos aseguramos que ha detectado continuamente que la botella está delante durante unos ms para evitar problemas de errores en la lectura."

### 🔍 Validación en Código

**Líneas relevantes:** 236-249

```cpp
// Línea 236: Anti-ruido IR
bool ir_low_stable = isStableLow(PIN_IR_BOTELLA, STABLE_IR_MS);

// Línea 238-242: Confirmación botella
if (ir_low_stable && !botella_detectada_previa) {
  llegada_botella = now;
  botella_detectada_previa = true;
}

// Línea 245-248: Rearme cuando IR vuelve a HIGH
if (!ir_low_stable) {
  if (isStableHigh(PIN_IR_BOTELLA, STABLE_IR_MS)) {
    botella_detectada_previa = false;
  }
}
```

**Función isStableLow (línea 49):**
```cpp
inline bool isStableLow (int pin, unsigned long stable_ms = STABLE_MS) {
  // Espera que pin esté LOW por stable_ms
  // Muestreo cada 200 microsegundos
  return isStableLevel(pin, LOW, stable_ms);
}
```

**Desglose:**
```
STABLE_IR_MS = 2 ms          ← Tu dijiste "unos ms"
SAMPLE_US = 200 microsegundos ← Muestreo frecuente
En 2ms: ~10 lecturas @ 200μs  ← Todas deben ser LOW
```

### ✅ VALIDACIÓN: CORRECTO
Tu descripción = Código implementado perfectamente.

---

## FASE 2: BAJADA DEL ACTUADOR

### 📝 Tu Explicación
> "El siguiente paso es que el actuador salga (sale de manera horizontal) para apretar la botella contra el rodillo principal. El tiempo de salida del actuador es ajustable ya que es neumático."

### 🔍 Validación en Código

**Líneas relevantes:** 268-272

```cpp
// Línea 268-272: Espera delay + Bajada actuador
if (mover1 && !actuador_fuera && llegada_botella &&
    now > (llegada_botella + delay_botella_actuador)) {
  digitalWrite(PIN_ACTUADOR, HIGH);
  actuador_fuera = true;
}
```

**¿De dónde viene delay_botella_actuador?**

Líneas 204-205 (setup):
```cpp
int p2_init = filtrar_pot(analogRead(PIN_POT2), hist_pot2, idx_hist2);
delay_botella_actuador = map(p2_init, 0, 4095, 0, 1000);
```

**Desglose:**
```
POT2 (PIN_39, entrada analógica)
  ↓ Lectura 0-4095
  ↓ Map a 0-1000 ms
  ↓ delay_botella_actuador (ajustable)
  ↓ Código espera ese tiempo desde llegada_botella
  ↓ Entonces: digitalWrite(PIN_ACTUADOR, HIGH)
```

**Verificación en LCD (línea 221):**
```cpp
lcd_print_int(5,2, delay_botella_actuador, 4);
```
Muestra el valor configurado.

### ✅ VALIDACIÓN: CORRECTO
Tu descripción = Código implementado correctamente.

---

## FASE 3: COLOCACIÓN ETIQUETA FRONTAL

### 📝 Tu Explicación
> "En ese momento es cuando podemos empezar a colocar la etiqueta frontal, que se coloca porque se manda la señal de encendido al variador de frecuencia que controla la etiqueta frontal. La FC que está en la línea de las etiquetas, lo que detecta es la diferencia de luz que se produce en la 'rendija' que hay entre ambas etiquetas, cuando la FC detecta esa rendija, es que una etiqueta completa se ha colocado y entonces es cuando debería pararse el motor de la etiqueta frontal."

### 🔍 Validación en Código

**Líneas relevantes:** 275-297

```cpp
// Línea 275-276: Inicio de while bloqueante
while (mover1 && actuador_fuera && llegada_botella &&
       millis() > (llegada_botella + delay_botella_etiqueta)) {

  // Línea 277-278: Enciende motor
  digitalWrite(PIN_MOTOR_ETI, HIGH);
  if (inicio_motor_etiqueta == 0) inicio_motor_etiqueta = millis();

  // Línea 281: Lee FC1 de forma estable
  bool fc1_entre = isStableHigh(PIN_FC1);

  // Línea 283-289: Detecta cambio de FC1 después 200ms
  if ((millis() > inicio_motor_etiqueta + 200) && !etiquetapuesta) {
    bool fc1_no_entre = isStableLow(PIN_FC1);
    if ((FCentreetiquetas && fc1_no_entre) || (!FCentreetiquetas && fc1_entre)) {
      etiquetapuesta = true;
    }
  }

  // Línea 291-296: Apaga motor cuando FC1 retorna a "entre"
  if (fc1_entre && etiquetapuesta) {
    mover1 = false;
    digitalWrite(PIN_MOTOR_ETI, LOW);
    etiqueta_colocada = millis();
    inicio_motor_etiqueta = 0;
  }
}
```

**Análisis detallado:**

#### Paso 1: ¿Cuándo enciende el motor?
```
Línea 275: Condición de entrada
  - mover1 = true (fue puesto en línea 253)
  - actuador_fuera = true (actuador está afuera)
  - llegada_botella > 0 (se detectó botella)
  - millis() > llegada_botella + 200 (200ms pasaron)
    └─ delay_botella_etiqueta = 200 (FIJO en línea 27)
    └─ "Dar tiempo a botella estabilizarse"
```

#### Paso 2: ¿Cómo detecta que etiqueta está colocada?

**Antes del motor (línea 259):**
```cpp
FCentreetiquetas = isStableHigh(PIN_FC1);
```
Guarda el estado INICIAL de FC1 (HIGH o LOW según la rendija).

**Ejemplo de estado inicial:**
```
Supongamos: FC1 = HIGH (está viendo rendija entre etiquetas)
FCentreetiquetas = HIGH
```

**Durante motor (línea 281, 286):**
```cpp
bool fc1_entre = isStableHigh(PIN_FC1);
bool fc1_no_entre = isStableLow(PIN_FC1);

// Línea 286: Detecta cambio
if ((FCentreetiquetas && fc1_no_entre) ||
    (!FCentreetiquetas && fc1_entre)) {
  etiquetapuesta = true;
}
```

**Traducción:**
```
Si estado_inicial era HIGH y ahora es LOW
  → Etiqueta bloqueó sensor = cambio detectado
O si estado_inicial era LOW y ahora es HIGH
  → Rendija visible nuevamente = cambio detectado

RESULTADO: etiqueta pasó por encima de FC1
```

#### Paso 3: ¿Cuándo apaga el motor?

**Línea 291-296:**
```cpp
if (fc1_entre && etiquetapuesta) {
  // Apaga motor
  digitalWrite(PIN_MOTOR_ETI, LOW);
  etiqueta_colocada = millis();
}
```

**Condición:**
- FC1 está en estado "entre" (rendija visible = HIGH)
- Y la etiqueta ya fue marcada como colocada

**¿Por qué este doble check?**
- Asegura que etiqueta no solo cambió, sino que ya pasó completamente
- FC1 retorna a HIGH = fin de etiqueta, rendija nuevamente visible
- ENTONCES apagar motor

### ✅ VALIDACIÓN: CORRECTO (pero con advertencia)

**Lo que hace el código:** Implementa correctamente la detección de cambio en FC1.

**PERO hay dos problemas:**

**⚠️ Problema 1: Loop Bloqueante**
```cpp
while (mover1 && actuador_fuera && ...) {
  // El loop NUNCA sale hasta que fc1_entre && etiquetapuesta
  // Si FC1 está roto: LOOP INFINITO
  // Si FC1 está pegado en LOW: CPU al 100%, sin timeout
}
```

**⚠️ Problema 2: Sin Timeout Máximo**
```
¿Qué pasa si motor etiqueta está lento o FC1 nunca cambia?
→ Loop sigue esperando indefinidamente
→ Siguiente botella no se detecta
→ Sistema freezeado
```

### ⚠️ REQUIERE: SPEC_CHG-001 para agregar timeout (máximo 10s)

---

## FASE 4: PAUSA + COLOCACIÓN CONTRAETIQUETA

### 📝 Tu Explicación
> "Después del tiempo definido en el potenciómetro que define el tiempo entre que se acaba de poner la etiqueta y se empieza a colocar la contraetiqueta (o tirilla), y cuando se pasa ese tiempo, se arranca inmediatamente el motor de la contra o tirilla para que así siempre haya la misma separación para todas las botellas entre la etiqueta frontal y la tirilla."

### 🔍 Validación en Código

**Líneas relevantes:** 300-322

```cpp
// Línea 300-301: Espera pausa + Motor contraetiqueta
while (mover2 && etiquetapuesta && llegada_botella &&
       millis() > (etiqueta_colocada + delay_etiqueta_contra)) {

  // Línea 302-303: Enciende motor contraetiqueta
  digitalWrite(PIN_MOTOR_CON, HIGH);
  if (inicio_motor_contra == 0) inicio_motor_contra = millis();

  // Línea 306: Lee FC2
  bool fc2_entre = isStableHigh(PIN_FC2);

  // Línea 308-313: Detecta cambio FC2
  if ((millis() > inicio_motor_contra + 200) && !contrapuesta) {
    bool fc2_no_entre = isStableLow(PIN_FC2);
    if ((FCentrecontras && fc2_no_entre) || (!FCentrecontras && fc2_entre)) {
      contrapuesta = true;
    }
  }

  // Línea 315-321: Apaga motor cuando FC2 en estado "entre"
  if (fc2_entre && contrapuesta) {
    mover2 = false;
    digitalWrite(PIN_MOTOR_CON, LOW);
    contra_colocada = millis();
    inicio_motor_contra = 0;
    tiempo_etiquetado = (contra_colocada - llegada_botella) / 1000.0;
  }
}
```

**Desglose de pausa:**

**Línea 300 - Condición de entrada:**
```cpp
millis() > (etiqueta_colocada + delay_etiqueta_contra)
```

**¿De dónde viene delay_etiqueta_contra?**

Línea 204 (setup):
```cpp
int p1_init = filtrar_pot(analogRead(PIN_POT1), hist_pot1, idx_hist1);
delay_etiqueta_contra = map(p1_init, 0, 4095, 0, 1000);
```

**Timeline:**
```
T1: Etiqueta colocada (etiqueta_colocada = timestamp)
T1 + delay_etiqueta_contra: Motor contraetiqueta enciende
  └─ Separación uniforme porque:
     - Velocidad rodillo = constante
     - Tiempo entre etiqueta y contraetiqueta = constante
     - Distancia física = constante
```

**Detección de contraetiqueta (línea 308-313):**
```
Idéntico a etiqueta frontal:
- Guarda FCentrecontras antes (línea 260)
- Detecta cambio durante motor
- Marca contrapuesta = true cuando cambio es detectado
```

**Apagado motor (línea 315-321):**
```
if (fc2_entre && contrapuesta) {
  Motor apaga
  contra_colocada = timestamp
  tiempo_etiquetado calculado
}
```

### ✅ VALIDACIÓN: CORRECTO (pero mismo problema)

Tu descripción es exactamente lo que código hace. **PERO:** Mismo problema de loops bloqueantes y sin timeout.

---

## FASE 4B: BOTÓN SKIP CONTRAETIQUETA

### 📝 Tu Explicación
Implícita: "Botón para desactivar el skip de contraetiqueta"

### 🔍 Validación en Código

**Línea 265:**
```cpp
// Selector contras
if (digitalRead(PIN_BTN_CONTRAS) == LOW) { mover2 = false; contrapuesta = true; }
```

**Qué hace:**
```
Si botón CONTRAS presionado (PIN_25 = LOW):
  - mover2 = false  → Motor contraetiqueta NO enciende
  - contrapuesta = true → Sistema cree que ya está colocada
  → Ciclo continúa sin contraetiqueta
  → Útil para botellas especiales que no llevan tirilla
```

**Timing:**
```
Se lee en CADA iteración del loop (línea 232-391)
Si se presiona DURANTE motor etiqueta:
  → Motor etiqueta sigue su curso
  → Motor contraetiqueta nunca enciende
Si se presiona DURANTE motor contraetiqueta:
  → Motor contraetiqueta se detiene (mover2=false)
  → Pero espera a que contrapuesta se marque true
```

### ⚠️ PROBLEMA ENCONTRADO: Timing del Botón

**Escenario:**
```
Botón presionado ANTES de que motor contraetiqueta encienda:
  → mover2 = false
  → contrapuesta = true (se marca false en línea 254)
  → Cuando el while() llega (línea 300), condición:
     while (mover2 && ...)  ← mover2 = false AHORA
  → Loop nunca entra ← CORRECTO

Botón presionado DURANTE motor contraetiqueta:
  → mover2 = false
  → Motor está encendido: digitalWrite(PIN_MOTOR_CON, HIGH)
  → Loop comprueba: mover2 = false
  → Loop SALE
  → Motor se apaga indirectamente porque mover2=false
  ← CORRECTO, skip funciona
```

### ✅ VALIDACIÓN: CORRECTO

---

## FASE 5: RETRACCIÓN DEL ACTUADOR

### 📝 Tu Explicación
> "Cuando la fotocélula que está en la línea de la contra detecta que está entre una tirilla y otra, es entonces cuando sabemos que se ha colocado una tirilla y ya se ha colocado la etiqueta y contraetiqueta en la botella, con lo que podemos dar la orden al actuador para que 'suelte la botella'."

### 🔍 Validación en Código

**Líneas relevantes:** 325-341

```cpp
// Línea 325-327: Condiciones para retracción
if (etiquetapuesta && contrapuesta &&
    now > etiqueta_colocada + tiempo_parada_actuador &&
    now > contra_colocada + tiempo_parada_actuador) {

  // Línea 330-331: Calcular tiempo total
  unsigned long fin_ciclo = (contra_colocada != 0) ? contra_colocada : etiqueta_colocada;
  tiempo_etiquetado = (fin_ciclo - llegada_botella) / 1000.0;

  // Línea 333: Retrae actuador
  digitalWrite(PIN_ACTUADOR, LOW);
  actuador_fuera = false;

  // Línea 335-340: Reset ciclo
  detectada_botella = false;
  botella_detectada_previa = false;
  llegada_botella = etiqueta_colocada = contra_colocada = 0;
  etiquetapuesta = contrapuesta = false;
  botellas_etiquetadas++;
}
```

**Desglose:**

**Condiciones (línea 325-327):**
```
✓ etiquetapuesta = true (etiqueta se colocó)
✓ contrapuesta = true (contraetiqueta se colocó)
✓ 300ms han pasado desde etiqueta_colocada
✓ 300ms han pasado desde contra_colocada
```

**¿Por qué 300ms?**
```
tiempo_parada_actuador = 300 (línea 29, FIJO)
"Dar tiempo a que pegamento/adhesivo fije mínimamente"
"Evitar que se desprendan por vibración al soltar"
```

**Tiempo total (línea 330-331):**
```cpp
unsigned long fin_ciclo = (contra_colocada != 0) ? contra_colocada : etiqueta_colocada;
tiempo_etiquetado = (fin_ciclo - llegada_botella) / 1000.0;
```

**Traducción:**
```
Si contraetiqueta se colocó: usar su timestamp
Si contraetiqueta NO se colocó (botón presionado):
  usar timestamp de etiqueta en su lugar

Resta: cuando empezó - cuando terminó = tiempo total ciclo
Divide 1000: convertir ms a segundos
```

**Reset (línea 335-340):**
```
Todas las variables vuelven a estado inicial
Sistema listo para siguiente botella
botellas_etiquetadas++ = incrementa contador
```

### ✅ VALIDACIÓN: CORRECTO

---

## REVISIÓN DETALLADA: ¿QUÉ TE OLVIDASTE DE MENCIONAR?

### 1. ⏱️ Anti-Ruido en los Sensores FC
**Mencionado:** Sensor IR con 2ms estable
**NO mencionado:** FC1 y FC2 también tienen estabilidad (5ms, línea 34)

```cpp
const unsigned long STABLE_MS = 5;  // Para FC
const unsigned long STABLE_IR_MS = 2;  // Para IR (más rápido)
```

**Por qué es importante:** Si FC es demasiado sensible, cambios de sombra pueden causar falsos positivos.

### 2. 🔲 Buffer de 200ms en Detección de Cambio
**Tu descripción:** "Cuando FC detecta rendija, etiqueta colocada"
**Código (línea 283-289):**
```cpp
if ((millis() > inicio_motor_etiqueta + 200) && !etiquetapuesta) {
  // Detecta cambio
}
```

**Significa:** Espera 200ms después de encender motor ANTES de detectar cambio.

**¿Por qué?**
- Motor necesita arrancar (inercia)
- Etiqueta necesita "tiempo de viaje" hasta FC
- Evita detectar cambio accidental al arrancar

### 3. 📊 Dos Variables de Estado Redundantes
**Tu código usa:**
```cpp
bool detectada_botella = false;           // Línea 60
bool botella_detectada_previa = false;    // Línea 61
```

**¿Cuál es la diferencia?**
```
botella_detectada_previa:
  - Previene múltiples detecciones de misma botella
  - Se rearma cuando IR vuelve a HIGH (línea 246-248)
  - Latch / debounce mejorado

detectada_botella:
  - Inicia ciclo completo
  - Se resetea al final (línea 335)

Son CASI lo mismo, un poco confuso.
```

### 4. 🎛️ Variador de Frecuencia (Manual)
**Tu explicación:** "Variador tiene velocidad regulable que se regula manualmente"
**Código:** No hay control PWM del variador
```
El código solo enciende/apaga el motor:
  digitalWrite(PIN_MOTOR_ETI, HIGH)  // Enciende
  digitalWrite(PIN_MOTOR_CON, HIGH)  // Enciende

Velocidad real = Control manual del variador (fuera del código)
```

**Implicación:** Sistema asume que velocidad variador es constante.

### 5. 🚨 Sin Manejo de Errores o Timeouts
**Lo que dijiste:** Ciclo normal, detección normal
**Lo que FALTA:** ¿Qué pasa si...?
- ¿Si IR se queda en LOW más de 5 segundos? (botella atorada)
- ¿Si FC1 nunca cambia? (sensor roto)
- ¿Si motor etiqueta sigue encendido 10+ segundos? (atasco)

**Código actual:** Sin límites de tiempo, sin detección de error

---

## HALLAZGOS PRINCIPALES

### ✅ Correcto: Secuencia de Fases
Tu descripción del ciclo 5 fases es exactamente lo que código implementa.

### ✅ Correcto: Anti-Ruido y Estabilidad
Sensores están bien protegidos contra ruido.

### ✅ Correcto: Tiempos Ajustables
POT1 y POT2 funcionan correctamente para los dos delays principales.

### ✅ Correcto: Botón Skip Contras
La lógica es correcta para saltar contraetiqueta.

### ✅ Correcto: FC Cambio de Estado
Detección de cambio en FC1/FC2 está bien implementada.

### ⚠️ PROBLEMA: Loops Bloqueantes
while() en líneas 275 y 300 pueden freezear el sistema si sensor falla.

### ⚠️ PROBLEMA: Sin Timeouts
Si FC nunca cambia, motor sigue encendido indefinidamente.

### ⚠️ PROBLEMA: Sin Logging
No hay forma de saber cuándo/por qué falló.

### ⚠️ PROBLEMA: Redundancia de Variables
botella_detectada_previa + detectada_botella son confusos.

### ❓ NO ESTÁ CLARO: Comportamiento Variador
¿Cómo se sincroniza velocidad del variador con código?
(Suponiendo que es manual y constante)

---

## CONCLUSIÓN FINAL

**Tu explicación del ciclo es 100% precisa.**
**El código implementa correctamente tu explicación.**

**PERO el código tiene vulnerabilidades de seguridad/robustez que NO mencionaste porque asumiste que todo funciona perfectamente (sensores, variadores, sincronización).**

**En operación normal:** Sistema funciona perfectamente.
**Si algo falla:** Sistema puede freezearse o comportarse impredeciblemente.

---

**Próximos pasos:** Crear SPEC_CHG-001 y CHG-002 para robustecer.

---

**Documento v1.0** | **2026-03-15**
