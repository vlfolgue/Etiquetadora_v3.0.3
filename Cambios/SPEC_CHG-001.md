# SPEC_CHG-001: Agregar Timeout en Motores - Protección contra Fallo de Sensores FC

**Versión:** 1.0
**Fecha:** 2026-03-15
**Prioridad:** CRÍTICA
**Estado:** En Implementación

---

## 1️⃣ JUSTIFICACIÓN

### Problema Actual
Sistema tiene **loops bloqueantes sin timeout** (líneas 275, 300 en main.cpp):
```cpp
while (mover1 && actuador_fuera && llegada_botella &&
       millis() > (llegada_botella + delay_botella_etiqueta)) {
  digitalWrite(PIN_MOTOR_ETI, HIGH);
  // Espera FC1 cambio...
  // ❌ SI FC1 FALLA → LOOP INFINITO
}
```

### Riesgos
1. **FC1 rota/desconectada:** Motor etiqueta NUNCA para → CPU bloqueada
2. **FC2 rota/desconectada:** Motor contraetiqueta NUNCA para → CPU bloqueada
3. **Sin visibilidad:** Usuario no sabe qué pasó
4. **Sin recuperación:** Solo reset físico soluciona

### Impacto Operativo
- Máquina detiene completamente
- Producto atascado
- Requiere reinicio hardware
- Pérdida de producción

---

## 2️⃣ ESPECIFICACIÓN TÉCNICA

### 2.1 Comportamiento Esperado

#### Caso 1: Operación Normal (FC funciona)
```
t=0ms:      Motor etiqueta enciende, inicia timer
t=50-150ms: FC1 detecta cambio de rendija
t=150ms:    Motor etiqueta para (detección correcta)
✅ Resultado: Ciclo normal, sin error
```

#### Caso 2: FC Falla (sensor roto/desconectado)
```
t=0ms:      Motor etiqueta enciende, inicia timer
t=100ms:    FC1 NUNCA cambia (roto)
t=5000ms:   TIMEOUT alcanzado (5 segundos)
→ Motor se apaga AUTOMÁTICAMENTE
→ Alerta en LCD: "FC Etiquetas" o "FC Contras"
→ Usuario ve error, pueden investigar
✅ Resultado: Máquina se auto-protege
```

### 2.2 Cambios en Código

#### Variables Nuevas (Global)
```cpp
// Timeouts para motores (5 segundos máximo)
const unsigned long TIMEOUT_MOTOR_MS = 5000;  // 5000ms = 5s

// Estados de error para los sensores
bool error_fc1_timeout = false;  // FC Etiquetas no respondió
bool error_fc2_timeout = false;  // FC Contras no respondió
```

#### Lógica en Motor Etiqueta (líneas 275-297)
```cpp
// ANTES: while sin timeout
while (mover1 && actuador_fuera && llegada_botella &&
       millis() > (llegada_botella + delay_botella_etiqueta)) {
  // Motor corre indefinidamente

// AHORA: while CON timeout
while (mover1 && actuador_fuera && llegada_botella &&
       millis() > (llegada_botella + delay_botella_etiqueta)) {

  digitalWrite(PIN_MOTOR_ETI, HIGH);
  if (inicio_motor_etiqueta == 0) inicio_motor_etiqueta = millis();

  // NUEVO: Verificar timeout
  if ((millis() - inicio_motor_etiqueta) > TIMEOUT_MOTOR_MS) {
    // 5 segundos sin respuesta = FC probablemente roto
    digitalWrite(PIN_MOTOR_ETI, LOW);  // APAGAR MOTOR
    error_fc1_timeout = true;          // REGISTRAR ERROR
    mover1 = false;                    // SALIR DEL LOOP
    break;                             // SALIR YA
  }

  // Resto de lógica igual...
}
```

#### Lógica en Motor Contraetiqueta (líneas 300-322)
```cpp
// Mismo patrón que motor etiqueta, pero con FC2 y error_fc2_timeout
```

### 2.3 Interfaz de Usuario - LCD

#### Modo Normal (Sin Error)
```
Línea 0: Botellas:   XXXX
Línea 1: FC1: HIGH   FC2: HIGH
Línea 2: TAct:          XXX
Línea 3: TTotal:   XX.XX
```

#### Modo Error (FC Etiquetas Falla)
```
Línea 0: Botellas:   XXXX
Línea 1: FC1: FAIL   FC2: HIGH  ← FC1 muestra "FAIL"
Línea 2: ERROR:FC Etiquetas     ← Nueva línea de error
Línea 3: Presiona BTN AJUSTES   ← Instrucción reset
```

#### Modo Error (FC Contras Falla)
```
Línea 0: Botellas:   XXXX
Línea 1: FC1: HIGH   FC2: FAIL  ← FC2 muestra "FAIL"
Línea 2: ERROR:FC Contras       ← Nueva línea de error
Línea 3: Presiona BTN AJUSTES   ← Instrucción reset
```

### 2.4 Reset de Error

**Cómo limpiar el error:**
1. Reparar/reconectar el sensor FC
2. Presionar botón AJUSTES
3. Sistema intenta nuevamente

---

## 3️⃣ CAMBIOS EN CÓDIGO

### Variables Globales (Agregar después línea 69)
```cpp
// Timeouts motor (5 segundos máximo antes de parada forzada)
const unsigned long TIMEOUT_MOTOR_MS = 5000;

// Estados de error
bool error_fc1_timeout = false;  // FC Etiquetas falló
bool error_fc2_timeout = false;  // FC Contras falló
```

### Actualizar lcd_draw_static_labels() (línea 160)
```cpp
void lcd_draw_static_labels(bool set_mode) {
  lcd.clear();

  // Si hay error, mostrar alerta
  if (error_fc1_timeout || error_fc2_timeout) {
    lcd.setCursor(0,0); lcd.print("Botellas:");
    lcd.setCursor(0,1); lcd.print("FC1: ");
    lcd.setCursor(5,1); lcd.print(error_fc1_timeout ? "FAIL" : "OK  ");
    lcd.setCursor(10,1); lcd.print("FC2: ");
    lcd.setCursor(15,1); lcd.print(error_fc2_timeout ? "FAIL" : "OK  ");

    lcd.setCursor(0,2);
    if (error_fc1_timeout) {
      lcd.print("ERROR:FC Etiquetas");
    } else {
      lcd.print("ERROR:FC Contras");
    }

    lcd.setCursor(0,3); lcd.print("Presiona BTN AJUSTE");
    return;
  }

  // Resto del código normal...
}
```

### Motor Etiqueta - Agregar Timeout (línea 275)
```cpp
// 4) Motor etiquetas (entre=HIGH)
while (mover1 && actuador_fuera && llegada_botella &&
       millis() > (llegada_botella + delay_botella_etiqueta)) {

  digitalWrite(PIN_MOTOR_ETI, HIGH);
  if (inicio_motor_etiqueta == 0) inicio_motor_etiqueta = millis();

  // ⭐ NUEVO: Verificar timeout (5 segundos)
  if ((millis() - inicio_motor_etiqueta) > TIMEOUT_MOTOR_MS) {
    digitalWrite(PIN_MOTOR_ETI, LOW);
    error_fc1_timeout = true;
    mover1 = false;
    need_full_redraw = true;
    break;
  }

  // Lectura estable de FC1
  bool fc1_entre = isStableHigh(PIN_FC1);

  if ((millis() > inicio_motor_etiqueta + 200) && !etiquetapuesta) {
    bool fc1_no_entre = isStableLow(PIN_FC1);
    if ((FCentreetiquetas && fc1_no_entre) || (!FCentreetiquetas && fc1_entre)) {
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
```

### Motor Contraetiqueta - Agregar Timeout (línea 300)
```cpp
// 5) Motor contras (entre=HIGH)
while (mover2 && etiquetapuesta && llegada_botella &&
       millis() > (etiqueta_colocada + delay_etiqueta_contra)) {

  digitalWrite(PIN_MOTOR_CON, HIGH);
  if (inicio_motor_contra == 0) inicio_motor_contra = millis();

  // ⭐ NUEVO: Verificar timeout (5 segundos)
  if ((millis() - inicio_motor_contra) > TIMEOUT_MOTOR_MS) {
    digitalWrite(PIN_MOTOR_CON, LOW);
    error_fc2_timeout = true;
    mover2 = false;
    need_full_redraw = true;
    break;
  }

  // Lectura estable de FC2
  bool fc2_entre = isStableHigh(PIN_FC2);

  if ((millis() > inicio_motor_contra + 200) && !contrapuesta) {
    bool fc2_no_entre = isStableLow(PIN_FC2);
    if ((FCentrecontras && fc2_no_entre) || (!FCentrecontras && fc2_entre)) {
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
```

### Reset de Error (en gestionar_ajustes, línea 105)
```cpp
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
        // ⭐ NUEVO: Si hay error, limpiar al presionar botón
        if (error_fc1_timeout || error_fc2_timeout) {
          error_fc1_timeout = false;
          error_fc2_timeout = false;
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
    delay_contra_preview   = map(pot1_preview, 0, 4095, 0, 1000);
    delay_actuador_preview = map(pot2_preview, 0, 4095, 0, 1000);
  }
}
```

---

## 4️⃣ PRUEBAS Y VALIDACIÓN

### Test 1: Operación Normal (Sin Error)
**Objetivo:** Verificar que sistema funciona normal si sensores OK

**Pasos:**
1. Encender sistema
2. Ejecutar 20 ciclos normales
3. Verificar que NO aparece error en LCD
4. Criterio: ✅ 20/20 ciclos sin error

### Test 2: Simular Falla FC1
**Objetivo:** Verificar que sistema detecta y para motor etiqueta

**Pasos:**
1. Encender sistema
2. Desconectar físicamente cable FC1 (o simular con cinta aislante)
3. Iniciar ciclo (IR detecta botella)
4. Observar:
   - ✅ Motor etiqueta enciende
   - ✅ Después de 5 segundos, motor para
   - ✅ LCD muestra "ERROR:FC Etiquetas"
   - ✅ FC1 aparece como "FAIL"

### Test 3: Simular Falla FC2
**Objetivo:** Verificar que sistema detecta y para motor contraetiqueta

**Pasos:**
1. Desconectar cable FC2
2. Iniciar ciclo
3. Observar:
   - ✅ Motor contraetiqueta enciende (si motor etiqueta se detiene correctamente)
   - ✅ Después de 5 segundos, motor para
   - ✅ LCD muestra "ERROR:FC Contras"
   - ✅ FC2 aparece como "FAIL"

### Test 4: Reset de Error
**Objetivo:** Verificar que presionar BTN AJUSTES limpia el error

**Pasos:**
1. Con error activo (FC desconectado)
2. Presionar BTN AJUSTES (1 segundo)
3. Observar:
   - ✅ Error desaparece del LCD
   - ✅ Si sensor está reconectado, siguiente ciclo funciona normal

### Test 5: Recuperación Post-Error
**Objetivo:** Verificar que máquina se recupera después de reparación

**Pasos:**
1. Causar error (desconectar FC)
2. Reconectar sensor
3. Presionar BTN AJUSTES
4. Ejecutar 10 ciclos
5. Criterio: ✅ 10/10 ciclos sin errores

---

## 5️⃣ RIESGOS Y MITIGACIÓN

| Riesgo | Probabilidad | Impacto | Mitigación |
|--------|------------|--------|-----------|
| 5s es demasiado corto | Baja | Medio | Ajustable vía TIMEOUT_MOTOR_MS |
| 5s es demasiado largo | Baja | Bajo | Cambiar a 3-4s si es necesario |
| Error persiste después reset | Baja | Bajo | Requiere desconectar/reconectar |
| Motor queda a mitad ciclo | Muy Baja | Alto | Pero mejor que loop infinito |

---

## 6️⃣ PLAN DE IMPLEMENTACIÓN

### Fase 1: Código (20 minutos)
- [ ] Agregar variables TIMEOUT_MOTOR_MS, error_fc1_timeout, error_fc2_timeout
- [ ] Implementar timeout en while motor etiqueta
- [ ] Implementar timeout en while motor contraetiqueta
- [ ] Implementar reset en gestionar_ajustes()
- [ ] Actualizar lcd_draw_static_labels() para mostrar errores

### Fase 2: Compilación (5 minutos)
- [ ] Compilar código
- [ ] Verificar sin errores de compilación
- [ ] Verificar sin warnings

### Fase 3: Test Inicial (15 minutos)
- [ ] Test 1: Operación normal (20 ciclos)
- [ ] Test 2: Simular falla FC1
- [ ] Test 3: Simular falla FC2
- [ ] Test 4: Reset de error
- [ ] Test 5: Recuperación post-error

### Fase 4: Documentación (10 minutos)
- [ ] Actualizar SPECS_GENERAL.md
- [ ] Documentar comportamiento nuevo
- [ ] Crear git commit [SPEC_CHG-001]

---

## 7️⃣ CRITERIOS DE ACEPTACIÓN

✅ **ACEPTADO si:**
- Test 1: 20/20 ciclos sin error (operación normal)
- Test 2: Motor etiqueta para después 5s, muestra "FC Etiquetas"
- Test 3: Motor contraetiqueta para después 5s, muestra "FC Contras"
- Test 4: BTN AJUSTES limpia error
- Test 5: 10/10 ciclos sin error después de recuperación
- Código compila sin errores/warnings

❌ **RECHAZADO si:**
- Motor no para después 5s
- Error no aparece en LCD
- Reset no funciona
- Código no compila

---

## 📎 REFERENCIAS

- main.cpp líneas 275-297 (Motor etiqueta)
- main.cpp líneas 300-322 (Motor contraetiqueta)
- main.cpp líneas 105-134 (gestionar_ajustes)
- DIAGNOSTICO_VARIADORES.md (contexto de problemas)

---

**Próximo paso:** Implementar código y compilar
