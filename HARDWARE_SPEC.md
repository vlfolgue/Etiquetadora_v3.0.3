# 📋 ESPECIFICACIÓN DE HARDWARE - ETIQUETADORA

## 🔧 Información General
- **Proyecto**: Máquina Etiquetadora Automática
- **Controlador Principal**: ESP32 (DevKit)
- **Fecha de creación**: 2026-03-20
- **Estado**: En documentación (Verificación de hardware pendiente)

---

## 🖥️ MICROCONTROLADOR

### ESP32 DevKit
| Especificación | Valor |
|---|---|
| Modelo | ESP32-DevKitC |
| Voltaje lógica | 3.3V |
| Pines GPIO | 36 (algunos multipropósito) |
| Resolución ADC | 12 bits (0-4095) |
| Frecuencia | 240 MHz |
| Comunicación | SPI, I2C, UART |

---

## 📌 DIAGRAMA DE PINES - ESP32

```
LADO IZQUIERDO                          LADO DERECHO
┌─────────────────────────────────┐    ┌─────────────────────────────────┐
│ GND                             │    │ VIN (5V del puerto USB)         │
│ 3V3 (3.3V salida)               │    │ GND                             │
│ EN (Reset)                      │    │ IO23                            │
│ IO36 (ADC) → PIN_POT2 ✓         │    │ IO19 → PIN_BTN_AJUSTES ✓       │
│ IO39 (ADC) → PIN_POT1 ✓         │    │ IO18                            │
│ IO34 (ADC) → PIN_FC2 ✓          │    │ IO5                             │
│ IO35 (ADC) → PIN_FC1 ✓          │    │ IO17 → PIN_IR_BOTELLA ✓        │
│ IO32                            │    │ IO16 → PIN_ACTUADOR ✓          │
│ IO33                            │    │ IO4                             │
│ IO25 → PIN_BTN_CONTRAS ✓        │    │ IO0                             │
│ IO26 → PIN_MOTOR_ETI (P26) ✓    │    │ IO2                             │
│ IO27 → PIN_MOTOR_CON (P27) ✓    │    │ IO15                            │
│ IO14                            │    │ GND                             │
│ IO12                            │    │ IO13                            │
│ GND                             │    │ GND                             │
│ IO13 (SPI CLK)                  │    │                                 │
│ SD2 (SPI)                       │    │                                 │
│ SD3 (SPI)                       │    │                                 │
│ CMD (SPI)                       │    │                                 │
│ CLK (SPI)                       │    │                                 │
│ SD0 (SPI)                       │    │                                 │
│ SD1 (SPI)                       │    │                                 │
│ GND                             │    │                                 │
│ IO21 → PIN_SDA (I2C LCD) ✓      │    │                                 │
│ IO22 → PIN_SCL (I2C LCD) ✓      │    │                                 │
│ TX (UART)                       │    │                                 │
│ RX (UART)                       │    │                                 │
└─────────────────────────────────┘    └─────────────────────────────────┘
```

✓ = Pin utilizado en el proyecto

---

## 📊 TABLA COMPLETA DE PINES UTILIZADOS

| ESP32 Pin | GPIO | Función | Tipo | Voltaje | Descripción |
|---|---|---|---|---|---|
| P21 | 21 | PIN_SDA | I2C OUTPUT | 3.3V | Línea SDA para LCD (Liquido Crystal I2C @ 0x27) |
| P22 | 22 | PIN_SCL | I2C OUTPUT | 3.3V | Línea SCL para LCD |
| P17 | 17 | PIN_IR_BOTELLA | DIGITAL INPUT | 5V | **[?]** Sensor infrarrojo detección botella |
| P34 | 34 | PIN_FC1 | DIGITAL INPUT | 5V | **[?]** Foto-interruptor 1 (posición etiquetas) |
| P35 | 35 | PIN_FC2 | DIGITAL INPUT | 5V | **[?]** Foto-interruptor 2 (posición contras) |
| P25 | 25 | PIN_BTN_CONTRAS | DIGITAL INPUT | 5V | **[?]** Botón selector contras |
| P19 | 19 | PIN_BTN_AJUSTES | DIGITAL INPUT | 5V | **[?]** Botón modo ajustes |
| P36 | 36 | PIN_POT2 | ADC INPUT | 0-3.3V | **[?]** Potenciómetro 2 (delay botella-actuador) |
| P39 | 39 | PIN_POT1 | ADC INPUT | 0-3.3V | **[?]** Potenciómetro 1 (delay etiqueta-contra) |
| P26 | 26 | PIN_MOTOR_ETI | DIGITAL OUTPUT | 3.3V | **[?]** Control motor etiquetadora → Transistor 2N2222 |
| P27 | 27 | PIN_MOTOR_CON | DIGITAL OUTPUT | 3.3V | **[?]** Control motor contraetiquetadora → Transistor |
| P16 | 16 | PIN_ACTUADOR | DIGITAL OUTPUT | 3.3V | **[?]** Control actuador neumático |

---

## 🔌 DIAGRAMA DE CONEXIONES

```
                           ESP32 DEVKIT
                    ┌──────────────────────┐
                    │                      │
        ┌───────────┤ P21 (SDA)    P22     ├────────────┐
        │           │ (SCL)               │             │
        │           └──────────────────────┘             │
        │                                                 │
        │                 I2C @ 0x27                     │
        │                                                 │
        ▼                                                 ▼
    ┌─────────┐                                    ┌──────────┐
    │   LCD   │                                    │ Sensores │
    │ 20x4    │                                    │  & Botón │
    │ 5V +12V │                                    │          │
    └─────────┘                                    └──────────┘
                                                        │
         ┌─────────────────────────────────────────────┼─────────────────┐
         │                                             │                 │
         ▼                                             ▼                 ▼
    ┌────────┐                                  ┌──────────┐      ┌──────────┐
    │ P17    │                                  │ P34, P35 │      │ P25, P19 │
    │ GPIO17 │                                  │ GPIO34/35│      │ GPIO25/19│
    │        │                                  │          │      │          │
    │ IR     │                                  │ FC1, FC2 │      │ Botones  │
    └────────┘                                  └──────────┘      └──────────┘


    ┌──────────────────────────────────────┐
    │     PINES DE SALIDA (CONTROL)        │
    └──────────────────────────────────────┘
         │                │                │
         ▼                ▼                ▼
    ┌────────┐      ┌────────┐      ┌────────┐
    │ P26    │      │ P27    │      │ P16    │
    │GPIO26  │      │GPIO27  │      │GPIO16  │
    │        │      │        │      │        │
    │MOTOR   │      │MOTOR   │      │ACTUADOR│
    │ETIQUETA│      │CONTRA  │      │NEUMAT. │
    └────────┘      └────────┘      └────────┘
         │                │                │
         ▼                ▼                ▼
    ┌──────────┐   ┌──────────┐   ┌──────────┐
    │Transistor│   │Transistor│   │**[?]**   │
    │2N2222    │   │**[?]**   │   │          │
    │(Base)    │   │          │   │          │
    └──────────┘   └──────────┘   └──────────┘
         │                │                │
         ▼                ▼                ▼
    ┌──────────┐   ┌──────────┐   ┌──────────┐
    │ VARIADOR │   │ VARIADOR │   │ SOLENOIDE│
    │MOTOR ETI │   │MOTOR CON │   │ O RELÉ  │
    │(X1)      │   │(X1)      │   │**[?]**   │
    └──────────┘   └──────────┘   └──────────┘
         │                │                │
         ▼                ▼                ▼
    ┌──────────┐   ┌──────────┐   ┌──────────┐
    │MOTOR AC  │   │MOTOR AC  │   │CILINDRO │
    │ETIQUETA  │   │CONTRA    │   │NEUMÁTICO│
    │220V      │   │220V      │   │**[?]**V │
    └──────────┘   └──────────┘   └──────────┘
```

---

## 🔋 CIRCUITO DE CONTROL P26 (MOTOR ETIQUETADORA)

**Estado actual: Problemas de apagado identificados**

```
ESP32 GPIO26 (3.3V)
        │
        ├─[**R base = ?**kΩ]─────┐
        │                         │
        │                         ├─(Base) 2N2222 NPN
        │                         │
        └─[C descarga = ? µF?]──┤ (Colector) → X1 Variador
                                 │
                            (Emisor)
                                 │
                                GND

**PREGUNTAS URGENTES:**
- [?] ¿Resistencia de base presente? ¿Cuanto ohmios?
- [?] ¿Hay capacitor entre Base-GND? ¿Capacidad?
- [?] ¿Hay diodo de protección? ¿Cuál?
- [?] ¿Está bien soldado todo?
- [?] ¿El Emisor va directo a GND?
- [?] ¿Colector va solo a X1 o hay más conexiones?
```

---

## ⚡ TABLA DE COMPONENTES HARDWARE

### Entrada/Sensores

| Componente | Modelo | Función | Voltaje | Observaciones |
|---|---|---|---|---|
| Sensor IR | **[?]** | Detectar botella | 5V (¿) | **[?] Reflectivo/Barrera?** |
| Foto-interruptor 1 | **[?]** | Posición etiquetas | 5V (¿) | **[?] Marca/Modelo?** |
| Foto-interruptor 2 | **[?]** | Posición contras | 5V (¿) | **[?] Marca/Modelo?** |
| Botón Contras | **[?]** | Selector funcional | 5V (¿) | **[?] Lógica HIGH/LOW?** |
| Botón Ajustes | **[?]** | Modo configuración | 5V (¿) | **[?] Lógica HIGH/LOW?** |
| POT1 | **[?]** | Variable delay 1 | 3.3V (¿) | **[?] Rango lineal completo?** |
| POT2 | **[?]** | Variable delay 2 | 3.3V (¿) | **[?] Rango lineal completo?** |

### Display

| Componente | Modelo | Especificación | Voltaje |
|---|---|---|---|
| LCD | LiquidCrystal_I2C | 20x4 caracteres | 5V |
| I2C Backpack | Genérico | Dirección 0x27 | 5V |

### Control de Motores (Problemas Activos)

| Componente | Modelo | Función | Voltaje | **VERIFICAR** |
|---|---|---|---|---|
| Transistor P26 | 2N2222 | Activar X1 variador etiqueta | 3.3V → 5V (¿) | **¿Por qué no apaga?** |
| Transistor P27 | **[?]** | Activar X1 variador contra | 3.3V | **[?] Modelo?** |
| Variador Motor Etiqueta | **[?]** | Control velocidad motor | 220V AC | **[?] Marca/Modelo? ¿Tiene X2 STOP?** |
| Variador Motor Contra | **[?]** | Control velocidad motor | 220V AC | **[?] Marca/Modelo?** |
| Motor Etiqueta | **[?]** | Motor principal | 220V AC | **[?] HP? RPM?** |
| Motor Contra | **[?]** | Motor secundario | 220V AC | **[?] HP? RPM?** |

### Actuador

| Componente | Modelo | Función | Voltaje | Control |
|---|---|---|---|---|
| Actuador Neumático | **[?]** | Extender/Retraer brazo | **[?]V** | **[?] Relé/Transistor/Solenoide?** |
| Cilindro/Brazo | **[?]** | Mecánica aplicación | Neumático | **[?] Tamaño? Recorrido?** |

---

## 🔗 CONEXIONES EXTERNAS A VERIFICAR

### Botón Manual X1
**Ubicación en la máquina**: **[?]**
**Función**: Activar/Parar motor etiquetadora manualmente
**Conexión actual**:
- ¿Va directo al variador?
- ¿Pasa por el transistor de P26?
- ¿Es independiente?

**PREGUNTAS**:
- [?] ¿Dónde exactamente está este botón en el hardware?
- [?] ¿Qué conector/cable usa?
- [?] ¿Va a X1 del variador directamente o pasa por transistor?

### Conexión GND (Tierra)
**Estado**: Crítico para funcionamiento
- [?] ¿Hay cable común de GND para sensores y controles?
- [?] ¿Los GND de 5V y 3.3V están unidos?
- [?] ¿El GND de los transistores es común?

### Alimentación
- [?] ¿De dónde viene la alimentación 5V de sensores?
- [?] ¿Hay fuente externa o viene de USB del ESP32?
- [?] ¿Hay fuente separada para los motores/variadores?

---

## 🔴 PROBLEMAS ACTIVOS

### P1: Motor Etiquetadora no se detiene (P26)

**Síntomas**:
- GPIO 26 en HIGH → Motor gira ✓
- GPIO 26 en LOW → Motor sigue girando ✗
- Desconectar ESP32 → Motor se detiene ✓

**Análisis actual**:
- No es problema del código (cambios recientes solo agregan garantías)
- No es problema de timing (100µs delay no afecta)
- **Probable causa: Hardware del transistor/circuito**

**Verificaciones requeridas**:
- [ ] Medir voltaje en Base del transistor cuando GPIO 26 = LOW
- [ ] Verificar capacitor de descarga (si existe)
- [ ] Revisar resistencia de base
- [ ] Comprobar soldaduras transistor
- [ ] Revisar si X1 del variador tiene otra fuente de señal

---

## 📝 PARÁMETROS DE OPERACIÓN (Código)

### Configurables por usuario
| Parámetro | Rango | Actual | Función |
|---|---|---|---|
| delay_botella_actuador | 0-1500ms | 0ms | Delay antes extender actuador |
| delay_etiqueta_contra | 0-1500ms | 200ms | Delay entre etiqueta y contraetiqueta |

### Fijos en código
| Parámetro | Valor | Función |
|---|---|---|
| MOTOR_IGNITION_MS | 200ms | Ventana detección flanco motor |
| DELAY_POST_ACTUADOR_MS | 50ms | Espera tras extender actuador |
| tiempo_parada_actuador | 300ms | Tiempo que se mantiene extendido |
| TIMEOUT_MOTOR_MS | 10000ms | Máximo para girar (detección error) |
| STABLE_MS | 5ms | Anti-ruido FC1/FC2 |
| STABLE_IR_MS | 20ms | Anti-ruido sensor IR |

---

## ✅ CHECKLIST DE VERIFICACIÓN

- [ ] **Pines ESP32**: Todos conectados según tabla
- [ ] **Sensores IR/FC**: Valores correctos en Serial Monitor
- [ ] **Botones**: Sin rebote, responden a presión
- [ ] **Potenciómetros**: Rango 0-4095 ADC completo
- [ ] **LCD**: Muestra datos correctamente @ 0x27
- [ ] **P26 (Motor Etiqueta)**: HIGH enciende, LOW apaga (¿REVISAR?)
- [ ] **P27 (Motor Contra)**: HIGH enciende, LOW apaga
- [ ] **P16 (Actuador)**: Extiende/Retrae correctamente
- [ ] **Variadores**: Responden a X1
- [ ] **Tierra (GND)**: Bien distribuida
- [ ] **Voltaje 5V**: Estable sin ruido
- [ ] **Voltaje 3.3V**: Estable sin ruido

---

## 📚 REFERENCIAS

- **Código**: `src/main.cpp`
- **Config PlatformIO**: `platformio.ini`
- **Librerías**:
  - LiquidCrystal_I2C (LCD via I2C)
  - Wire (I2C communication)
  - Arduino (ESP32 framework)

---

**Última actualización**: 2026-03-20 | **Estado**: En desarrollo
**Próximo paso**: Abrir cuadro y verificar componentes físicos
