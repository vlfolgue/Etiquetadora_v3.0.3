# ESPECIFICACIÓN DETALLADA - CICLO DE OPERACIÓN

**Documento de especificación del ciclo completo de etiquetado de la máquina.**

---

## 1. RESUMEN EJECUTIVO

La máquina etiquetadora funciona con un **ciclo de 5 fases** que atrapa, gira, etiqueta, contra-etiqueta y suelta botellas. Cada fase depende de sensores ópticos (IR y fotocélulas) y tiempos ajustables.

---

## 2. DESCRIPCIÓN FÍSICA DEL SISTEMA

### 2.1 Componentes Principales
- **Sensor IR:** Detecta presencia de botella en zona de entrada
- **Rodillo Principal:** Gira botellas continuamente (siempre girando)
- **Actuador Neumático:** Presiona botella contra rodillo (sale/entra horizontalmente)
- **Motor Etiqueta Frontal:** Controla velocidad de colocación etiqueta frontal (variador de frecuencia)
- **Motor Contraetiqueta/Tirilla:** Controla velocidad de colocación tirilla (variador de frecuencia)
- **FC1 (Fotocélula Etiqueta):** Detecta rendija entre etiquetas frontales
- **FC2 (Fotocélula Contraetiqueta):** Detecta rendija entre tirillas

### 2.2 Movimiento de la Botella
```
Zona Libre (sin botella)
    ↓ [IR detecta botella]
Botella Atrapada (actuador presiona)
    ↓ [Gira con rodillo]
Etiqueta Frontal Se Coloca
    ↓ [FC1 detecta rendija = etiqueta completa]
Pausa (tiempo configurable entre etiqueta y tirilla)
    ↓ [Tiempo potenciómetro cumplido]
Contraetiqueta/Tirilla Se Coloca
    ↓ [FC2 detecta rendija = tirilla completa]
Botella Suelta
    ↓ [Actuador retrae]
Zona Libre Nuevamente
```

---

## 3. CICLO DETALLADO POR FASES

### FASE 1: DETECCIÓN DE BOTELLA
**Duración:** Variable (depende de velocidad línea)
**Sensor:** IR (PIN_IR_BOTELLA)
**Objetivo:** Confirmar presencia de botella con anti-ruido

#### 3.1.1 Condición Inicial
- Actuador retraído (LOW)
- Ambos motores apagados
- Sistema en espera

#### 3.1.2 Evento Disparador
- Sensor IR detecta botella = nivel LOW
- **Anti-ruido:** Debe estar en LOW de forma estable durante STABLE_IR_MS (2 ms)
- Muestreo: cada 200 μs durante ventana de estabilidad

#### 3.1.3 Confirmación
```
Si IR está LOW por 2+ ms sin interrupciones
  → Botella confirmada
  → Registrar tiempo: llegada_botella = millis()
  → Pasar a FASE 2
Si IR vuelve a HIGH antes de 2ms
  → Falsa alarma (ruido)
  → Esperar siguiente botella
```

#### 3.1.4 Estado al Final de Fase
- `botella_detectada_previa = true` (para rearme)
- `detectada_botella = true` (inicia ciclo)
- `llegada_botella = [timestamp]`

---

### FASE 2: BAJADA DEL ACTUADOR
**Duración:** delay_botella_actuador (0-1000 ms, ajustable por POT2)
**Actuador:** PIN_ACTUADOR = HIGH
**Objetivo:** Atrapar botella contra rodillo principal

#### 3.2.1 Espera Inicial
```
Esperar: delay_botella_actuador ms desde llegada_botella
Ejemplo: Si delay=200ms
  T0: botella detectada
  T0+200ms: iniciar bajada actuador
```

#### 3.2.2 Acción
```
if (millis() > llegada_botella + delay_botella_actuador) {
  digitalWrite(PIN_ACTUADOR, HIGH)  // Actuador sale
  actuador_fuera = true
}
```

#### 3.2.3 Física
- Actuador sale horizontalmente
- Presiona botella contra rodillo
- Botella comienza a girar verticalmente
- Rodillo principal gira continuamente

#### 3.2.4 Estado al Final de Fase
- `actuador_fuera = true`
- Botella está girando
- Listo para etiqueta frontal

---

### FASE 3: COLOCACIÓN ETIQUETA FRONTAL
**Duración:** Variable (depende velocidad de colocación etiqueta)
**Sensor:** FC1 (PIN_FC1)
**Motor:** PIN_MOTOR_ETI (conectado a variador frecuencia)
**Objetivo:** Colocar etiqueta frontal completa

#### 3.3.1 Espera Previa
```
Esperar: delay_botella_etiqueta (200 ms fijo)
  Desde llegada_botella
Razón: Dar tiempo a botella estabilizarse girando
```

#### 3.3.2 Detección Estado Inicial de FC1
Antes de encender motor:
```
Leer FC1 de forma estable:
  Si HIGH → "entre etiquetas" (rendija visible)
  Si LOW  → "dentro de etiqueta" (bloqueado)
Guardar estado: FCentreetiquetas = [resultado]
```

#### 3.3.3 Encendido Motor Etiqueta
```
if (mover1 && actuador_fuera && millis() > llegada_botella + 200) {
  digitalWrite(PIN_MOTOR_ETI, HIGH)  // Motor enciende
  inicio_motor_etiqueta = millis()
  mover1 = true
}
```

**¿Qué hace el variador?**
- Gira a velocidad configurada (potenciómetro manual del variador)
- Despliega etiqueta sobre botella
- Rodillo y botella trabajan juntos para aplanar etiqueta

#### 3.3.4 Detección de Colocación Completa
**Condición:** FC1 cambia de estado inicial

```
Esperar 200ms desde inicio motor (estabilización):
  if (millis() > inicio_motor_etiqueta + 200) {
    Leer FC1 de forma estable:
      Si cambió desde FCentreetiquetas
        → Etiqueta completa colocada
        → etiquetapuesta = true
  }
```

**Ejemplo de cambio:**
- Estado inicial: FCentreetiquetas = HIGH (rendija visible)
- Después colocación: FC1 = LOW (etiqueta bloquea sensor)
- **Cambio detectado:** etiqueta ha pasado por sensor

#### 3.3.5 Apagado Motor Etiqueta
```
Si (FC1 retorna a estado "entre" && etiquetapuesta) {
  digitalWrite(PIN_MOTOR_ETI, LOW)  // Motor apaga
  mover1 = false
  etiqueta_colocada = millis()
}
```

**¿Por qué espera a que FC1 retorne a "entre"?**
- Asegura que la etiqueta está completamente en la botella
- FC1 = HIGH nuevamente = rendija visible = fin de etiqueta
- Entonces es seguro apagar motor

#### 3.3.6 Estado al Final de Fase
- `etiquetapuesta = true`
- `etiqueta_colocada = [timestamp]`
- Motor etiqueta apagado
- Listo para pausa y contraetiqueta

---

### FASE 4: PAUSA + COLOCACIÓN CONTRAETIQUETA
**Duración:** delay_etiqueta_contra (0-1000 ms, ajustable por POT1)
**Sensor:** FC2 (PIN_FC2)
**Motor:** PIN_MOTOR_CON
**Objetivo:** Dejar separación uniforme entre etiqueta frontal y contraetiqueta

#### 3.4.1 Pausa Configurable
```
Esperar: delay_etiqueta_contra ms desde etiqueta_colocada
Ejemplo: Si delay=100ms
  T1: Etiqueta frontal completa (FC1 cambió)
  T1+100ms: Iniciar motor contraetiqueta
```

**¿Por qué este tiempo?**
- Las etiquetas se colocan a velocidad constante del rodillo
- Separación física entre etiqueta y contraetiqueta es fija
- Tiempo de pausa ajusta CUÁNDO se inicia tirilla
- Así todas las botellas tienen misma separación visual

#### 3.4.2 Detección Estado Inicial de FC2
Antes de encender motor contraetiqueta:
```
Leer FC2 de forma estable:
  Si HIGH → "entre tirillas"
  Si LOW  → "dentro de tirilla"
Guardar estado: FCentrecontras = [resultado]
```

#### 3.4.3 Encendido Motor Contraetiqueta
```
if (mover2 && etiquetapuesta &&
    millis() > etiqueta_colocada + delay_etiqueta_contra) {
  digitalWrite(PIN_MOTOR_CON, HIGH)  // Motor enciende
  inicio_motor_contra = millis()
  mover2 = true
}
```

#### 3.4.4 Detección de Colocación Completa
**Idéntico a etiqueta frontal:**

```
Esperar 200ms desde inicio motor:
  Leer FC2 de forma estable:
    Si cambió desde FCentrecontras
      → Tirilla completa colocada
      → contrapuesta = true
```

#### 3.4.5 Apagado Motor Contraetiqueta
```
Si (FC2 retorna a estado "entre" && contrapuesta) {
  digitalWrite(PIN_MOTOR_CON, LOW)  // Motor apaga
  mover2 = false
  contra_colocada = millis()
}
```

#### 3.4.6 Opción: Botón Skip Contras
```
if (digitalRead(PIN_BTN_CONTRAS) == LOW) {
  mover2 = false      // No colocar tirilla
  contrapuesta = true // Simular como si estuviera colocada
}
```

**Efecto:**
- Presionar botón = saltarse contraetiqueta
- Útil para botellas que no necesitan contraetiqueta
- Ciclo continúa normalmente

#### 3.4.7 Estado al Final de Fase
- `contrapuesta = true` (o skipped)
- `contra_colocada = [timestamp]`
- Motor contraetiqueta apagado
- Listo para soltar botella

---

### FASE 5: RETRACCIÓN DEL ACTUADOR Y CICLO COMPLETO
**Duración:** tiempo_parada_actuador (300 ms fijo)
**Actuador:** PIN_ACTUADOR = LOW
**Objetivo:** Liberar botella para que continúe en la línea

#### 3.5.1 Condición para Retracción
```
Si (etiquetapuesta && contrapuesta) {
  Y (millis() > etiqueta_colocada + 300)
  Y (millis() > contra_colocada + 300)
  {
    Retrae actuador
  }
}
```

**¿Por qué 300 ms después de ambas?**
- Garantiza que ambas etiquetas están bien pegadas
- Evita que se desprenda por vibración
- Permite que adhesivo seque/fije mínimamente

#### 3.5.2 Acción
```
digitalWrite(PIN_ACTUADOR, LOW)  // Actuador retrae
actuador_fuera = false
```

#### 3.5.3 Reset de Variables
```
detectada_botella = false
botella_detectada_previa = false
llegada_botella = 0
etiqueta_colocada = 0
contra_colocada = 0
etiquetapuesta = false
contrapuesta = false
botellas_etiquetadas++
tiempo_etiquetado = (contra_colocada - llegada_botella) / 1000.0
```

#### 3.5.4 Estado al Final de Fase
- Sistema listo para siguiente botella
- Contador actualizado
- Tiempo total registrado

---

## 4. TIMELINE COMPLETO DE UN CICLO

```
T0:           IR detecta botella → Fase 1 comienza
              ↓
T0+2ms:       IR confirmado estable
              ↓
T0+[delay_ba]: Actuador sale → Fase 2
              ↓
T0+200ms:     Motor etiqueta enciende → Fase 3 comienza
              ↓
T0+200+[dur_etiq]: FC1 cambia → Etiqueta completa
                   Motor etiqueta apaga
              ↓
T0+[etiq]+[delay_ec]: Motor contraetiqueta enciende → Fase 4
              ↓
T0+[etiq]+[delay_ec]+[dur_contra]: FC2 cambia → Tirilla completa
                                    Motor contraetiqueta apaga
              ↓
T0+[etiq]+[delay_ec]+[dur_contra]+300ms: Actuador retrae
              ↓
CICLO COMPLETO: tiempo_etiquetado = T_total / 1000 segundos
```

---

## 5. PARÁMETROS AJUSTABLES

| Parámetro | Origen | Rango | Default | Ajuste |
|-----------|--------|-------|---------|--------|
| delay_botella_actuador | POT2 | 0-1000 ms | 0 | Cuándo sale actuador |
| delay_etiqueta_contra | POT1 | 0-1000 ms | 0 | Separación etiq-contra |
| Velocidad Motor Etiqueta | Variador manual | Variable | Manual | Qué tan rápido cubre |
| Velocidad Motor Contra | Variador manual | Variable | Manual | Qué tan rápido cubre |

**Fijos (no ajustables):**
- delay_botella_etiqueta = 200 ms (estabilización)
- tiempo_parada_actuador = 300 ms (pegado)
- STABLE_IR_MS = 2 ms (anti-ruido IR)

---

## 6. ANTI-RUIDO Y ESTABILIDAD

### 6.1 IR Anti-Ruido
```
Ventana: 2 ms
Muestreo: cada 200 μs
Repeticiones: ~10 lecturas
Requisito: TODAS deben ser LOW
```

**Previene:**
- Falsos positivos de variaciones lumínicas
- Detecciones múltiples de una botella
- Botella parcialmente detectada

### 6.2 FC Anti-Ruido
```
Ventana: 5 ms
Muestreo: cada 200 μs
Repeticiones: ~25 lecturas
Requisito: Lectura estable al menos una vez
```

**Previene:**
- Falsa detección de rendija (vibraciones)
- Cambios de estado ruidosos

---

## 7. VALIDACIÓN CONTRA COMPORTAMIENTO ESPERADO

### 7.1 ¿Qué Debería Pasar?

✅ **Secuencia Correcta:**
1. Botella detectada (IR LOW estable)
2. Actuador sale
3. Motor etiqueta enciende
4. FC1 cambia (etiqueta colocada)
5. Motor etiqueta apaga
6. Espera [delay_etiqueta_contra]
7. Motor contraetiqueta enciende
8. FC2 cambia (contraetiqueta colocada)
9. Motor contraetiqueta apaga
10. Espera [tiempo_parada_actuador]
11. Actuador retrae
12. Ciclo completo

### 7.2 Escenarios Especiales

#### Escenario A: Botón Contras Presionado
```
→ Motor contraetiqueta NO enciende
→ contrapuesta se marca como true sin activar motor
→ Ciclo continúa normalmente (solo etiqueta, sin contraetiqueta)
```

#### Escenario B: Velocidad de Colocación Lenta
```
→ Motor etiqueta tarda más en completarse
→ FC1 cambia después de más tiempo
→ Motor apaga cuando cambio es detectado
→ Sistema se adapta automáticamente
```

#### Escenario C: Velocidad de Colocación Muy Rápida
```
→ Motor etiqueta se completa muy rápido
→ FC1 cambia después de poco tiempo
→ Espera 200ms desde inicio = buffer de seguridad
→ Evita apagar demasiado pronto
```

---

## 8. POSIBLES PROBLEMAS O GAPS

### Gap A: ⚠️ SIN TIMEOUT EN MOTORES
**Problema:** Si FC1 nunca cambia (sensor atascado/roto)
```
Motor etiqueta sigue encendido indefinidamente
Loop bloqueado en while()
Sistema no responde
```
**Especificación Faltante:** Máximo 10 segundos de operación

### Gap B: ⚠️ SIN VALIDACIÓN DE SENSOR
**Problema:** ¿Qué si FC comienza ya en LOW?
```
¿El código detecta correctamente cambio de estado?
¿O se confunde con estado inicial?
```
**Nota:** Código guarda FCentreetiquetas/FCentrecontras antes de motor

### Gap C: ⚠️ SIN LOGGING DE EVENTOS
**Problema:** ¿Cuándo se detectó botella? ¿Por qué no completó etiqueta?
```
No hay registro de transiciones
No hay debugging information
```

### Gap D: COMPORTAMIENTO DE BOTELLA RECHAZADA
**Problema:** Si IR está en LOW durante 3+ segundos (botella atorada)
```
¿El código detecta esto?
¿Hay límite de tiempo para completar ciclo?
```
**Actual:** Solo se inicia ciclo si IR LOW, pero sin timeout máximo

### Gap E: RESET INCOMPLETO
**Problema:** Si ciclo no completa (falla FC2, botón presionado)
```
Variables se quedan en estado inconsistente
Siguiente botella podría comportarse diferente
```

---

## 9. VERIFICACIÓN CON CÓDIGO ACTUAL

### Punto de Entrada: Línea 232-392 (loop principal)

#### ✅ Fase 1 (Detección): CORRECTO
```
Línea 236: isStableLow(PIN_IR_BOTELLA, STABLE_IR_MS)
Línea 238-242: Rearme con HIGH estable
Línea 252-261: Inicio ciclo cuando IR LOW estable
```

#### ✅ Fase 2 (Actuador): CORRECTO
```
Línea 268-272: Actuador sale tras delay_botella_actuador
digitalWrite(PIN_ACTUADOR, HIGH) cuando tiempo cumplido
```

#### ✅ Fase 3 (Etiqueta): PARCIALMENTE CORRECTO
```
Línea 275-297: while() loop para motor etiqueta
Línea 259: Guarda FCentreetiquetas ANTES de motor (correcto)
Línea 281: Lee FC1 estable durante motor
Línea 286: Detecta cambio de estado (correcto)
Línea 291-296: Apaga motor cuando FC1 retorna a "entre"

⚠️ PROBLEMA:
- Loop bloqueante (línea 275) puede saturar CPU
- Sin timeout (si FC nunca cambia, loop infinito)
```

#### ✅ Fase 4 (Contraetiqueta): PARCIALMENTE CORRECTO
```
Línea 300-322: while() loop para motor contraetiqueta
Línea 260: Guarda FCentrecontras ANTES (correcto)
Línea 306: Lee FC2 estable durante motor
Línea 310: Detecta cambio de estado
Línea 315-321: Apaga motor cuando FC2 retorna a "entre"
Línea 265: Botón contras desactiva motor (correcto)

⚠️ MISMO PROBLEMA:
- Loop bloqueante
- Sin timeout
```

#### ✅ Fase 5 (Retracción): CORRECTO
```
Línea 325-341: Espera tiempo_parada_actuador
Línea 333: Actuador retrae (LOW)
Línea 337-340: Reset de variables
Línea 340: Contador de botellas
Línea 320-331: Tiempo total calculado
```

#### ✅ LCD + Ajustes: CORRECTO
```
Línea 344: Gestión de ajustes
Línea 353-391: Actualización LCD selectiva
```

---

## 10. CONCLUSIONES

### ✅ Lo que el Código Hace Bien
1. **Secuencia correcta:** Detección → Bajada → Etiqueta → Contraetiqueta → Retracción
2. **Anti-ruido:** isStableLevel() bien implementado para sensores
3. **FC1/FC2 correctamente:** Detecta cambios de estado adecuadamente
4. **Tiempos ajustables:** POT1 y POT2 configuran correctamente delays
5. **Botón contras:** Función skip contraetiqueta implementada
6. **Contador y tiempo total:** Registra métricas
7. **LCD:** Muestra información en tiempo real

### ⚠️ Lo que Falta o está Roto

**CRÍTICO:**
1. **Timeout en motores:** Sin límite máximo (10s recomendado)
2. **Loop bloqueante:** while() en líneas 275 y 300 bloquean CPU
3. **Sin manejo de errores:** Si sensor falla, sistema no responde
4. **Sin logging:** No se sabe cuándo/por qué falla

**IMPORTANTE:**
5. **Validación de estado FC:** ¿Qué si FC está roto desde inicio?
6. **Botella atascada:** Sin detección de ciclo muy largo
7. **Variables redundantes:** botella_detectada_previa + detectada_botella

### 🎯 Próximos Cambios Recomendados

1. **SPEC_CHG-001:** Agregar timeout en motores (10s)
2. **SPEC_CHG-002:** Refactorizar while() → máquina de estados
3. **SPEC_CHG-003:** Sistema de errores y logging
4. **SPEC_CHG-004:** Validación de estado inicial sensores

---

## 11. MATRIZ DE TRAZABILIDAD

| Requerimiento | Línea Código | Status | Notas |
|---------------|--------------|--------|-------|
| Detección IR estable | 236, 238-249 | ✅ | Correcto |
| Bajada actuador temporal | 268-272 | ✅ | Correcto |
| Motor etiqueta activación | 277 | ✅ | Pero en while bloqueante |
| FC1 cambio de estado | 286 | ✅ | Correcto |
| Motor etiqueta desactivación | 293 | ✅ | Correcto |
| Pausa entre etiqueta-contra | 301 | ✅ | Correcto |
| Motor contra activación | 302 | ✅ | Pero en while bloqueante |
| FC2 cambio de estado | 310 | ✅ | Correcto |
| Motor contra desactivación | 317 | ✅ | Correcto |
| Retracción actuador | 333 | ✅ | Correcto |
| Reset de ciclo | 335-340 | ✅ | Correcto |
| Timeout motores | — | ❌ | FALTA |
| Manejo de errores | — | ❌ | FALTA |
| Logging eventos | — | ❌ | FALTA |

---

## 12. RESUMEN FINAL

**El código implementa correctamente el ciclo de etiquetado descrito.** La secuencia es exacta: detección → actuador → etiqueta → pausa → contraetiqueta → retracción.

**Sin embargo, tiene vulnerabilidades críticas:**
- Loops bloqueantes pueden freezear sistema
- Sin timeouts para sensores defectuosos
- Sin forma de saber qué falló

**Funciona bien en operación normal.** Falla silenciosamente si algo sale mal.

**Recomendación:** Implementar SPEC_CHG-001 y CHG-002 para robustecer antes de pasar a producción intensiva.

---

**Especificación v1.0** | **2026-03-15**
