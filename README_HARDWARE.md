# 🔧 DOCUMENTACIÓN DE HARDWARE - GUÍA RÁPIDA

Bienvenido a la documentación completa de hardware de la Etiquetadora Automática.

---

## 📁 ARCHIVOS DE DOCUMENTACIÓN

### 1. **HARDWARE_SPEC.md** 📋
**Especificación técnica completa del hardware**
- Tabla de pines ESP32 y sus funciones
- Diagrama de conexiones
- Componentes hardware instalados
- Parámetros de operación
- Problemas activos identificados

**Cuándo usarlo**: Cuando necesites saber exactamente cómo está conectado algo o qué componente hace qué.

**Ejemplo**: "¿En qué pin va el sensor IR?" → Busca en HARDWARE_SPEC.md

---

### 2. **HARDWARE_PREGUNTAS.md** ❓
**Almacén de preguntas para verificar físicamente**
- Preguntas organizadas por prioridad (🔴 Crítica, 🟠 Alta, 🟡 Media, 🟢 Baja)
- Detalles específicos que faltan por confirmar
- Template de verificación para cuando abras la placa
- Modelo para registrar mediciones

**Cuándo usarlo**: Antes de abrir la placa, sabe exactamente qué preguntar y qué verificar.

**Ejemplo**: "Necesito saber si hay capacitor en P26" → Pregunta #2 en HARDWARE_PREGUNTAS.md

---

## 🚀 FLUJO DE TRABAJO

### 1️⃣ Hoy (Documentación)
```
✓ Código cargado en ESP32
✓ Especificación de hardware creada
✓ Preguntas almacenadas
→ Código listo para probar
```

### 2️⃣ Cuando abras la placa (Verificación)
```
1. Lee HARDWARE_PREGUNTAS.md (especialmente sección 🔴)
2. Abre el cuadro de controles
3. Llena el template de verificación
4. Me comunicas los valores
5. Actualizo HARDWARE_SPEC.md con información real
6. Resolvemos problemas con datos reales
```

### 3️⃣ Futuro (Referencia)
```
Cualquier duda → Consulta HARDWARE_SPEC.md
Nuevo problema → Registro en HARDWARE_PREGUNTAS.md
Cambios de hardware → Actualizo HARDWARE_SPEC.md
```

---

## 🎯 PROBLEMAS ACTIVOS DOCUMENTADOS

### ❌ P26 Motor Etiquetadora no apaga
**Archivo**: HARDWARE_SPEC.md → Sección "Problemas Activos"
**Archivo de preguntas**: HARDWARE_PREGUNTAS.md → Preguntas #1-7

**Qué se necesita verificar**:
- Voltaje en base del transistor
- Existencia de capacitor
- Estado de soldaduras
- Conexión de GND

---

## 📊 TABLA DE COMPONENTES

| Componente | Estado | Ubicación en docs |
|---|---|---|
| ESP32 | ✓ Documentado | HARDWARE_SPEC.md - Pinout |
| LCD 20x4 | ✓ Documentado | HARDWARE_SPEC.md - Display |
| Sensores (IR, FC1, FC2) | ? Especificaciones incompletas | HARDWARE_PREGUNTAS.md #16-18 |
| Transistor P26 | 🔴 PROBLEMA ACTIVO | HARDWARE_SPEC.md + PREGUNTAS #1-7 |
| Transistor P27 | ? No verificado | HARDWARE_PREGUNTAS.md #11-12 |
| Variador Etiqueta | ? Modelo desconocido | HARDWARE_PREGUNTAS.md #25 |
| Variador Contra | ? Modelo desconocido | HARDWARE_PREGUNTAS.md #26 |
| Actuador Neumático | ? No especificado | HARDWARE_PREGUNTAS.md #13-15 |
| Botones/Potenciómetros | ? Especificaciones incompletas | HARDWARE_PREGUNTAS.md #16-28 |

---

## ✅ VERIFICACIÓN FÍSICA (Checklist)

Cuando tengas acceso a la placa abierta:

```
VERIFICACIÓN DE P26 (CRÍTICO):
□ Medir voltaje Base con GPIO26=LOW
□ Medir voltaje Base con GPIO26=HIGH
□ Fotografiar transistor (frente y soldaduras)
□ Buscar capacitor entre Base-GND
□ Medir resistencia de base (si existe)
□ Verificar diodo de protección

VERIFICACIÓN DE ALIMENTACIÓN:
□ Medir voltaje 5V (debe ser 4.75-5.25V)
□ Medir voltaje 3.3V (debe ser 3.0-3.6V)
□ Rastrear cables de GND

VERIFICACIÓN DE SENSORES:
□ Medir voltaje IR en reposo y con botella
□ Medir voltaje FC1 en HOME y con etiqueta
□ Medir voltaje FC2 en HOME y con contraetiqueta

VERIFICACIÓN DE COMPONENTES:
□ Identificar modelo exacto de variadores
□ Confirmar tipo de actuador neumático
□ Fotografiar toda la instalación
```

---

## 💡 TIPS PARA INVESTIGACIÓN

1. **Toma fotos**: Una foto del circuito vale más que mil palabras
2. **Mide voltajes**: Usa multímetro en cada punto clave
3. **Documenta**: Anota medidas exactas en el template de HARDWARE_PREGUNTAS.md
4. **Marca cables**: Usa cinta de masking para etiquetar conexiones
5. **Sé cuidadoso**: Ten cuidado con voltaje AC (220V en motores)

---

## 🔗 INTEGRACIÓN CON CÓDIGO

El código en `src/main.cpp` asume:
- **Todos los pines** según tabla en HARDWARE_SPEC.md
- **Todos los componentes** sin problemas
- **Si cambias hardware**, actualiza HARDWARE_SPEC.md y el código

Referencia en código:
```cpp
const int PIN_MOTOR_ETI = 26;   // Documentado en HARDWARE_SPEC.md
const int PIN_SDA = 21;         // Documentado en HARDWARE_SPEC.md
// ... etc
```

---

## 📞 CÓMO USARLOS CONMIGO

**Cuando reportes un problema**:
1. Consulta HARDWARE_SPEC.md para contexto
2. Lee HARDWARE_PREGUNTAS.md para ideas
3. Si necesitas verificar hardware:
   - Abre la placa
   - Llena el template de HARDWARE_PREGUNTAS.md
   - Comparte mediciones conmigo
   - Actualizo especificaciones

**Ejemplo de comunicación mejorada**:
```
TÚ: "¿Cuántos voltios debería medir en P26?"
YO: "Según HARDWARE_SPEC.md, Pin 26 es salida 3.3V. ¿Cuál es el
     problema? Mide según template en HARDWARE_PREGUNTAS.md #1 y comparte."
```

---

## 🎓 ESTRUCTURA PROPUESTA PARA FUTURO

Si necesitamos expandir:
- **HARDWARE_DIAGRAMA.pdf** - Esquema visual (cuando verifiques)
- **HARDWARE_FOTOS/** - Carpeta con fotos del circuito
- **HARDWARE_CALIBRACION.md** - Valores de calibración de sensores
- **HARDWARE_MANTENIMIENTO.md** - Guía de mantenimiento preventivo

---

## 📅 HISTORIAL DE ACTUALIZACIONES

| Fecha | Cambio | Realizado por |
|---|---|---|
| 2026-03-20 | Creación inicial de especificación | Claude Code |
| 2026-03-20 | Creación de preguntas de verificación | Claude Code |
| **[TU FECHA]** | **[TUS CAMBIOS]** | **[TÚ]** |

---

## 🚨 PROBLEMA CRÍTICO ACTUAL

**P26 Motor Etiquetadora**
- Estado: Motor no apaga aunque GPIO 26 = LOW
- Causa: Hardware (transistor/circuito), no software
- Solución: Verificación física requerida
- Detalles: Ver HARDWARE_SPEC.md "Problemas Activos"
- Preguntas: Ver HARDWARE_PREGUNTAS.md #1-7

---

**Última actualización**: 2026-03-20
**Próximo paso**: Abrir cuadro y llenar template de verificación

Para cualquier duda, consulta el archivo relevante arriba. Si necesitas más información, comunícate compartiendo:
1. El problema específico
2. Foto/medición del hardware (si aplica)
3. Referencia al archivo de documentación
