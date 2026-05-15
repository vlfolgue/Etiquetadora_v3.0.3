# Specification Quality Checklist: FC Sensor HOME → ETIQUETA → HOME

**Purpose**: Validar completitud y calidad de la spec antes de pasar a planning
**Created**: 2026-05-15
**Feature**: [spec.md](../spec.md)

## Content Quality

- [x] CHK001 Sin detalles de implementación (lenguajes, frameworks, APIs) — la spec describe comportamiento observable, no código
- [x] CHK002 Centrado en valor de negocio/operario — cada FR tiene impacto en producción
- [x] CHK003 Legible por personal técnico de taller y de software — glosario físico incluido
- [x] CHK004 Todas las secciones obligatorias completadas

## Requirement Completeness

- [x] CHK005 Sin marcadores [NEEDS CLARIFICATION] — todos los casos resueltos con supuestos documentados
- [x] CHK006 Requisitos son testables y sin ambigüedad — cada FR indica condición medible
- [x] CHK007 Success criteria son cuantitativos — SC-001 a SC-007 con métricas o porcentajes
- [x] CHK008 Success criteria no mencionan tecnología específica (GPIO, ESP32, etc.)
- [x] CHK009 Todos los acceptance scenarios definidos — cuatro User Stories con Given/When/Then
- [x] CHK010 Edge cases identificados — 6 casos límite documentados
- [x] CHK011 Alcance claramente delimitado — se especifica qué incluye y qué excluye la feature
- [x] CHK012 Dependencias y supuestos identificados — sección Assumptions & Dependencies completa

## Feature Readiness

- [x] CHK013 Todos los FR tienen acceptance scenarios que los cubren
- [x] CHK014 User stories cubren flujo nominal (US1), caso alternativo (US2), fallo (US3), ruido (US4)
- [x] CHK015 La feature cumple los measurable outcomes definidos en Success Criteria
- [x] CHK016 Sin detalles de implementación en la spec — no aparecen variables, funciones ni fragmentos de código

## Notes

- La spec adapta el template (orientado a web app) al dominio de firmware embebido industrial.
  Las "User Stories" representan escenarios de operación de máquina, no interacciones de usuario web.
- SC-003 (0% falsos paros por ruido) es una aspiración de producción nominal; en entornos
  con interferencia extrema puede requerir revisión del umbral STABLE_MS=5ms.
- Los tiempos mencionados (5 ms, 10 s, 300 ms) son requisitos funcionales que SÍ pueden
  aparecer en la spec porque son directamente observables y medibles por el operario o
  con un osciloscopio, no son detalles internos de implementación.
