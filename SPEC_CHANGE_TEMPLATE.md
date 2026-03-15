# TEMPLATE - ESPECIFICACIÓN DE CAMBIO
Usar este template ANTES de hacer cambios en el código. Completar todas las secciones.

---

## INFORMACIÓN GENERAL
- **ID Cambio:** [ej. CHG-001]
- **Título:** [Breve descripción del cambio]
- **Autor:** [Nombre]
- **Fecha:** [YYYY-MM-DD]
- **Estado:** [ ] Propuesto [ ] Aprobado [ ] En Desarrollo [ ] Completado

---

## JUSTIFICACIÓN
### Problema/Necesidad
Describir qué problema resuelve o qué necesidad satisface este cambio.

### Impacto Actual
¿Cómo afecta la ausencia de este cambio al sistema actual?

### Beneficio Esperado
¿Qué mejora trae este cambio? (rendimiento, confiabilidad, mantenibilidad, etc.)

---

## ESPECIFICACIÓN TÉCNICA

### Cambios en Hardware
- [ ] Nuevos pines
- [ ] Cambios en conexiones
- [ ] Nuevos periféricos

**Detalles:**
```
[Describir cambios de hardware si aplica]
```

### Cambios en Lógica
**Área afectada:** [ej. Detección de botellas / Control de motores / LCD]

**Módulos afectados:**
- [ ] setup()
- [ ] loop()
- [ ] Función: ___________
- [ ] Función: ___________

**Cambios propuestos:**
```
[Pseudocódigo o descripción lógica del cambio]
```

### Cambios en Tiempos/Parámetros
| Parámetro | Valor Actual | Valor Nuevo | Justificación |
|-----------|--------------|-------------|---------------|
| | | | |

### Cambios en Restricciones
- [ ] Nueva limitación de hardware
- [ ] Nuevo requisito de rendimiento
- [ ] Nueva limitación de seguridad

**Describir:**

---

## PRUEBAS REQUERIDAS

### Prueba Unitaria
```cpp
// Pseudocódigo de prueba
Dado: [condición inicial]
Cuando: [acción]
Entonces: [resultado esperado]
```

### Prueba de Integración
- [ ] Cambio no rompe ciclo etiquetado normal
- [ ] Interfaz LCD sigue siendo legible
- [ ] Botones siguen respondiendo
- [ ] Potenciómetros siguen funcionando

### Prueba de Campo
- [ ] Verificar con botellas reales
- [ ] Validar con velocidad de producción
- [ ] Confirmar no hay regresiones

### Casos de Error a Probar
1. [Error caso 1]
2. [Error caso 2]

---

## COMPATIBILIDAD

### Retrocompatibilidad
- [ ] Cambio es backward compatible
- [ ] Cambio requiere recalibración
- [ ] Cambio requiere hardware nuevo

**Detalles:**

### Configuración Anterior
¿Se puede usar código anterior? ¿Se necesita migración de parámetros?

---

## RIESGOS

| Riesgo | Probabilidad | Impacto | Mitigación |
|--------|--------------|---------|-----------|
| [Riesgo 1] | Alta/Media/Baja | Alto/Medio/Bajo | [Plan de mitigación] |
| [Riesgo 2] | | | |

---

## RECURSOS Y ESTIMACIÓN

### Complejidad
- [ ] Simple (< 50 líneas, 1 función)
- [ ] Media (50-200 líneas, múltiples funciones)
- [ ] Alta (> 200 líneas, refactor arquitectura)

### Tiempo Estimado
- Desarrollo: ____ horas
- Pruebas: ____ horas
- Total: ____ horas

### Dependencias
- [ ] Cambio anterior: CHG-___
- [ ] Esperar aprobación de: ___________

---

## PLAN DE IMPLEMENTACIÓN

### Pasos
1. [Paso 1]
2. [Paso 2]
3. [Paso 3]
...

### Validación Post-Cambio
- [ ] Código compila sin errores/warnings
- [ ] Pruebas unitarias pasan
- [ ] Pruebas integración pasan
- [ ] Sin regresiones en LCD
- [ ] Sin regresiones en sensores

---

## APROBACIÓN

| Rol | Nombre | Firma | Fecha |
|-----|--------|-------|-------|
| Desarrollador | | | |
| Revisor | | | |
| Responsable Sistema | | | |

---

## HISTORIAL DE CAMBIOS

| Versión Spec | Cambio | Fecha |
|--------------|--------|-------|
| 1.0 | Creación inicial | YYYY-MM-DD |

---

## NOTAS ADICIONALES
[Cualquier información adicional relevante]

---

**Para usar:** Copiar este archivo con nombre `SPEC_CHG-NNN.md` antes de hacer cambios.
