# ESPECIFICACIONES TÉCNICAS - Sistema Etiquetadora ESP32

## 1. DESCRIPCIÓN GENERAL
Sistema de control automático para máquina etiquetadora de botellas.
- **Hardware:** ESP32
- **Lenguaje:** Arduino C++
- **Interfaz:** LCD 20x4 I2C + Botones + Potenciómetros
- **Versión:** v4.0 FC Digitales

---

## 2. ARQUITECTURA Y COMPONENTES

### 2.1 Hardware (Pinout)
| Función | Pin | Tipo | Propósito |
|---------|-----|------|----------|
| IR Botella | 17 | INPUT | Detector IR botellas |
| FC1 (Etiqueta) | 34 | INPUT | Final de carrera motor etiqueta |
| FC2 (Contra) | 35 | INPUT | Final de carrera motor contra |
| Botón Contras | 25 | INPUT | Seleccionar skip contras |
| Botón Ajustes | 19 | INPUT | Entrar modo configuración |
| POT1 | 36 | INPUT ADC | Control delay etiqueta-contra |
| POT2 | 39 | INPUT ADC | Control delay botella-actuador |
| Motor Etiqueta | 26 | OUTPUT | Control motor etiquetas |
| Motor Contra | 27 | OUTPUT | Control motor contras |
| Actuador | 16 | OUTPUT | Control bajada/puesta botella |
| SDA (I2C) | 21 | I2C | Comunicación LCD |
| SCL (I2C) | 22 | I2C | Comunicación LCD |

### 2.2 Periféricos
- **LCD:** LiquidCrystal_I2C (dirección 0x27, 20 cols × 4 filas)
- **Sensores:** 3 digitales (IR, 2× FC), 2 analógicos (potenciómetros)
- **Actuadores:** 3 salidas digitales (motores + actuador)

---

## 3. CICLO DE OPERACIÓN

### 3.1 Estados del Sistema
```
[IDLE]
  ↓ (botella detectada por IR)
[CICLO_ACTIVO]
  ├─ T0: Detección botella
  ├─ T1: Bajada actuador (delay_botella_actuador)
  ├─ T2: Motor etiqueta (espera FC1 transición)
  ├─ T3: Motor contra (espera FC2 transición) [si no está desactivado]
  ├─ T4: Parada actuador (delay_parada_actuador)
  └─ T5: Ciclo completo
```

### 3.2 Tiempos Críticos
| Parámetro | Rango | Default | Unidad | Notas |
|-----------|-------|---------|--------|-------|
| delay_botella_actuador | 0-1000 | 0 | ms | Tiempo entre detección y bajada actuador |
| delay_botella_etiqueta | Fixed | 200 | ms | Offset fijo desde detección |
| delay_etiqueta_contra | 0-1000 | 0 | ms | Tiempo entre fin etiqueta y motor contra |
| tiempo_parada_actuador | Fixed | 300 | ms | Tiempo mínimo antes de subir actuador |
| STABLE_IR_MS | Fixed | 2 | ms | Ventana estabilidad IR |
| STABLE_MS | Fixed | 5 | ms | Ventana estabilidad FC |
| SAMPLE_US | Fixed | 200 | μs | Período muestreo durante ventana |
| DEBOUNCE_MS | Fixed | 30 | ms | Debounce botones |

---

## 4. ESPECIFICACIONES FUNCIONALES

### 4.1 Detección de Botellas (IR)
- **Activación:** Nivel LOW estable durante STABLE_IR_MS
- **Rearme:** Retorno a HIGH estable
- **Antirruido:** isStableLevel() con 5 muestreos @ 200 μs

### 4.2 Control de Motores
- **Motor Etiqueta:**
  - Se activa en flanco de llegada botella + delay_botella_etiqueta
  - Se desactiva cuando FC1 retorna a estado inicial ("entre")
  - Timeout implícito: depende de tiempo físico

- **Motor Contra:**
  - Se activa después motor etiqueta completa + delay_etiqueta_contra
  - Se desactiva cuando FC2 retorna a estado inicial
  - Puede ser desactivado por botón PIN_BTN_CONTRAS

### 4.3 Actuador
- **Activación:** delay_botella_actuador ms después de detección
- **Desactivación:** tiempo_parada_actuador ms después de último evento

### 4.4 Interfaz LCD
- **Modo IDLE:** Muestra botellas, FC1/FC2 estado, tiempos, tiempo total
- **Modo SET:** Muestra previewers de potenciómetros en tiempo real
- **Actualización:** Max 200 ms (intervalo_lcd_idle)
- **Cambios parciales:** Solo redibuja valores que cambiaron

### 4.5 Ajustes (Potenciómetros)
- **Filtrado:** Media móvil 5 muestras
- **Rango ADC:** 0-4095 → 0-1000 ms
- **Aplicación:** Al soltar botón ajustes
- **Preview:** Visible en tiempo real mientras está en SET

---

## 5. RESTRICCIONES Y LIMITACIONES

### 5.1 Limitaciones Conocidas
- ⚠️ **Bucles while() bloqueantes:** Motores ocupan CPU durante operación
  - Afecta: Responsividad botones, actualización LCD durante movimiento

- ⚠️ **Sin timeout en motores:** Si FC falla, motor se queda encendido

- ⚠️ **Sin logging de errores:** Fallos silenciosos (sensor atascado, etc.)

- ⚠️ **Booleanos redundantes:** botella_detectada_previa + detectada_botella

- ⚠️ **LCD layout ajustado:** Columnas críticas para display de 4 dígitos

### 5.2 Supuestos de Hardware
- FC: sensor de contacto o inducción (NO-C digital)
- FC en estado "entre" = HIGH, "fuera" = LOW (o viceversa, configurable)
- Motores: control simple ON/OFF (sin PWM)
- Actuador: control simple ON/OFF
- IR: activo bajo (LOW = botella, HIGH = libre)

---

## 6. REQUISITOS NO FUNCIONALES

### 6.1 Rendimiento
- Loop debe ejecutarse > 1 kHz (actual: ~100 Hz durante ciclo por while bloqueante)
- Latencia detección-movimiento: < 50 ms
- Refresh LCD: 200 ms máximo

### 6.2 Confiabilidad
- Sin rebotes en sensores digitales: ✓ (implementado)
- Sin falsos positivos IR: ✓ (implementado)
- Recuperación de errores: ✗ (no implementado)

### 6.3 Usabilidad
- Botón ajustes: respuesta < 100 ms
- LCD legible en condiciones industriales
- Potenciómetros sin saltos: ✓ (filtrado)

---

## 7. MÉTRICAS Y MONITOREO

### 7.1 Contadores
- `botellas_etiquetadas` (int): Total desde arranque
- `tiempo_etiquetado` (float): Tiempo ciclo última botella (s)

### 7.2 Estados Monitoreados
- FC1/FC2: Mostrar en LCD (HIGH/LOW)
- Tiempos configurados: Mostrar en LCD
- Modo SET: Activo/Inactivo

### 7.3 Falta Logging Centralizado
- ✗ No hay Serial.print() para debugging
- ✗ No hay registro de eventos (botellas, errores, cambios config)
- ✗ No hay detección de fallos

---

## 8. CAMBIOS FUTUROS Y MEJORAS

### Prioridad ALTA (Seguridad)
- [ ] Timeouts en motores (máx 10s)
- [ ] Detección y logging de errores
- [ ] Límite máximo ciclo incompleto (sensor atascado)

### Prioridad MEDIA (Arquitectura)
- [ ] Refactorizar while() → máquina de estados
- [ ] Usar interrupciones para botones
- [ ] Eliminar variables booleanas redundantes

### Prioridad BAJA (Polish)
- [ ] Serial output para debugging
- [ ] Estadísticas (velocidad promedio, fallos)
- [ ] Ajustes persistentes en EEPROM

---

## 9. HISTORIAL DE VERSIONES

| Versión | Fecha | Cambios |
|---------|-------|---------|
| v4.0 | 2025-08-11 | FC Digitales, estructura actual |
| v4.1 | TBD | [Pendiente cambios] |

---

## 10. NOTAS PARA DESARROLLADORES

- Modificar `delay_*` solo desde modo SET o código compilado
- FC1/FC2 deben ser mecánicamente confiables (sin chatter)
- Potenciómetros: recalibrar si rango es < 200 mV
- LCD: verificar dirección I2C (0x27) si no inicializa
- Loop bloqueante en while(): considerar refactor con máquina de estados para futuro

---

**Documento Base:** Especificaciones v1.0
**Próxima Revisión:** Cuando se implementen cambios ALTA prioridad
