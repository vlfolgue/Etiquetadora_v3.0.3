# 📋 GUÍA DE ESPECIFICACIONES - Etiquetadora ESP32

**Este es tu punto de entrada para entender, documentar y cambiar el código de forma estructurada.**

---

## 🎯 ¿QUÉ NECESITAS?

### ✅ Quiero entender el sistema actual
→ Lee: **[SPECS_GENERAL.md](SPECS_GENERAL.md)**
- Descripción de hardware
- Ciclo de operación
- Parámetros del sistema
- Restricciones conocidas

### ⚠️ Quiero saber qué falta documentar
→ Lee: **[SPEC_GAPS_ANALYSIS.md](SPEC_GAPS_ANALYSIS.md)**
- 8 gaps principales identificados
- Impacto de cada gap
- Prioridad de cada uno
- Próximos pasos recomendados

### 🔧 Quiero hacer un cambio en el código
→ Lee: **[WORKFLOW_CAMBIOS.md](WORKFLOW_CAMBIOS.md)**
- Flujo paso a paso (10 pasos)
- Criterios de aceptación
- Plantillas a usar
- Comando de git

### 📝 Voy a proponer un cambio específico
→ Usa: **[SPEC_CHANGE_TEMPLATE.md](SPEC_CHANGE_TEMPLATE.md)**
- Template completo para escribir especificaciones
- Copiar y renombrar como `SPEC_CHG-001.md`, `SPEC_CHG-002.md`, etc.
- Llenar todas las secciones

### 📖 Veo cambios completados anteriormente
→ Mira: **[Cambios/](Cambios/)** (cuando existan)
- SPEC_CHG-001.md (ejemplo: una vez completado)
- SPEC_CHG-002.md
- etc.

### 📚 Historial de todos los cambios
→ Lee: **[HISTORIAL_CAMBIOS.md](HISTORIAL_CAMBIOS.md)** (crear cuando sea necesario)
- Log de cambios implementados
- Dates, versiones, impactos

---

## 📋 DOCUMENTOS DISPONIBLES

### Documentos Base (Sistema)
| Archivo | Propósito | Audiencia | Cambio |
|---------|-----------|-----------|--------|
| [SPECS_GENERAL.md](SPECS_GENERAL.md) | Especificación técnica del sistema | Developers | Raro |
| [SPEC_GAPS_ANALYSIS.md](SPEC_GAPS_ANALYSIS.md) | Análisis de lagunas en especificaciones | Developers, PM | Raro |
| [WORKFLOW_CAMBIOS.md](WORKFLOW_CAMBIOS.md) | Proceso para hacer cambios | Developers, Revisores | Muy raro |
| [SPEC_CHANGE_TEMPLATE.md](SPEC_CHANGE_TEMPLATE.md) | Template para cambios | Developers | NUNCA |
| [README_SPECS.md](README_SPECS.md) | Este archivo (índice) | Todos | Raro |

### Cambios Propuestos/Completados
| Archivo | Estado | Descripción |
|---------|--------|-------------|
| `Cambios/SPEC_CHG-001.md` | [TBD] | Agregar timeouts en motores |
| `Cambios/SPEC_CHG-002.md` | [TBD] | Sistema de errores y logging |
| `Cambios/SPEC_CHG-NNN.md` | [Template] | Copiar para nuevo cambio |

---

## 🚀 QUICK START

### Para Developers
```
1. Lee SPECS_GENERAL.md para entender el sistema
2. Cuando quieras hacer cambio:
   a) Copia SPEC_CHANGE_TEMPLATE.md → Cambios/SPEC_CHG-NNN.md
   b) Llena documento completamente
   c) Busca aprobaciones
   d) Implementa siguiendo WORKFLOW_CAMBIOS.md paso a paso
   e) Haz commit referenciando SPEC_CHG-NNN
```

### Para Revisores
```
1. Lee SPECS_GENERAL.md para contexto
2. Revisa SPEC_CHG-NNN.md del cambio propuesto:
   ☐ Justificación clara
   ☐ Sin conflictos con SPECS_GENERAL
   ☐ Riesgos identificados
   ☐ Testing es completo
   ☐ Implementación es realista
3. Aprueba o sugiere cambios
```

### Para QA/Testing
```
1. Lee SPECS_GENERAL.md para casos normales
2. Revisa SPEC_CHG-NNN.md del cambio:
   - Sección "Pruebas Requeridas"
3. Ejecuta todas las pruebas listadas
4. Documenta resultados en SPEC_CHG-NNN.md
```

---

## 📊 ESTADO ACTUAL DEL PROYECTO

### Especificaciones
- ✅ SPECS_GENERAL.md creada (v1.0)
- ✅ SPEC_GAPS_ANALYSIS.md creada (v1.0)
- ✅ WORKFLOW_CAMBIOS.md creada (v1.0)
- ⏳ SPEC_CHG-001 (Timeouts): Por proponer
- ⏳ SPEC_CHG-002 (Error handling): Por proponer

### Código Actual
- ✅ Funcionalidad básica: Operacional
- ⚠️ Manejo de errores: No implementado
- ⚠️ Logging: No implementado
- ⚠️ Timeouts en motores: No implementado

### Gaps Críticos
| Gap | Prioridad | Next Step |
|-----|-----------|-----------|
| Timeout motores | 🔴 CRÍTICA | Crear SPEC_CHG-001 |
| Manejo errores | 🔴 CRÍTICA | Crear SPEC_CHG-002 |
| Máquina de estados | 🟠 ALTA | Crear SPEC_CHG-003 (futuro) |

---

## 💡 FILOSOFÍA DE ESTE SISTEMA

### Principio 1: Especificación Primero
Nunca hagas cambios sin especificación aprobada. Esto previene:
- Cambios no documentados
- Testing incompleto
- Sorpresas en producción

### Principio 2: Documentación = Código
Especificaciones no son "papeleó", son parte integral del proyecto:
- Futuro developer entiende por qué se hizo
- Debugging es más fácil
- Riesgos están documentados

### Principio 3: Cambios Trazables
Cada cambio tiene su SPEC_CHG-NNN:
- Git commit referencia spec
- Historial es transparente
- Reversión es segura

### Principio 4: Testing Definido Antes
Test cases se definen EN LA ESPECIFICACIÓN:
- Antes de escribir código
- Asegura cobertura
- Facilita QA

---

## 🔍 CÓMO NAVEGAR

### Buscar Información
```
"¿Cuál es el pinout del ESP32?"
→ SPECS_GENERAL.md, sección 2.1

"¿Cómo funciona el ciclo de etiquetado?"
→ SPECS_GENERAL.md, sección 3

"¿Qué cambios se han hecho?"
→ HISTORIAL_CAMBIOS.md (o SPEC_CHG-NNN.md individual)

"¿Qué falta documentar?"
→ SPEC_GAPS_ANALYSIS.md

"¿Cómo hago un cambio seguro?"
→ WORKFLOW_CAMBIOS.md
```

### Buscar por Código
```
grep -r "PIN_MOTOR_ETI" SPECS_GENERAL.md
→ Definición del pin

grep -r "timeout" SPEC_GAPS_ANALYSIS.md
→ Gap #1 es sobre timeouts

grep -r "ERR-001" SPEC_GAPS_ANALYSIS.md
→ Definición del error
```

---

## ✏️ CUANDO CAMBIAR DOCUMENTOS BASE

### NUNCA cambiar sin consenso:
- ❌ SPEC_CHANGE_TEMPLATE.md (es template)

### RARO cambiar (solo si especificación general cambia):
- ⚠️ SPECS_GENERAL.md (cuando agregar nueva feature mayor)
- ⚠️ WORKFLOW_CAMBIOS.md (si proceso cambia)

### NORMAL actualizar después cambio:
- ✅ SPEC_GAPS_ANALYSIS.md (actualizar gaps completados)
- ✅ HISTORIAL_CAMBIOS.md (agregar entry nuevo)

### SIEMPRE con especificación:
- ✅ Cambios en carpeta `Cambios/SPEC_CHG-NNN.md`

---

## 📞 SOPORTE Y PREGUNTAS

### "No sé qué hacer"
→ Leer WORKFLOW_CAMBIOS.md, sección "FLUJO COMPLETO"

### "¿Es esto compatible con el código actual?"
→ Revisar SPECS_GENERAL.md contra tu cambio

### "¿Cuáles son los riesgos?"
→ SPEC_CHG-NNN.md, sección "Riesgos"

### "¿Cómo reviso mis cambios?"
→ WORKFLOW_CAMBIOS.md, sección "CRITERIOS DE ACEPTACIÓN"

### "¿Hay precedentes de esto?"
→ HISTORIAL_CAMBIOS.md o Cambios/SPEC_CHG-*.md

---

## 🎓 EJEMPLOS DE USO

### Caso 1: Revisar especificación de cambio
```
1. Alguien propone: "Agregar logging Serial"
2. Crea: Cambios/SPEC_CHG-003.md
3. Tú haces:
   - Lees SPEC_CHG-003.md completo
   - Verificas contra SPECS_GENERAL.md: ¿hay conflictos?
   - Revisas SPEC_GAPS_ANALYSIS.md: ¿cubre gap #6?
   - Validates testing: ¿qué se va a probar?
   - Apruebas o sugieres cambios
```

### Caso 2: Implementar cambio aprobado
```
1. SPEC_CHG-001 está aprobado: "Timeouts motores"
2. Tú implementas:
   a) Lees SPEC_CHG-001.md completamente
   b) Entiendes pasos en "Plan de Implementación"
   c) Implementas código
   d) Ejecutas pruebas de SPEC_CHG-001.md
   e) Commitas: "[SPEC_CHG-001] Agregar timeout motor etiqueta"
   f) Actualizas: SPEC_GAPS_ANALYSIS.md (gap #1 = done)
   g) Agregas: HISTORIAL_CAMBIOS.md entry nueva
```

### Caso 3: Encontrar información
```
"Motor etiqueta se queda encendido a veces, ¿por qué?"
1. Busca en SPEC_GAPS_ANALYSIS.md: "Motor Etiqueta"
   → Encuentra: Gap #1, CRÍTICO, sin timeout
2. Busca en SPEC_CHG-001.md (si existe)
   → Encuentra: especificación de fix
3. Busca en HISTORIAL_CAMBIOS.md
   → Encuentra: ¿ya fue implementado?
```

---

## 📈 MÉTRICAS DE CALIDAD

### Cobertura de Especificación
- [ ] SPECS_GENERAL.md: ≥ 80% del código documentado
- [ ] Cada función tiene propósito claro
- [ ] Cada variable crítica está documentada

### Cambios Trazables
- [ ] Cada cambio tiene SPEC_CHG-NNN.md
- [ ] Cada commit referencia SPEC_CHG-NNN
- [ ] HISTORIAL_CAMBIOS.md está al día

### Testing
- [ ] Cada SPEC_CHG-NNN tiene sección de testing
- [ ] Testing covers: normal, edges, errors

---

## 🔄 CICLO DE VIDA DE UN CAMBIO

```
IDEA
  ↓
SPEC_CHG-NNN.md (Propuesta)
  ↓
Revisión + Aprobación
  ↓
SPEC_CHG-NNN.md (Aprobado)
  ↓
Implementación
  ↓
Testing
  ↓
SPEC_CHG-NNN.md (Completado)
  ↓
Commit + HISTORIAL_CAMBIOS.md
  ↓
DONE
```

---

## 📝 VERSIÓN DE ESTE DOCUMENTO

- **v1.0** (2026-03-15): Creación inicial
- Próxima actualización: Cuando haya SPEC_CHG-001+ completado

---

## 🔗 ENLACES RÁPIDOS

- [SPECS_GENERAL.md](SPECS_GENERAL.md) - Especificación general
- [SPEC_GAPS_ANALYSIS.md](SPEC_GAPS_ANALYSIS.md) - Análisis de gaps
- [WORKFLOW_CAMBIOS.md](WORKFLOW_CAMBIOS.md) - Cómo hacer cambios
- [SPEC_CHANGE_TEMPLATE.md](SPEC_CHANGE_TEMPLATE.md) - Template para cambios
- [Carpeta de Cambios](Cambios/) - Especificaciones de cambios

---

**¿Primera vez aquí?** Empieza por [SPECS_GENERAL.md](SPECS_GENERAL.md).

**¿Quieres hacer un cambio?** Sigue [WORKFLOW_CAMBIOS.md](WORKFLOW_CAMBIOS.md).

**¿Busca un cambio específico?** Revisa carpeta [Cambios/](Cambios/).

---

*Este sistema de especificaciones asegura que el código sea documentado, predecible y fácil de mantener.*
