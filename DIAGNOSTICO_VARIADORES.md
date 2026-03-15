# 🔴 DIAGNÓSTICO: Variación 2-5mm en Espaciado Etiqueta-Contraetiqueta

**Fecha:** 2026-03-15
**Estado:** CRÍTICO - Causa raíz identificada

---

## 📊 Parámetros Actuales Medidos

### Motor Etiqueta
```
P09 (1st Accel):    0.001s  ⚠️ INSTANTÁNEO
P10 (1st Decel):    0.001s  ⚠️ INSTANTÁNEO
P11 (2nd Accel):    0.0001s 🔴 CRÍTICO - MÁS RÁPIDO QUE P09
P12 (2nd Decel):    0.001s  ⚠️ INSTANTÁNEO
P54 (Slip Comp):    0.00    🔴 SIN COMPENSACIÓN
P92 (Control Mode): 0       ✓ V/F (correcto)
```

### Motor Contraetiqueta
```
P09 (1st Accel):    0.001s  ⚠️ INSTANTÁNEO
P10 (1st Decel):    0.001s  ⚠️ INSTANTÁNEO
P11 (2nd Accel):    10.00s  ⚠️ DIFERENTE del etiqueta
P12 (2nd Decel):    10.0s   ⚠️ DIFERENTE del etiqueta
P54 (Slip Comp):    0.00    🔴 SIN COMPENSACIÓN
P92 (Control Mode): 0       ✓ V/F (correcto)
```

---

## 🎯 CAUSA RAÍZ IDENTIFICADA

### ❌ Problema 1: CONFIGURACIÓN ASIMÉTRICA
**Etiqueta vs Contraetiqueta tienen tiempos DIFERENTES:**
- Etiqueta: 0.001s (instantáneo)
- Contraetiqueta: 10.0s (lento en segunda etapa)
- ➡️ **Resultado:** Aceleran/desaceleran en momentos diferentes
- ➡️ **Impacto:** Velocidades relativas varían → Espaciado variable

### ❌ Problema 2: ACELERACIÓN INSTANTÁNEA (0.001s)
Motor debe alcanzar velocidad en **1 milisegundo**:
- Transición abrupta: velocidad 0 → velocidad nominal
- Motor no tiene tiempo para estabilizarse mecánicamente
- Microvibraciones durante aceleración desplazan botella
- FC detecta rendija en punto ligeramente diferente cada vez

### ❌ Problema 3: SIN SLIP COMPENSATION (P54 = 0)
Motor NO compensa cambios de carga:
- Si hay fricciones variables (desgaste, suciedad), velocidad real fluctúa
- Variador solo tiene **feedforward** (V/F proporcional)
- Sin **feedback** de velocidad real
- Ejemplo: Botella más pesada → motores frenan ligeramente → espaciado aumenta 2-3mm

### ❌ Problema 4: P11 Y P12 DESPROPORCIONADOS
En Motor Contraetiqueta:
- P09/P10 = 0.001s (aceleración rápida)
- P11/P12 = 10.0s (desaceleración lenta)
- ➡️ Suggiere que **en algún momento usa segunda rampa**
- ¿Cuándo? Depende de carga/frecuencia → VARIABLE

---

## 📈 GRÁFICO DEL PROBLEMA

```
ETIQUETA (actual):           CONTRAETIQUETA (actual):
Velocidad |                  Velocidad |
    100% |     _____             100% |     ___
         |    /                       |    /         \
       0%|___/                         |___/           \____
         t                                t
         0.001s                         0.001s  10.0s

Resultado: ➡️ Etiqueta llega a velocidad ANTES
           ➡️ Motores desincronizados
           ➡️ Espaciado variable 2-5mm
```

---

## ✅ SOLUCIÓN RECOMENDADA

### Paso 1: SINCRONIZAR AMBOS VARIADORES
**Motor Etiqueta → igual que Contraetiqueta (o viceversa)**

Opción A (recomendada - más estable):
```
P09: 0.500s (500ms) ← Aceleración controlada
P10: 0.500s (500ms) ← Desaceleración controlada
P11: 0.500s (500ms) ← Coherencia
P12: 0.500s (500ms) ← Coherencia
```

Opción B (si necesitas más velocidad):
```
P09: 0.200s (200ms)
P10: 0.200s (200ms)
P11: 0.200s (200ms)
P12: 0.200s (200ms)
```

### Paso 2: AGREGAR SLIP COMPENSATION
Cambiar P54 de 0.00 a ~500-800:
```
P54: 500 (ó 800)  ← Motor mantiene velocidad bajo carga
```

**Por qué:** Cuando hay carga variable (botella pesada, fricción):
- Sin compensación: velocidad baja 2-3%
- Con compensación: velocidad se mantiene constante
- ➡️ Espaciado reproducible

### Paso 3: VALIDAR EN HARDWARE
1. Cambiar parámetros
2. Probar con 50+ botellas
3. Medir variación (debería caer a ±0.5mm)

---

## 🔬 TEST DIAGNÓSTICO ALTERNATIVO (opcional)

Si cambiar P54 no mejora, podría ser:
- **Problema mecánico**: Rodillos desgastados (vibración → diferentes puntos FC)
- **Problema sensor**: FC sucia o con contacto intermitente
- **Problema variador**: Inestabilidad en V/F control (degradación)

---

## 📋 RESUMEN EJECUTIVO

| Aspecto | Estado | Impacto |
|---------|--------|---------|
| **Sincronización motores** | ❌ Asimétrica | ALTO - Principal causa |
| **Slip compensation** | ❌ Desactivado | MEDIO - Variabilidad con carga |
| **Aceleración** | ⚠️ Instantánea | MEDIO - Estabilización deficiente |
| **Modo control** | ✅ V/F OK | BAJO - Correcto para app |

**Acción Inmediata:** Cambiar P09-P12 a 0.5s en AMBOS variadores y P54 a 500+

**Tiempo estimado cambio:** 10 minutos
**Mejora esperada:** Variación reducida a ±0.5-1mm
