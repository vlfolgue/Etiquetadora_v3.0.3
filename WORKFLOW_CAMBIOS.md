# WORKFLOW DE CAMBIOS - Proceso Estructurado

## PRINCIPIO FUNDAMENTAL
**Especificación Primero, Implementación Después**

No se hace NINGÚN cambio en el código sin especificación aprobada.

---

## FLUJO COMPLETO

```
┌─────────────────────────────────────────────────────────┐
│ 1. IDENTIFICAR NECESIDAD DE CAMBIO                      │
│    (Bug, mejora, nueva feature)                         │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│ 2. CREAR ESPECIFICACIÓN DE CAMBIO                       │
│    Usar SPEC_CHANGE_TEMPLATE.md                         │
│    Llenar todas las secciones                           │
│    Guardar como SPEC_CHG-NNN.md                         │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│ 3. REVISAR CONTRA ESPECIFICACIONES EXISTENTES           │
│    - SPECS_GENERAL.md                                  │
│    - SPEC_GAPS_ANALYSIS.md                             │
│    ¿Hay conflictos? ¿Rompe algo?                       │
└────────────────┬────────────────────────────────────────┘
                 │
                 ├─ SÍ HAY CONFLICTOS ─┐
                 │                      ▼
                 │          ┌───────────────────────┐
                 │          │ Revisar especificación│
                 │          │ Ajustar si necesario  │
                 │          └───────────┬───────────┘
                 │                      │
                 │                      └──────────┬────┐
                 │                                 │    │
                 │                                 ▼    │
                 │                                [R]───┘
                 │
                 ├─ NO HAY CONFLICTOS ─► OK
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│ 4. APROBACIÓN                                           │
│    ☐ Desarrollador: Firma especificación               │
│    ☐ Revisor técnico: Valida arquitectura              │
│    ☐ Responsable: Autoriza cambio                      │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│ 5. IMPLEMENTACIÓN                                       │
│    Seguir pasos del plan de implementación              │
│    Validar compilación limpia                          │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│ 6. TESTING                                              │
│    Ejecutar todas las pruebas de SPEC_CHG-NNN.md       │
│    ☐ Unitarias                                         │
│    ☐ Integración                                       │
│    ☐ Campo (si aplica)                                 │
│    ☐ Errores                                           │
└────────────────┬────────────────────────────────────────┘
                 │
                 ├─ ALGO FALLA ─────┐
                 │                  ▼
                 │      ┌─────────────────────┐
                 │      │ Revisar tests       │
                 │      │ Ajustar código/spec │
                 │      └────────┬────────────┘
                 │               │
                 │               └──────────┬────┐
                 │                          │    │
                 │                          ▼    │
                 │                         [R]───┘
                 │
                 ├─ TODO OK
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│ 7. CÓDIGO EN STAGING (git)                              │
│    Cambios listos, no committed                        │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│ 8. DOCUMENTACIÓN DE CAMBIOS                             │
│    ☐ Actualizar SPECS_GENERAL.md si cambios afectan    │
│    ☐ Agregar entrada a historial en SPECS              │
│    ☐ Actualizar SPEC_GAPS_ANALYSIS.md                  │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│ 9. COMMIT Y PUSH                                        │
│    Mensaje: [SPEC_CHG-NNN] Breve descripción           │
│    Incluir: referencia a SPEC_CHG-NNN.md               │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│ 10. VERIFICACIÓN FINAL EN HARDWARE                      │
│     Si es posible, probar en máquina real               │
│     Documentar resultados                               │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
         [CAMBIO COMPLETADO]
```

---

## PLANTILLAS Y DOCUMENTOS

### Documentos Base (No tocar sin consenso)
```
SPECS_GENERAL.md           ← Especificación general del sistema
SPEC_GAPS_ANALYSIS.md      ← Análisis de qué falta documentar
WORKFLOW_CAMBIOS.md        ← Este archivo (proceso)
```

### Para Cada Cambio
```
SPEC_CHG-NNN.md            ← Copia de SPEC_CHANGE_TEMPLATE.md
                              Nombrar con ID único (001, 002, etc.)
```

### Historial
```
HISTORIAL_CAMBIOS.md       ← Registro de todos los cambios completados
```

---

## CRITERIOS DE ACEPTACIÓN POR ETAPA

### Especificación (Antes de cambiar código)
- ☐ Documento completo (todas las secciones)
- ☐ Justificación clara: ¿por qué es necesario?
- ☐ Riesgos identificados y mitigados
- ☐ Pruebas definidas antes de implementar
- ☐ Impacto en especificaciones existentes evaluado
- ☐ Aprobaciones recolectadas

### Implementación
- ☐ Código compila sin errores
- ☐ Código compila sin warnings
- ☐ Cambios solo en áreas especificadas
- ☐ No introduce cambios no especificados
- ☐ No rompe funcionalidad existente

### Testing
- ☐ Todas las pruebas unitarias pasan
- ☐ Todas las pruebas integración pasan
- ☐ Pruebas de error pasan
- ☐ Si es campo: validado en hardware real
- ☐ Documentar resultados en SPEC_CHG-NNN.md

### Documentación
- ☐ SPECS_GENERAL.md actualizado si hay cambios de comportamiento
- ☐ HISTORIAL_CAMBIOS.md con entrada nueva
- ☐ SPEC_GAPS_ANALYSIS.md revisado
- ☐ Commit message referencia SPEC_CHG-NNN.md

---

## TIPOS DE CAMBIOS Y RUTA RÁPIDA

### CAMBIO TIPO A: Bug Simple (< 10 líneas)
**Ejemplo:** Typo en variable, constante incorrecta

```
Especificación: 1 página
Implementación: Directo
Testing: Compilación + lógica básica
Riesgo: Bajo
```

**Pasos reducidos:**
1. ✓ SPEC_CHG-NNN.md (simplificado)
2. ✓ Implementar
3. ✓ Compilar + revisar lógica
4. ✓ Commit

---

### CAMBIO TIPO B: Feature Media (50-200 líneas)
**Ejemplo:** Agregar logging, mejorar LCD

```
Especificación: 3-4 páginas completas
Implementación: Planeado
Testing: Unitario + integración
Riesgo: Medio
```

**Pasos completos:**
1. ✓ SPEC_CHG-NNN.md completo
2. ✓ Revisión de especificación
3. ✓ Aprobación
4. ✓ Implementar
5. ✓ Testing completo
6. ✓ Actualizar SPECS_GENERAL.md
7. ✓ Commit

---

### CAMBIO TIPO C: Refactor Mayor (> 200 líneas)
**Ejemplo:** Máquina de estados, timeouts motores

```
Especificación: 5+ páginas, diagramas
Implementación: Iterativa, con revisiones
Testing: Unitario + integración + campo
Riesgo: Alto
```

**Pasos completos + incrementales:**
1. ✓ SPEC_CHG-NNN.md COMPLETO
2. ✓ Revisión de especificación (formal)
3. ✓ Aprobación (múltiples stakeholders)
4. ✓ Implementar módulo por módulo
5. ✓ Testing después cada módulo
6. ✓ Testing integración final
7. ✓ Testing en hardware
8. ✓ Actualizar SPECS_GENERAL.md completamente
9. ✓ Actualizar SPEC_GAPS_ANALYSIS.md
10. ✓ Crear rama de feature si refactor es grande

---

## GESTIÓN DE ESPECIFICACIONES EN GIT

### Estructura
```
Codigo-Workspace/
├── SPECS_GENERAL.md              [Versionado - cambios raros]
├── SPEC_GAPS_ANALYSIS.md         [Versionado - cambios raros]
├── WORKFLOW_CAMBIOS.md           [Versionado - cambios muy raros]
├── SPEC_CHANGE_TEMPLATE.md       [Versionado - NUNCA CAMBIAR]
├── Cambios/
│   ├── SPEC_CHG-001.md           [Versionado - completado]
│   ├── SPEC_CHG-002.md           [WIP]
│   └── SPEC_CHG-NNN.md           [Template para nuevo cambio]
├── HISTORIAL_CAMBIOS.md          [Versionado - append only]
└── Codigo_v4.0 - FC Digitalesv01.cpp
```

### Commit Message Estándar
```
[SPEC_CHG-NNN] Descripción breve (50 chars max)

Descripción más larga si es necesario (wrap at 72).
Línea vacía anterior al cuerpo.

Especificación: SPEC_CHG-NNN.md
Testing: [Qué se probó]
Impacto: [Comportamiento anterior vs nuevo]

Fixes #[issue si aplica]
```

**Ejemplo:**
```
[SPEC_CHG-001] Agregar timeout en motor etiqueta

Implementa límite máximo de 10s en operación
del motor etiqueta. Si se alcanza, motor se apaga
y se registra error ERR-001.

Especificación: SPEC_CHG-001.md
Testing: Unitario y simulación en 3 casos
Impacto: Motor no puede quedar encendido indefinidamente
```

---

## CHECKLIST PARA REVISOR

Antes de aprobar un SPEC_CHG-NNN:

- ☐ Justificación es clara y válida
- ☐ Especificación no crea conflictos con SPECS_GENERAL.md
- ☐ Riesgos están identificados
- ☐ Plan de testing es completo
- ☐ Complejidad es realista para el scope
- ☐ Cambios son alcanzables
- ☐ No introduce deuda técnica
- ☐ Mejora o mantiene confiabilidad
- ☐ Aprobaciones necesarias están recolectadas

---

## HERRAMIENTAS Y COMANDOS ÚTILES

### Compilar
```bash
# En VS Code o Arduino IDE
# Verificar: Ctrl+Alt+V (Arduino)
# Subir: Ctrl+Alt+U (Arduino)
```

### Git Workflow
```bash
# Ver especificación de cambio
cat SPEC_CHG-001.md

# Staged changes
git diff --staged

# Ver commits con specs
git log --grep="SPEC_CHG"

# Ver rama de feature (si aplica)
git checkout -b feature/CHG-001
```

### Checklist Rápido Pre-Commit
```bash
# 1. ¿Hay SPEC_CHG-NNN.md aprobado?
ls -la Cambios/SPEC_CHG-*.md

# 2. ¿Código compila?
# [Compilar en IDE]

# 3. ¿Qué cambios hay?
git diff --staged

# 4. ¿Es consistente con spec?
# [Revisar SPEC_CHG-NNN.md]

# 5. ¿Tests pasan?
# [Ejecutar tests si aplica]

# Entonces: commit
git add Cambios/SPEC_CHG-NNN.md Codigo_v4.0*.cpp
git commit -m "[SPEC_CHG-NNN] Descripción"
```

---

## RESPONSABILIDADES

| Rol | Responsabilidad |
|-----|-----------------|
| Desarrollador | Crear SPEC_CHG-NNN completo, implementar cambio, testing |
| Revisor Técnico | Validar especificación, revisar código, tests |
| Responsable Sistema | Aprobar cambio, verificar impacto en producción |

---

## VERSIONADO DE DOCUMENTOS BASE

**SPECS_GENERAL.md**
- v1.0 (2026-03-15): Documento inicial
- v1.1 (FUTURO): Después CHG-001, CHG-002

**SPEC_GAPS_ANALYSIS.md**
- v1.0 (2026-03-15): Análisis inicial
- v1.1 (FUTURO): Después implementar CHG-001, CHG-002

---

## EJEMPLO: APLICAR ESTE WORKFLOW

### Caso: "Agregar timeout en motores"

1. **Crear especificación**
   ```
   cp SPEC_CHANGE_TEMPLATE.md Cambios/SPEC_CHG-001.md
   # Editar: llenar todas las secciones
   # Resultado: documento de 2-3 páginas
   ```

2. **Revisar**
   ```
   ¿Conflicta con SPECS_GENERAL.md? No
   ¿Rompe algo? No, es adición de seguridad
   ¿Riesgos? Bajo, cambio localizado
   → APROBADO
   ```

3. **Implementar**
   ```cpp
   // En loop principal, agregar después cada while()
   if (millis() - inicio_motor_etiqueta > MOTOR_ETI_TIMEOUT_MS) {
     digitalWrite(PIN_MOTOR_ETI, LOW);
     // ... error handling
   }
   ```

4. **Testing**
   - ✓ Compilación sin errors
   - ✓ Lógica: timeout se dispara a 10s
   - ✓ Motor se apaga correctamente
   - ✓ No rompe ciclo normal

5. **Commit**
   ```
   [SPEC_CHG-001] Agregar timeout motor etiqueta (10s)
   ```

6. **Resultado**
   - Código más seguro
   - Cambio documentado y trazable
   - Próxima persona entiende por qué se hizo

---

## RESUMEN

**Antes de tocar código:**
1. ✓ Crear/Revisar especificación (SPEC_CHG-NNN.md)
2. ✓ Conseguir aprobación
3. ✓ Implementar siguiendo spec exactamente
4. ✓ Testing exhaustivo
5. ✓ Actualizar documentos
6. ✓ Commit con referencia a spec

**Beneficios:**
- No hay cambios sorpresa
- Debugging es más fácil (hay historial)
- Pruebas son exhaustivas
- Código es trazable
- Próximo desarrollador entiende las decisiones

---

**Workflow v1.0**
**Última actualización:** 2026-03-15
