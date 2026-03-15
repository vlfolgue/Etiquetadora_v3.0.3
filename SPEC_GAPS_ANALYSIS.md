# ANÁLISIS DE GAPS - Especificaciones vs Código Actual

## RESUMEN EJECUTIVO
El código actual funciona correctamente para el ciclo normal, pero **carece de especificaciones claras** en áreas críticas de confiabilidad, mantenimiento y debugging.

**Riesgo General:** MEDIO
- ✓ Funcionalidad básica: Documentada
- ✗ Manejo de errores: No documentado
- ✗ Debugging: No documentado
- ✗ Recuperación de fallos: No implementada
- ✗ Arquitectura detallada: Implícita

---

## GAP #1: FALTA TIMEOUT EN MOTORES ⚠️ CRÍTICO

### Problema
```cpp
// Línea 275-297: Motor etiqueta
while (mover1 && actuador_fuera && llegada_botella &&
       millis() > (llegada_botella + delay_botella_etiqueta)) {
  digitalWrite(PIN_MOTOR_ETI, HIGH);
  // ... espera transición FC1
  if (fc1_entre && etiquetapuesta) {
    mover1 = false;  // ← Solo aquí sale el loop
    digitalWrite(PIN_MOTOR_ETI, LOW);
  }
}
```

**¿Qué pasa si FC1 nunca cambia?**
- Motor etiqueta se queda encendido indefinidamente
- Loop bloqueado, CPU al 100%
- LCD no se actualiza
- Sistema no responde

### Especificación Faltante
```
REQUISITO: Timeout Motor Etiqueta
- Máximo tiempo de operación: 10 segundos
- Si se alcanza: Apagar motor, registrar error, intentar reset
- Mínimo tiempo: 1 segundo (sensor muy rápido)
```

### Impacto
| Aspecto | Impacto |
|--------|---------|
| Seguridad | ALTO - motor encendido de forma no controlada |
| Confiabilidad | ALTO - bloqueo del sistema |
| Producción | ALTO - máquina se para |

### Solución Especificada
```cpp
const unsigned long MOTOR_ETI_TIMEOUT_MS = 10000;  // 10s máximo
const unsigned long MOTOR_ETI_MIN_MS = 1000;       // 1s mínimo
```

---

## GAP #2: FALTA ESPECIFICACIÓN DE ERRORES ⚠️ CRÍTICO

### Problema
No hay definición de qué es un error, cuándo ocurre, cómo detectarlo, qué hacer.

**Escenarios sin cobertura:**
1. FC1 nunca transiciona → motor bloqueado
2. FC2 nunca transiciona → motor bloqueado
3. IR se queda en LOW → detección permanente
4. Botella no sale a tiempo → actuador atascado
5. Potenciómetro defectuoso → cambios erráticos

### Especificación Faltante
```
CATÁLOGO DE ERRORES

ERROR-001: Motor Etiqueta Timeout
- Condición: Motor activo > 10s sin completar
- Detección: millis() - inicio_motor_etiqueta > 10000
- Acción: Apagar motor, registrar evento
- Recuperación: Reset ciclo

ERROR-002: Motor Contra Timeout
- Condición: Motor activo > 10s sin completar
- Detección: millis() - inicio_motor_contra > 10000
- Acción: Apagar motor, registrar evento
- Recuperación: Reset ciclo

ERROR-003: Botella Atascada
- Condición: Ciclo > 20s sin completar
- Detección: millis() - llegada_botella > 20000
- Acción: Apagar todos, activar alarma
- Recuperación: Manual (reset botón)

ERROR-004: FC Defectuoso
- Condición: FC no cambia estado durante motor activo
- Detección: Esperar 500ms sin transición
- Acción: Registrar sensor fallido
- Recuperación: Continuar o stop

ERROR-005: IR Pegado
- Condición: IR en LOW > 5s (sin movimiento botella)
- Detección: millis() - llegada_botella > 5000 && ir_low_stable
- Acción: Advertencia en LCD
- Recuperación: Manual, revisar sensor
```

### Especificación de Respuesta a Errores
```
Nivel 1 (Aviso):
- Log en LCD: "ERR:MOTOR_ETIQUETA"
- Continuar intentando

Nivel 2 (Parada de ciclo):
- Apagar todos los actuadores
- Congelar máquina
- Mostrar en LCD

Nivel 3 (Reset de sistema):
- Requiere intervención manual
- Botón físico de reset
```

### Impacto
| Aspecto | Impacto |
|--------|---------|
| Mantenimiento | ALTO - sin visibilidad de fallos |
| Debugging | ALTO - no se sabe qué falló |
| Confiabilidad | ALTO - comportamiento impredecible |

---

## GAP #3: FALTA ARQUITECTURA DE ESTADO EXPLÍCITA ⚠️ MEDIA

### Problema
Estado está repartido en 10+ variables booleanas, difícil de seguir.

```cpp
bool detectada_botella = false;
bool botella_detectada_previa = false;
bool actuador_fuera = false;
bool mover1 = false, mover2 = false;
bool etiquetapuesta = false, contrapuesta = false;
bool FCentreetiquetas = false, FCentrecontras = false;
bool ajustes_activos = false;
```

### Especificación Faltante
```
MÁQUINA DE ESTADOS REQUERIDA:

Estados Principales:
- IDLE: Esperando botella
- DETECTING: Botella detectada, confirmación
- MOVING_ACTUADOR: Bajando actuador
- MOTOR_ETIQUETA_ON: Etiquetador moviendo
- MOTOR_ETIQUETA_WAIT: Esperando FC1
- MOTOR_CONTRA_ON: Contrador moviendo
- MOTOR_CONTRA_WAIT: Esperando FC2
- PARADA_ACTUADOR: Esperando tiempo de parada
- CICLO_COMPLETO: Listo para siguiente botella
- ERROR: Estado de error

Transiciones:
IDLE --(IR LOW)--> DETECTING
DETECTING --(confirmación)--> MOVING_ACTUADOR
MOVING_ACTUADOR --(tiempo)--> MOTOR_ETIQUETA_ON
...
```

### Ventaja
- Código más legible
- Evita estados inconsistentes
- Debugging más fácil

---

## GAP #4: FALTA ESPECIFICACIÓN DE COMPORTAMIENTO DE BOTONES ⚠️ MEDIA

### Problema
Botón contras se lee de forma simple, sin especificar comportamiento detallado.

```cpp
// Línea 265
if (digitalRead(PIN_BTN_CONTRAS) == LOW) { mover2 = false; contrapuesta = true; }
```

**Preguntas sin respuesta:**
1. ¿Se puede presionar durante ciclo?
2. ¿Presionar dos veces = vuelve a activar contras?
3. ¿Qué pasa si se presiona durante ajustes?
4. ¿Se necesita debounce?

### Especificación Faltante
```
BOTÓN CONTRAS:
- Comportamiento: Saltea etiqueta de contras en ciclo actual
- Timing: Válido solo entre fin etiqueta y inicio contras
- Debounce: 30ms (como btn_ajustes)
- Múltiples pulsaciones: Ignoradas, solo primera cuenta
- Modo SET: Sin efecto
- Feedback: Cambio en LCD indicando "CONTRAS: OFF"
```

### Impacto
| Aspecto | Impacto |
|--------|---------|
| Usabilidad | MEDIO - comportamiento ambiguo |
| Testing | MEDIO - casos de prueba indefinidos |

---

## GAP #5: FALTA ESPECIFICACIÓN DE PANTALLA LCD ⚠️ BAJA

### Problema
Layout LCD es un "magic number" de columnas.

```cpp
// Línea 167-168
lcd.setCursor(0,2); lcd.print("[SET] TAct:");
// Valor en columna 12?
```

### Especificación Faltante
```
LAYOUT LCD DETALLADO:

MODO IDLE (4 filas, 20 columnas):

Fila 0: "Botellas:      0000"
         0123456789012345678

Fila 1: "FC1:  HIGH  FC2:  HIGH"
         0123456789012345678

Fila 2: "TAct:  000  TCtE:  000"
         0123456789012345678

Fila 3: "TTotal: 00.00"
         0123456789012345678

MODO SET:
Fila 2: "[SET] TAct:  000"
Fila 3: "[SET] TCtE:  000"

- Columna crítica TAct preview: 12
- Columna crítica TCtE preview: 12
```

### Impacto
| Aspecto | Impacto |
|--------|---------|
| Mantenimiento | BAJO - es cosmético |
| Debugging | BAJO - pero dificulta cambios |

---

## GAP #6: FALTA LOGGING/DEBUGGING ⚠️ MEDIA

### Problema
No hay forma de saber qué está pasando internamente.

**Información no capturada:**
- Cuándo se detectan botellas
- Transiciones de estado
- Errores y timeout
- Cambios de configuración
- Fallos de sensor

### Especificación Faltante
```
LOGGING REQUERIDO:

Formato: [TIMESTAMP] [NIVEL] EVENTO
Niveles: DEBUG, INFO, WARN, ERROR

Ejemplos:
[001234] DEBUG Botella detectada
[001456] DEBUG Iniciando motor_etiqueta
[010234] ERROR Motor etiqueta timeout → reset
[010235] INFO Ciclo completado en 10.23s
[020100] INFO Cambio config: TAct=200ms

Destino: Serial @ 115200 baud
Rotación: No (memoria limitada)
```

---

## GAP #7: FALTA ESPECIFICACIÓN DE PERSISTENCIA ⚠️ BAJA

### Problema
Cambios en potenciómetros se pierden al reiniciar.

```cpp
// Línea 120-121: Solo en RAM
delay_etiqueta_contra  = delay_contra_preview;
delay_botella_actuador = delay_actuador_preview;
```

### Especificación Faltante
```
PERSISTENCIA (FUTURO):
- Guardar en EEPROM: Últimos 10 valores de TAct, TCtE
- Cargar al startup
- Versión de config para migración
```

---

## GAP #8: COMPORTAMIENTO DE VARIABLES REDUNDANTES ⚠️ BAJA

### Problema
`botella_detectada_previa` y `detectada_botella` hacen casi lo mismo.

```cpp
// Línea 60-61
bool detectada_botella = false;
bool botella_detectada_previa = false;
```

**Especificación faltante:**
```
botella_detectada_previa:
  - Propósito: Latch para rearme, evita detecciones múltiples
  - Ciclo: IR LOW → se activa → espera IR HIGH para desactivar

detectada_botella:
  - Propósito: Inicia ciclo completo
  - Ciclo: IR LOW estable → se activa → se desactiva al final ciclo

Mejor arquitectura: Usar máquina de estados
```

---

## RESUMEN DE GAPS

| Gap | Prioridad | Área | Impacto | Estado |
|-----|-----------|------|--------|--------|
| #1 Timeout motores | 🔴 CRÍTICA | Seguridad | Bloqueo sistema | Pendiente |
| #2 Manejo errores | 🔴 CRÍTICA | Confiabilidad | Comportamiento impredecible | Pendiente |
| #3 Máquina de estados | 🟠 ALTA | Arquitectura | Difícil mantener | Pendiente |
| #4 Spec botones | 🟡 MEDIA | Comportamiento | Ambigüedad | Pendiente |
| #5 Layout LCD | 🟡 MEDIA | Documentación | Fricciones | Pendiente |
| #6 Logging | 🟡 MEDIA | Debugging | Sin visibilidad | Pendiente |
| #7 Persistencia | 🟢 BAJA | Feature | Config se pierde | Future |
| #8 Redundancia vars | 🟢 BAJA | Limpieza | Confusión | Refactor |

---

## PRÓXIMOS PASOS

**Inmediatos (Semana 1):**
1. ✅ Crear SPECS_GENERAL.md ← DONE
2. ⏳ Crear SPEC_CHG-001: "Agregar timeouts en motores"
3. ⏳ Crear SPEC_CHG-002: "Sistema de errores y logging"

**Corto plazo (Mes 1):**
4. Implementar CHG-001 y CHG-002
5. Refactorizar máquina de estados (SPEC_CHG-003)

**Mediano plazo:**
6. Agregar logging Serial
7. Revisar y documentar cada función

---

**Documento de Análisis:** GAPS v1.0
**Fecha:** 2026-03-15
**Revisor:** Por designar
