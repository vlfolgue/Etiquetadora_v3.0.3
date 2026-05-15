# ❓ PREGUNTAS DE HARDWARE - ETIQUETADORA

Almacén de todas las preguntas pendientes para verificar cuando abras la placa.
**Organizado por prioridad y categoría**

---

## 🔴 CRÍTICAS (Bloquean funcionamiento)

### P26 - Motor Etiquetadora no apaga

**Estado**: Bloqueante - Investigación activa
**Síntomas**: Motor sigue girando aunque GPIO 26 = LOW

#### Preguntas:

1. **¿Voltaje en la base del transistor?**
   - Cuando GPIO 26 = HIGH (activo): ¿Cuánto voltaje hay en la base?
   - Cuando GPIO 26 = LOW (inactivo): ¿Cuánto voltaje hay en la base?
   - Rango esperado: 0V cuando LOW, ~2.5-3.3V cuando HIGH

2. **¿Capacitor de descarga?**
   - ¿Hay algún capacitor conectado entre Base y GND?
   - Si existe: ¿Cuál es su capacidad? (nF, µF?)
   - Si no existe: ¿Deberíamos agregar uno?

3. **¿Resistencia de base?**
   - ¿Hay resistencia entre GPIO 26 y Base del transistor?
   - Si existe: ¿Cuántos ohmios? (1k, 10k, 100k?)
   - ¿Es la correcta para saturar un 2N2222?

4. **¿Diodo de protección?**
   - ¿Hay diodo entre Colector-Emisor?
   - ¿Qué modelo? (1N4148, 1N4007?)
   - ¿Está en la dirección correcta?

5. **¿Soldaduras del transistor?**
   - ¿Están bien soldados los tres pines?
   - ¿Hay fracturas o soldaduras frías?
   - ¿Está bien identificado el modelo (2N2222)?

6. **¿Conexión de GND?**
   - ¿El Emisor va directo a GND?
   - ¿Hay un punto de GND común para todo?
   - ¿El GND del ESP32 está conectado al GND del circuito?

7. **¿Voltaje de saturación?**
   - Cuando GPIO 26 = LOW: ¿Hay voltaje residual en el colector?
   - ¿Es mayor de 0.2V? (Si es sí, transistor no apaga bien)

### X1 del Variador - Comportamiento anómalo

**Estado**: Bloqueante - Relacionado con P26
**Síntoma**: X1 sigue siendo activado aunque transistor debería estar OFF

#### Preguntas:

8. **¿X1 tiene otra fuente de señal?**
   - ¿Botón manual X1 está conectado solo a transistor P26?
   - ¿O hay otra entrada/componente alimentando X1?
   - ¿El variador tiene internamente un latch?

9. **¿Terminal STOP del variador?**
   - ¿El variador tiene terminal X2 o STOP?
   - ¿Está conectado a algo o libre?
   - ¿Debería estarlo?

10. **¿Tipo de control del variador?**
    - ¿Es monostable (necesita pulso continuo) o latch (se queda encendido)?
    - ¿Una vez presionado START, cómo se detiene?
    - ¿Necesita presionar un STOP físico separado?

---

## 🟠 ALTA PRIORIDAD (Necesarios para funcionamiento correcto)

### Motor Contraetiquetadora (P27)

11. **¿Transistor P27?**
    - ¿Es también 2N2222 o diferente?
    - ¿Model exacto?
    - ¿Tiene los mismos problemas que P26?

12. **¿Circuito de P27 idéntico a P26?**
    - ¿Base con resistencia?
    - ¿Capacitor de descarga?
    - ¿Diodo de protección?

### Actuador Neumático (GPIO 16)

13. **¿Cómo se controla exactamente?**
    - ¿Es un Relé, Transistor, Solenoide?
    - ¿Modelo exacto?
    - ¿Especificaciones?

14. **¿Voltaje de operación?**
    - ¿5V, 12V, 24V?
    - ¿Corriente que consume?

15. **¿Comportamiento esperado?**
    - GPIO 16 = HIGH: Extiende brazo
    - GPIO 16 = LOW: Retrae brazo
    - ¿O es lo contrario?

### Sensores (Voltajes)

16. **¿Sensor IR voltaje real?**
    - ¿Es 5V o 3.3V?
    - ¿Hay resistor de pull-up/pull-down?
    - ¿Necesita protección de voltaje (divisor)?

17. **¿FC1 y FC2 voltajes?**
    - ¿Son 5V o 3.3V?
    - ¿Tienen pull-up internos?
    - ¿Necesitan protección?

18. **¿Botones P25 y P19 voltajes?**
    - ¿Son 5V o 3.3V?
    - ¿Lógica activo HIGH o LOW?
    - ¿Resistencias de pull-up/pull-down?

### Potenciómetros

19. **¿POT1 y POT2 conexionado?**
    - ¿De dónde vienen 3.3V?
    - ¿GND está bien?
    - ¿Wiper va directo a GPIO o hay resistor?
    - ¿Rango completo 0-4095?

---

## 🟡 MEDIA PRIORIDAD (Mejora)

### Alimentación General

20. **¿Fuente de 5V?**
    - ¿De dónde viene para los sensores?
    - ¿Es fuente externa o del USB ESP32?
    - ¿Capacidad (amperage)?

21. **¿Fuente de 3.3V?**
    - ¿Viene del regulador del ESP32?
    - ¿Capacidad de carga?
    - ¿Hay condensador de desacoplamiento?

22. **¿Separación de GND?**
    - ¿GND de 5V y 3.3V es común?
    - ¿O están separados con resistor/inductor?

### Conectores y Cables

23. **¿Conectores utilizados?**
    - ¿Tipos? (pinhead, bornes, JST?)
    - ¿Están bien ajustados?
    - ¿Hay oxidación o corrosión?

24. **¿Cables de señal vs potencia?**
    - ¿Están separados (sin ruido)?
    - ¿Blindados?
    - ¿Longitud de cada uno?

### Motores/Variadores

25. **¿Variador Motor Etiqueta**
    - Marca: **[?]**
    - Modelo: **[?]**
    - Entrada: AC 220V (¿confirmado?)
    - Terminales de control: X1, X2 (STOP?), RUN
    - ¿Comportamiento de latch o momentáneo?

26. **¿Variador Motor Contra**
    - Marca: **[?]**
    - Modelo: **[?]**
    - Especificaciones iguales a etiqueta?

27. **¿Motor Etiquetadora (AC)**
    - HP: **[?]**
    - RPM: **[?]**
    - Voltaje: 220V AC
    - Frecuencia: 50Hz o 60Hz?

28. **¿Motor Contraetiquetadora (AC)**
    - Especificaciones iguales a etiqueta?

---

## 🟢 BAJA PRIORIDAD (Documentación)

### Especificaciones Mecánicas

29. **¿Actuador neumático?**
    - Carrera (recorrido): **[?]** mm
    - Diámetro cilindro: **[?]** mm
    - Fuerza máxima: **[?]** kg

30. **¿Sensores de posición exactos?**
    - FC1: ¿Distancia hasta rueda de etiquetas?
    - FC2: ¿Distancia hasta rueda de contras?
    - ¿Tienen ajuste de sensibilidad?

### Tuberías y Aire

31. **¿Sistema neumático?**
    - ¿De dónde viene aire comprimido?
    - ¿Presión de trabajo?
    - ¿Hay válvula de escape?

### Mantenimiento

32. **¿Historial de problemas previos?**
    - ¿Cuándo fue la última reparación?
    - ¿Qué se cambió?
    - ¿Funcionaba bien antes?

---

## 📋 PLANTILLA PARA VERIFICACIÓN EN CAMPO

Cuando abras la placa, usa este template:

```
FECHA DE VERIFICACIÓN: [___________]
HORA: [___________]

TRANSISTOR P26:
  Base: [________] voltios (LOW), [________] voltios (HIGH)
  Colector: [________] voltios (LOW), [________] voltios (HIGH)
  Modelo verificado: [________] ✓ / ✗
  Soldaduras: [Bien] [Fracturadas] [Dudosas]
  Capacitor Base-GND: [Sí] [No] Capacidad: [_________]
  Resistencia Base: [_________] ohmios
  Diodo: [_________] modelo

VARIADOR ETIQUETAS:
  Modelo: [_________]
  Marca: [_________]
  X1: [Conectado a transistor] [Conectado a botón] [Ambos]
  X2/STOP: [Existe] [No existe] [Conectado]
  Comportamiento: [Latch] [Momentáneo]

SENSORES:
  IR Voltaje: [_________] V (Level: HIGH/LOW)
  FC1 Voltaje: [_________] V (State: HOME/ETIQUETA)
  FC2 Voltaje: [_________] V (State: HOME/ETIQUETA)

ALIMENTACIÓN:
  5V: [_________] V (Estable: Sí/No)
  3.3V: [_________] V (Estable: Sí/No)
  GND común: [Sí] [No]

OBSERVACIONES:
[_________________________________________________________________]
[_________________________________________________________________]
[_________________________________________________________________]
```

---

## 🎯 CONCLUSIÓN PENDIENTE

**Conclusión del análisis**: Cuando tengas verificado todo arriba, completaremos el HARDWARE_SPEC.md con los valores reales.

**Archivo relacionado**: `HARDWARE_SPEC.md`

---

Última actualización: 2026-03-20
Estado: Esperando verificación física del hardware
