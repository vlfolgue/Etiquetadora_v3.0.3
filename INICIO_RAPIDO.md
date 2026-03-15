# ⚡ INICIO RÁPIDO - Sistema de Especificaciones

## Lo que acabo de crear para ti

He construido un **framework completo de especificaciones técnicas** para tu proyecto Etiquetadora ESP32. Esto asegura que todos los cambios sean **documentados, aprobados y trazables**.

---

## 📚 5 Documentos Clave

### 1️⃣ **README_SPECS.md** ← EMPIEZA AQUÍ
Tu índice principal. Leer primero cuando vuelvas al proyecto.
- ¿Qué necesitas? → Te dice qué leer
- Quick start por rol
- Links a todo

### 2️⃣ **SPECS_GENERAL.md**
Especificación técnica completa del sistema actual.
- Pinout exacto de todos los pines
- Ciclo de etiquetado explicado
- Tiempos críticos (delay_botella_actuador, etc.)
- Conocer **qué existe ahora**

### 3️⃣ **SPEC_GAPS_ANALYSIS.md**
Análisis de **qué falta documentar y qué hay que arreglar**.
- 8 gaps identificados (3 son CRÍTICOS)
- Impacto de cada uno
- Qué hacer primero
- **Entender qué está roto**

### 4️⃣ **WORKFLOW_CAMBIOS.md**
Proceso paso a paso para hacer cambios de forma segura.
- 10 pasos detallados
- Criterios de aceptación
- Comandos git
- **Cómo hacerlo bien**

### 5️⃣ **SPEC_CHANGE_TEMPLATE.md**
Template para escribir especificaciones de cambios.
- Copiar y renombrar como `SPEC_CHG-001.md`
- Llenar todas las secciones
- Nunca modificar este archivo
- **Herramienta para escribir specs**

---

## 🚀 PRÓXIMOS PASOS (Para Ti)

### Ahora (15 minutos)
```
1. Lee README_SPECS.md de arriba a abajo
2. Entiende: "Esto es cómo organizaremos cambios"
3. No necesitas memorizar, solo saber que existe
```

### Cuando quieras hacer un cambio (1-2 horas)
```
1. Lee WORKFLOW_CAMBIOS.md completo
2. Copia SPEC_CHANGE_TEMPLATE.md → Cambios/SPEC_CHG-001.md
3. Llena documento (justificación, spec, testing, riesgos)
4. Consigue aprobación
5. Implementa siguiendo exactamente tu spec
6. Testing exhaustivo
7. Commit referenciando SPEC_CHG-001
```

### Cuando encuentres un bug
```
1. Busca en SPEC_GAPS_ANALYSIS.md si ya está documentado
2. Si es nuevo: create SPEC_CHG-NNN para documentarlo
3. Especifica: ¿Cómo ocurre? ¿Qué es lo correcto?
4. Luego implementa
```

---

## 🎯 PROBLEMA → SOLUCIÓN (Ejemplos)

### Problema: "Motor etiqueta se queda encendido"
```
1. Busca: SPEC_GAPS_ANALYSIS.md
2. Encuentra: GAP #1 - "Falta timeout en motores"
3. Crea: Cambios/SPEC_CHG-001.md
4. Especifica: "Máximo 10 segundos, luego apagar"
5. Implementa + testing
6. Commit: "[SPEC_CHG-001] Agregar timeout..."
```

### Problema: "¿Cuál es el delay entre botella y etiqueta?"
```
1. Abre: SPECS_GENERAL.md
2. Sección: "3.2 Tiempos Críticos"
3. Encuentra: delay_botella_etiqueta = 200 ms (fixed)
```

### Problema: "Quiero cambiar el layout LCD"
```
1. Lee: SPEC_GAPS_ANALYSIS.md, GAP #5
2. Lee: SPECS_GENERAL.md, sección "4.4 Interfaz LCD"
3. Crea: Cambios/SPEC_CHG-NNN.md
4. Documenta: "Nuevo layout LCD porque..."
5. Define pruebas: "LCD debe mostrar..."
6. Implementa + testing
```

---

## ✅ BENEFICIOS INMEDIATOS

| Beneficio | Por qué |
|-----------|---------|
| 🔒 **Cambios seguros** | Especificación aprobada antes de código |
| 📖 **Documentación actualizada** | No hay "código sin documentar" |
| 🐛 **Debugging fácil** | "¿Por qué se hizo esto?" → SPEC_CHG-NNN.md |
| 🧪 **Testing exhaustivo** | Tests definidos en spec, no después |
| 📊 **Trazabilidad** | Git commit → SPEC_CHG-NNN → HISTORIAL |
| 🚀 **Próximo developer** | Entiende decisiones y restricciones |

---

## 📋 CHECKLIST - TODO LISTO

- ✅ README_SPECS.md - Tu índice/guía de inicio
- ✅ SPECS_GENERAL.md - Especificación del sistema actual
- ✅ SPEC_GAPS_ANALYSIS.md - Qué falta y qué está roto (8 gaps)
- ✅ WORKFLOW_CAMBIOS.md - Proceso de 10 pasos para cambios
- ✅ SPEC_CHANGE_TEMPLATE.md - Template para SPEC_CHG-NNN.md
- ✅ INICIO_RAPIDO.md - Este archivo

**Carpeta para cambios:** `Cambios/` (crear cuando sea necesario)

---

## 🔥 PRIMER CAMBIO RECOMENDADO

### SPEC_CHG-001: Agregar Timeout en Motores
**Prioridad:** CRÍTICA (previene bloqueo del sistema)

```
Problema: Si FC1 o FC2 fallan, motor se queda encendido
Solución: Máximo 10 segundos, luego apagar y registrar error
Impacto: Sistema más seguro, recuperable de fallos

Pasos para implementar:
1. Crear: Cambios/SPEC_CHG-001.md (basado en template)
2. Documentar: Timeout 10s, qué pasa si se alcanza
3. Definir: Testing (¿cómo pruebo timeout?)
4. Implementar: ~20 líneas de código
5. Testing: Compilar, validar lógica
6. Commit: "[SPEC_CHG-001] Agregar timeout motor etiqueta"
```

Tiempo estimado: 2-3 horas

---

## 📞 PREGUNTAS FRECUENTES

### "¿Debo completar TODA la especificación?"
**Sí.** Template tiene secciones por razón. Algunas son cortas.
- Mínimo: 1-2 páginas para cambio pequeño
- Normal: 2-3 páginas para cambio medio
- Completo: 4+ páginas para refactor mayor

### "¿Quién aprueba?"
Depende complejidad (ver WORKFLOW_CAMBIOS.md):
- Simple: Revisor técnico
- Media: Revisor + Responsable
- Mayor: Todos + Testing en hardware

### "¿Puedo saltarme la especificación para arreglos rápidos?"
**No.** Mínimo: 1 página de spec, incluso para bugs simples.
Evita sorpresas y documenta por qué se hizo.

### "¿Se pierden los documentos si borro carpeta Cambios?"
**No.** Documentos base (SPECS_GENERAL, GAPS, WORKFLOW) son permanentes.
Completados en Git, así que está en historial.

### "¿Qué pasa con versiones antiguas de specs?"
Mantén HISTORIAL_CAMBIOS.md actualizado.
Cada entry: fecha, versión, qué cambió.

---

## 🎓 FILOSOFÍA DETRÁS

Este sistema **NO es burocracia**, es **protección**:

```
Antes (sin specs):
  Hago cambio en código
  → Compila ✓
  → Parece funcionar ✓
  → Commit ✓
  → Semanas después: "¿Por qué se hizo esto?"
  → Nadie sabe ✗
  → Miedo a tocar ✗

Con specs:
  Creo SPEC_CHG-001 completa
  → Apruebo primero ✓
  → Valido contra especificaciones ✓
  → Implemento exactamente la spec ✓
  → Testing cubre todos los casos ✓
  → Commit referencia SPEC_CHG-001 ✓
  → Años después: puedo leer SPEC_CHG-001 ✓
  → Entiendo decisión y restricciones ✓
  → Cambios futuros son seguros ✓
```

---

## 🗺️ MAPEO RÁPIDO

```
¿QUÉ NECESITO?                      → DÓNDE ESTÁ

Entender el sistema                 → SPECS_GENERAL.md
Saber qué falta arreglar           → SPEC_GAPS_ANALYSIS.md
Hacer un cambio seguro             → WORKFLOW_CAMBIOS.md
Escribir una especificación        → SPEC_CHANGE_TEMPLATE.md
Indice general                     → README_SPECS.md
Empezar rápido                     → INICIO_RAPIDO.md (aquí)

Mi primer cambio (timeout)         → Crear Cambios/SPEC_CHG-001.md
Historial de cambios               → HISTORIAL_CAMBIOS.md (crear pronto)
```

---

## ✨ RESUMEN EN UNA FRASE

**"Antes de cambiar código, escribe una especificación. Así el siguiente developer (o tú en 6 meses) sabe por qué se hizo."**

---

## 📚 LECTURA RECOMENDADA

1. **HOY:** Este archivo (INICIO_RAPIDO.md) - 5 min
2. **HOY:** README_SPECS.md - 10 min
3. **Cuando hagas cambio:** WORKFLOW_CAMBIOS.md - 20 min
4. **Cuando empieces:** SPECS_GENERAL.md (referencia) - 30 min
5. **Si encuentras problema:** SPEC_GAPS_ANALYSIS.md - variable

**Total:** 1 hora para entender todo (el primero)
**Siguiente:** 30 min por cambio (escribir + implementar)

---

## 🚦 TRÁFICO DE LOS DOCUMENTOS

```
README_SPECS.md ← Todos empiezan aquí
  ↓
  ├─ Quiero entender sistema
  │   → SPECS_GENERAL.md
  │
  ├─ Quiero saber qué falta
  │   → SPEC_GAPS_ANALYSIS.md
  │
  ├─ Quiero hacer cambio
  │   → WORKFLOW_CAMBIOS.md
  │       → SPEC_CHANGE_TEMPLATE.md
  │           → Cambios/SPEC_CHG-NNN.md (copia)
  │
  └─ Quiero precedentes
      → Cambios/ (carpeta)
          → HISTORIAL_CAMBIOS.md (cuando exista)
```

---

## 🎬 AHORA

1. ✅ Acabas de leer INICIO_RAPIDO.md
2. ⏭️ Lee README_SPECS.md (10 minutos)
3. 📖 Lee SPECS_GENERAL.md (referencia, no memorizar)
4. 🚀 Próximo paso: tu primer SPEC_CHG-001

---

**¿Preguntas?** Busca en README_SPECS.md "¿QUÉ NECESITAS?"

**¿Listo para cambiar código?** Lee WORKFLOW_CAMBIOS.md

**¿Confundido?** Vuelve aquí y sigue el flujo.

---

*v1.0 - 2026-03-15*
*Framework de Especificaciones para Etiquetadora ESP32*
