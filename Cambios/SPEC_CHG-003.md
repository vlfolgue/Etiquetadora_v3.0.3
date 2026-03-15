# SPEC_CHG-003: Optimización Parámetros Variadores - Eliminar Variación Espaciado

**Versión:** 1.0
**Fecha:** 2026-03-15
**Prioridad:** ALTA
**Estado:** Propuesto

---

## 1️⃣ JUSTIFICACIÓN

### Problema Actual
Sistema produce variación inconsistente de **2-5mm** en espaciado etiqueta-contraetiqueta entre botellas. Esto causa:
- Botellas con etiquetas mal alineadas
- Rechazo de producto en QC
- Reprocesamiento manual

### Diagnóstico
Análisis de parámetros Mini-VFD SUSWE revela:

| Parámetro | Etiqueta | Contraetiqueta | Problema |
|-----------|----------|---|----------|
| P09-P12 (Accel/Decel) | 0.001s | 10.0s | **Asimétricos** |
| P54 (Slip Comp) | 0 | 0 | **Desactivado** |

**Causa Raíz:**
1. Variadores NO aceleran/desaceleran al mismo tiempo → velocidades relativas varían
2. SIN compensación slip → velocidad real fluctúa con carga variable
3. Resultado: FC detecta rendija en puntos diferentes → espaciado variable

### Root Cause Analysis
```
Variador Etiqueta acelera en 0.001s
Variador Contraetiqueta acelera en 0.001s (P09) pero desacelera en 10.0s (P12)
                    ↓
Motores alcanzan velocidad en momentos diferentes
                    ↓
Velocidades relativas variables durante movimiento
                    ↓
Botella se mueve diferente cada ciclo
                    ↓
FC detecta rendija en posición diferente (±2-5mm)
                    ↓
Espaciado VARIABLE
```

---

## 2️⃣ ESPECIFICACIÓN TÉCNICA

### 2.1 Cambios en Hardware (Mini-VFD SUSWE)

#### Motor Etiqueta - Nuevos Parámetros:
```
P09: 0.001 → 0.500  (1st Acceleration time: 500ms)
P10: 0.001 → 0.500  (1st Deceleration time: 500ms)
P11: 0.0001 → 0.500 (2nd Acceleration time: coherencia)
P12: 0.001 → 0.500  (2nd Deceleration time: coherencia)
P54: 0.00 → 500     (Slip Compensation coefficient: activada)
```

#### Motor Contraetiqueta - Nuevos Parámetros:
```
P09: 0.001 → 0.500  (1st Acceleration time: alineación)
P10: 0.001 → 0.500  (1st Deceleration time: alineación)
P11: 10.00 → 0.500  (2nd Acceleration time: CRÍTICO - reducir 20x)
P12: 10.0 → 0.500   (2nd Deceleration time: CRÍTICO - reducir 20x)
P54: 0.00 → 500     (Slip Compensation coefficient: activada)
```

### 2.2 Justificación de Valores

#### ¿Por qué 0.500s (500ms)?
- **Mínimo para estabilización mecánica:** Motor necesita tiempo para que eje se estabilice
- **No es instantáneo (0.001s):** Evita shock mecánico → microvibraciones
- **No es lento (10.0s):** Sigue siendo rápido para producción
- **Compromiso:** Balance entre estabilidad y velocidad de ciclo

#### ¿Por qué P54 = 500?
- Rango: 0-1000 (0=sin compensación, 1000=máxima)
- 500 = compensación media → mantiene velocidad ±0.5% bajo carga variable
- Evita que botella pesada cause reducción de velocidad
- Asegura spacing reproducible

### 2.3 Impacto en Sistema

**Ciclo de Operación (sin cambios):**
- Fase 1: Detección IR → 0-10ms (sin cambio)
- Fase 2: Bajada actuador → delay_botella_actuador (sin cambio)
- **Fase 3: Aceleración motor etiqueta** → 0.5s (ERA: 0.001s)
- **Fase 4: Parada motor etiqueta** → 0.5s (ERA: 0.001s)
- Fase 5: Espera contraetiqueta → delay_etiqueta_contra (sin cambio)
- **Fase 6: Aceleración motor contraetiqueta** → 0.5s (ERA variable 0.001-10.0s)
- **Fase 7: Parada motor contraetiqueta** → 0.5s (ERA variable 0.001-10.0s)

**Tiempo Total Ciclo:**
- Actual: ~2-3s (depende ciclo)
- Después: ~2-3.5s (200-300ms más por aceleración controlada)
- **Impacto:** -10% velocidad producción, +100% precisión

---

## 3️⃣ PRUEBAS Y VALIDACIÓN

### 3.1 Test Plan

#### Test 1: Sincronización de Aceleración
**Objetivo:** Verificar que ambos motores aceleran simultáneamente

**Pasos:**
1. Aplicar cambios P09-P12 a ambos variadores
2. Iniciar sistema en modo "sin botellas" (solo motores)
3. Registrar tiempo que tarda cada motor en alcanzar velocidad nominal
4. **Criterio de aceptación:** Diferencia < 50ms entre motores

**Cómo medir:**
- Usar taquómetro digital en cada motor
- O usar sensores ópticos en poleas para medir aceleración

#### Test 2: Variación de Espaciado
**Objetivo:** Medir reducción en variación 2-5mm

**Pasos:**
1. Ejecutar 100 botellas consecutivas
2. Medir espaciado etiqueta-contraetiqueta en cada botella (regla/vernier)
3. Calcular: media, desviación estándar, min-max
4. **Criterio de aceptación:**
   - Antes: 2-5mm variación (std dev ~1mm)
   - Después: ±0.5-1mm variación (std dev <0.3mm)

**Datos a recopilar:**
```
Botella #  |  Espaciado (mm)  |  Desviación respecto media
1          |  15.2            |  -0.3
2          |  15.8            |  +0.3
...
100        |  15.5            |  0.0
           |
MEDIA:     |  15.5mm          |
STD DEV:   |  0.25mm          |  ✅ (objetivo < 0.3mm)
MIN:       |  15.0mm          |
MAX:       |  16.0mm          |
RANGO:     |  1.0mm           |  ✅ (antes era 5mm)
```

#### Test 3: Robustez con Carga Variable
**Objetivo:** Verificar que P54 compensación funciona

**Pasos:**
1. Colocar botellas de diferente peso (vacías vs llenas)
2. Ejecutar 50 ciclos mixtos (25 vacías, 25 llenas)
3. Medir espaciado en cada tipo
4. **Criterio de aceptación:**
   - Variación entre vacías y llenas: < 0.5mm
   - (Antes: probablemente 1-2mm diferencia)

#### Test 4: Efecto en Ciclo Productivo
**Objetivo:** Validar que no hay degradación de rendimiento

**Pasos:**
1. Medir velocidad producción ANTES cambios: X botellas/hora
2. Ejecutar 30 minutos con nuevos parámetros
3. Medir velocidad producción DESPUÉS: Y botellas/hora
4. **Criterio de aceptación:** Pérdida < 15% (500ms aceleración cuesta ~200-300ms/ciclo)

### 3.2 Rollback Plan (Si algo falla)

Si test 1 o 2 fallan, **volver a parámetros originales:**
```
Etiqueta: P09=0.001, P10=0.001, P11=0.0001, P12=0.001, P54=0.00
Contraetiqueta: P09=0.001, P10=0.001, P11=10.00, P12=10.0, P54=0.00
```

---

## 4️⃣ RIESGOS Y MITIGACIÓN

| Riesgo | Probabilidad | Impacto | Mitigación |
|--------|------------|--------|-----------|
| Aceleración más lenta causa pérdida rendimiento | Media | Bajo | Medir velocidad antes/después |
| P54 mal calibrado causa instabilidad | Baja | Medio | Empezar con 500, ajustar a 300-800 |
| Cambios no apliquen correctamente | Baja | Alto | Verificar parámetros ANTES y DESPUÉS cambio |
| Problema NO era variadores sino mecánico | Baja | Alto | Si spacing sigue igual, revisar rodillos/FC |

---

## 5️⃣ PLAN DE IMPLEMENTACIÓN

### Fase 1: Preparación (1 hora)
- [ ] Verificar estado actual (foto de spacing)
- [ ] Documentar parámetros originales completos (todo P00-P116)
- [ ] Hacer backup en dispositivo variador (si posible)
- [ ] Preparar hoja de verificación para test

### Fase 2: Cambio (10-15 minutos)
- [ ] Apagar ambos variadores
- [ ] Acceder a menú parámetros motor etiqueta
  - [ ] P09: cambiar a 0.500
  - [ ] P10: cambiar a 0.500
  - [ ] P11: cambiar a 0.500
  - [ ] P12: cambiar a 0.500
  - [ ] P54: cambiar a 500
- [ ] Acceder a menú parámetros motor contraetiqueta
  - [ ] P09: cambiar a 0.500
  - [ ] P10: cambiar a 0.500
  - [ ] P11: cambiar a 0.500 (⚠️ reducir de 10.00)
  - [ ] P12: cambiar a 0.500 (⚠️ reducir de 10.0)
  - [ ] P54: cambiar a 500
- [ ] Encender variadores

### Fase 3: Validación (30-45 minutos)
- [ ] Test 1: Sincronización aceleración (5 min)
- [ ] Test 2: 100 botellas + medición spacing (20 min)
- [ ] Test 3: Botellas mixtas peso variable (10 min)
- [ ] Test 4: Medir velocidad producción (5 min)
- [ ] Revisar criterios aceptación

### Fase 4: Documentación (15 minutos)
- [ ] Actualizar SPECS_GENERAL.md con nuevos parámetros
- [ ] Documentar resultados test en archivo de resultados
- [ ] Actualizar SPEC_GAPS_ANALYSIS.md (marcar este gap como RESUELTO)
- [ ] Crear git commit [SPEC_CHG-003]

---

## 6️⃣ CRITERIOS DE ACEPTACIÓN

✅ **ACEPTADO si:**
- Test 1: Diferencia sincronización < 50ms
- Test 2: Espaciado promedio ±0.5-1mm (std dev < 0.3mm)
- Test 3: Variación vacías vs llenas < 0.5mm
- Test 4: Velocidad producción no cae > 15%

❌ **RECHAZADO si:**
- Algún test no cumple criterios
- Spacing igual o peor después cambios
- Sistema inestable o impredecible

---

## 7️⃣ NOTAS TÉCNICAS

### ¿Por qué P11 en contraetiqueta era 10.0s?
Posible que el sistema original estuviera configurado para **multi-stage speed**:
- P09/P10 (0.001s) para arranque rápido
- P11/P12 (10.0s) para decelerar suave al final
- Pero esto desincroniza con motor etiqueta

### ¿Qué es P54 Slip Compensation?
- Motor asíncrono tiene "slip" = diferencia entre velocidad síncrona y real
- Sin compensación: bajo carga, motor se desacelera automáticamente
- Con compensación: variador aumenta voltaje para mantener velocidad
- Resultado: velocidad más constante bajo carga variable

### ¿Habrá mejor solución?
- **Futuro (SPEC_CHG-004):** Cambiar a modo Vector (P92=1) para control más preciso
- **Futuro (SPEC_CHG-005):** Agregar encoder para retroalimentación de velocidad real
- Pero P54 es la solución rápida y efectiva ahora

---

## 📎 REFERENCIAS

- DIAGNOSTICO_VARIADORES.md - Análisis detallado del problema
- mini-vfd-suswe-750w-1500w.pdf - Manual variador (secciones P09, P10, P54)
- SPECS_GENERAL.md - Especificación sistema actual

---

**Próximo paso:** Obtener aprobación y proceder a Fase 1
