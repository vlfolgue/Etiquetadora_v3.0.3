# Feature Specification: Detección FC Sensor HOME → ETIQUETA → HOME

**Feature Branch**: `001-fc-home-eti-home`  
**Created**: 2026-05-15  
**Status**: Draft  
**Firmware**: ESP32 / Arduino — Etiquetadora automática de botellas v3.0.3  

---

## Contexto del Sistema

La etiquetadora es una máquina industrial que aplica etiqueta frontal y contraetiqueta
a botellas de vino que pasan en cinta transportadora. El control de cada motor de
aplicación (variador de frecuencia) se basa en una fotocelda reflexiva (FC) que detecta
la posición del rollo de etiquetas en tiempo real.

### Anatomía física del rollo de etiquetas

```
  [GAP] [ETIQUETA-1] [GAP] [ETIQUETA-2] [GAP] [ETIQUETA-3]
   ^^^                ^^^                ^^^
   HIGH               HIGH               HIGH     ← FC lee HIGH (entre etiquetas)
        ^^^^^^^^^^^^       ^^^^^^^^^^^^
        LOW                LOW                    ← FC lee LOW (sobre la etiqueta)
```

- **GAP (espacio entre etiquetas)** → La fotocelda no detecta material reflectante →
  señal HIGH (reposo)
- **ETIQUETA (sobre la etiqueta)** → La fotocelda detecta el material de la etiqueta →
  señal LOW (activo)

### Definición de posiciones

| Posición | Nombre en código | Nivel lógico FC | Descripción física |
|----------|-----------------|------------------|--------------------|
| HOME | `FCentreetiquetas = true` | HIGH | El sensor está en el GAP entre etiquetas. El rollo está posicionado correctamente para que la siguiente etiqueta quede frente al aplicador. |
| ETIQUETA | — | LOW | El sensor está pasando por encima del cuerpo de la etiqueta. |

### Los dos motores y sus FC

| Motor | Pin GPIO | FC | Pin GPIO | Variable de control |
|-------|----------|----|----------|---------------------|
| Motor Etiqueta (ETI) | GPIO26 (DAC2) | FC1 | GPIO34 (input-only) | `mover1`, `etiquetapuesta` |
| Motor Contraetiqueta (CON) | GPIO27 | FC2 | GPIO35 (input-only) | `mover2`, `contrapuesta` |

**Restricción hardware crítica**: GPIO34 y GPIO35 en ESP32 son pines de solo entrada
y carecen de resistencia pull-up o pull-down interna. Esto los hace susceptibles a
capturar ruido eléctrico del entorno industrial, especialmente mientras los variadores
de frecuencia están operando. Toda lectura de estos pines debe estar filtrada.

---

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Ciclo completo con FC en posición HOME al arrancar (Priority: P1)

El operario coloca una botella en la cinta. El sensor IR detecta la botella. El
rollo de etiquetas está en posición correcta de reposo (FC en HIGH, entre etiquetas).
El sistema debe arrancar el motor, esperar a que el rollo pase completamente por una
etiqueta (FC baja a LOW y vuelve a subir a HIGH) y parar el motor en el siguiente GAP.
La etiqueta ha quedado aplicada sobre la botella.

**Why this priority**: Es el caso nominal de operación. Si este caso falla, ninguna
botella queda etiquetada. Es el flujo de mayor impacto productivo.

**Independent Test**: Posicionar manualmente el rollo en el GAP (FC debe leer HIGH en
LCD). Arrancar ciclo. Verificar en LCD que FC pasa a LOW y vuelve a HIGH. El motor
debe parar solo. La etiqueta debe haber avanzado exactamente una posición.

**Acceptance Scenarios**:

1. **Given** el rollo está en HOME (FC1 = HIGH estable), **When** se detecta una
   botella y arranca el motor ETI, **Then** el motor continúa girando mientras FC1
   esté en HIGH (no ha llegado a la etiqueta todavía).

2. **Given** el motor ETI está corriendo y FC1 estaba en HIGH, **When** FC1 pasa a
   LOW de forma estable (>5 ms) porque el rollo ha llegado al cuerpo de la etiqueta,
   **Then** el sistema registra internamente que ha visto la etiqueta (`fc1_vio_etiqueta = true`).

3. **Given** `fc1_vio_etiqueta = true` (ya se vio la etiqueta), **When** FC1 vuelve
   a HIGH de forma estable (>5 ms) porque el rollo ha pasado la etiqueta y ha llegado
   al siguiente GAP, **Then** el motor ETI se para inmediatamente y `etiquetapuesta = true`.

4. **Given** el mismo ciclo pero para contraetiqueta (FC2/motor CON), **Then** exactamente
   la misma secuencia aplica tras un retardo configurable después de que la etiqueta
   principal quedó puesta.

---

### User Story 2 — Ciclo con FC en medio de una etiqueta al arrancar (Priority: P1)

El operario arrancó el ciclo anterior pero el rollo se detuvo por una incidencia justo
en medio de una etiqueta. FC1 lee LOW al inicio del nuevo ciclo. El sistema debe
reconocer que ya estamos sobre una etiqueta, arrancar el motor, esperar solo a que el
rollo llegue al siguiente GAP (HOME) y parar ahí. No debe exigir pasar por una etiqueta
completa adicional.

**Why this priority**: Ocurre con frecuencia en entornos reales. Si el sistema lo trata
incorrectamente, el rollo avanzaría dos etiquetas en lugar de una, desperdiciando material
y desalineando la máquina.

**Independent Test**: Posicionar manualmente el rollo sobre el cuerpo de una etiqueta
(FC1 debe leer LOW en LCD). Arrancar ciclo. El motor debe parar en el primer HIGH
que encuentre, sin esperar un LOW previo.

**Acceptance Scenarios**:

1. **Given** el rollo está sobre una etiqueta (FC1 = LOW estable al inicio del ciclo),
   **When** se detecta la botella y arranca el motor ETI, **Then** el sistema inicializa
   `fc1_vio_etiqueta = true` (etiqueta ya vista) sin necesitar detectar ningún flanco
   LOW adicional.

2. **Given** `fc1_vio_etiqueta = true` desde el inicio, **When** FC1 pasa a HIGH
   estable (>5 ms) porque el rollo llegó al siguiente GAP, **Then** el motor ETI
   se para y `etiquetapuesta = true`.

3. **Given** el mismo escenario, **Then** el rollo habrá avanzado exactamente hasta
   el siguiente GAP, equivalente a una posición de etiqueta aplicada, sin avanzar de más.

---

### User Story 3 — Protección por timeout: FC nunca cambia de estado (Priority: P2)

Un sensor FC se desconecta, se rompe o el rollo de etiquetas se acaba. El motor
lleva corriendo más de 10 segundos sin que la detección HOME→ETIQUETA→HOME complete.
El sistema debe parar el motor de forma segura, retraer el actuador, mostrar un error
claro en pantalla y esperar a que el operario lo resuelva. No debe quedar ningún
motor encendido de forma indefinida.

**Why this priority**: Seguridad de la máquina y del material. Un motor de variador
funcionando sin control destruiría el rollo de etiquetas y podría dañar el mecanismo.

**Independent Test**: Desconectar el cable del FC1. Arrancar un ciclo. Verificar que
después de 10 segundos el motor para, el actuador retrae, y la LCD muestra el mensaje
de error del sensor. El motor no debe volver a arrancar hasta que el operario pulse
el botón de ajustes para limpiar el error.

**Acceptance Scenarios**:

1. **Given** el motor ETI lleva corriendo 10 segundos sin que FC1 complete el
   ciclo HOME→ETIQUETA→HOME, **Then** el sistema para el motor ETI, retrae el
   actuador, cancela el ciclo activo y activa `error_fc1_timeout = true`.

2. **Given** el motor CON lleva corriendo 10 segundos sin que FC2 complete el
   ciclo, **Then** misma acción pero activa `error_fc2_timeout = true`.

3. **Given** hay un error activo, **When** el operario pulsa el botón AJUSTES,
   **Then** el error se limpia, la LCD vuelve a la pantalla normal y el sistema
   está listo para el siguiente ciclo.

4. **Given** hay un error activo, **When** pasa otra botella por el IR, **Then**
   el sistema NO inicia un nuevo ciclo mientras el error esté presente.

---

### User Story 4 — Protección anti-ruido en GPIO34/GPIO35 (Priority: P1)

Los variadores de frecuencia generan interferencia electromagnética cuando operan.
GPIO34 y GPIO35 del ESP32 no tienen pull resistor interno. Un spike de ruido de
<5 ms que haga leer LOW cuando el FC está realmente en HIGH no debe confundirse
con una etiqueta real ni detener el motor prematuramente.

**Why this priority**: Sin este filtrado, un único pico de ruido provoca que el motor
pare a la mitad de la rotación: la etiqueta no se aplica, el rollo queda en posición
incorrecta, y el siguiente ciclo fallará también en cascada.

**Independent Test**: Con el motor ETI en marcha y FC1 en HIGH, generar un pulso
eléctrico breve (<5 ms) en el cable del FC1 (simulando interferencia). El motor
no debe parar. Solo si FC1 permanece LOW >5 ms consecutivos debe registrarse la
detección de etiqueta.

**Acceptance Scenarios**:

1. **Given** FC1 está en HIGH (HOME) y el motor ETI está corriendo, **When** FC1
   cae a LOW durante menos de 5 ms y vuelve a HIGH (spike de ruido), **Then**
   `fc1_vio_etiqueta` permanece `false` y el motor continúa.

2. **Given** FC1 está en HIGH y el motor corre, **When** FC1 cae a LOW y permanece
   LOW durante al menos 5 ms (etiqueta real), **Then** `fc1_vio_etiqueta` se pone
   a `true` y el motor queda en espera del retorno a HOME.

3. **Given** `fc1_vio_etiqueta = true` y el motor corre, **When** FC1 sube a HIGH
   pero solo durante menos de 5 ms (ruido o borde de etiqueta), **Then** el motor
   NO para: la transición a HOME debe ser también estable.

---

### Edge Cases

- **FC en transición al inicio del ciclo**: Si FC1 está justo cambiando de estado
  en el momento en que la botella activa el IR, la lectura de inicialización
  (estable 5 ms) distinguirá correctamente si estamos en HOME o en ETIQUETA.

- **FC1 lee LOW pero FC2 lee HIGH al inicio**: Cada sensor se inicializa
  independientemente. FC1 puede iniciar con `fc1_vio_etiqueta=true` mientras FC2
  inicia con `fc2_vio_etiqueta=false`. Ambos motores gestionan su propio estado
  de forma autónoma.

- **Interrupción de ciclo por error en ETI antes de que arranque CON**: Si el
  motor ETI genera timeout, el motor CON nunca arranca porque `etiquetapuesta`
  no se pone a `true`. El actuador retrae, ciclo completo cancelado.

- **Botón contras desactivado**: Si el operario ha desactivado las contraetiquetas
  (PIN_BTN_CONTRAS LOW), `mover2 = false` al inicio del ciclo y `contrapuesta = true`
  de inicio. El motor CON nunca arranca. El ciclo termina tras la etiqueta frontal.

- **Fallo eléctrico mid-cycle (reset ESP32)**: Al reiniciar, todas las variables
  vuelven a estado inicial seguro (`mover1=false`, `mover2=false`, motores LOW).
  La botella en proceso no es contada. El operario reinicia el ciclo manualmente.

- **Ciclo completo sin botella real (vibración en IR)**: La detección de IR exige
  20 ms estables en LOW. Una vibración corta no inicia ciclo y no mueve ningún motor.

---

## Assumptions & Dependencies

### Supuestos hardware confirmados

- **FC activo-LOW**: El sensor FC es de tipo NPN (o equivalente) que da señal LOW
  cuando detecta el material de la etiqueta y HIGH cuando no hay material. Esta
  polaridad está fijada en hardware y no es configurable por software.
- **HOME = HIGH**: La posición de reposo del rollo (entre etiquetas) produce
  siempre HIGH en los pines FC1 y FC2. Confirmado por el equipo de producción.
- **Etiqueta > 5 ms bajo el sensor**: A la velocidad máxima del variador, la etiqueta
  permanece bajo el sensor al menos 100 ms. El GAP entre etiquetas permanece
  bajo el sensor al menos 50 ms. Ambos valores están muy por encima del umbral
  de filtrado de 5 ms.
- **GPIO26 (PIN_MOTOR_ETI) es DAC2**: Requiere `dacDisable()` en `setup()` para
  evitar que el convertidor DAC produzca tensión analógica residual al iniciar,
  lo que podría arrancar el variador de forma no controlada en el primer segundo
  tras el encendido.
- **GPIO34 y GPIO35 son input-only**: No tienen pull resistor interno. Requieren
  filtrado por software. Los cables de estas señales deben ser lo más cortos posible
  y alejados de los cables de potencia de los variadores para minimizar la interferencia
  captada.

### Supuestos de operación

- El operario coloca botellas de forma individual, no en ráfaga. El tiempo mínimo
  entre botellas consecutivas es superior al tiempo de ciclo completo (~2-4 segundos).
- Los potenciómetros de ajuste están calibrados correctamente antes de producción.
  El delay entre botella y actuador (`delay_botella_actuador`) y entre etiqueta y
  contra (`delay_etiqueta_contra`) están dentro del rango 0-2000 ms.
- El variador de frecuencia tiene configurada su propia rampa de aceleración. La señal
  del ESP32 solo actúa como enable/disable del variador, no controla la velocidad.

### Dependencias del ciclo completo

La lógica HOME→ETIQUETA→HOME para el motor ETI (FC1) es la sección central del ciclo.
Depende de:

1. **Detección de botella (IR)**: El ciclo HOME→ETIQUETA→HOME solo se inicia cuando
   `detectada_botella = true`. Si el IR falla o da falsas detecciones, el ciclo de
   los FC nunca debe arrancar.
2. **Actuador extendido**: El motor ETI no arranca hasta que el actuador esté extendido
   (`actuador_fuera = true`) y haya transcurrido `DELAY_POST_ACTUADOR_MS = 50 ms`.
   El actuador acerca el aplicador a la botella antes de que el rollo empiece a girar.
3. **Motor CON depende de motor ETI**: El motor CON (FC2) solo puede arrancar cuando
   `etiquetapuesta = true`, es decir, cuando el motor ETI ya completó su ciclo
   HOME→ETIQUETA→HOME exitosamente. Además debe haber transcurrido `delay_etiqueta_contra`.
4. **Retracción actuador depende de ambos**: El actuador solo retrae cuando tanto
   `etiquetapuesta = true` como `contrapuesta = true`. Si las contraetiquetas están
   desactivadas, `contrapuesta` se inicializa a `true` al inicio del ciclo.

---

## Requirements *(mandatory)*

### Functional Requirements

**Detección y filtrado de señal FC:**

- **FR-001**: El sistema DEBE filtrar las lecturas de FC1 (GPIO34) y FC2 (GPIO35)
  exigiendo que el nivel detectado se mantenga estable durante al menos 5 ms
  consecutivos antes de considerarlo válido. Esto aplica tanto para detectar la
  posición ETIQUETA (LOW) como la posición HOME (HIGH).

- **FR-002**: El sistema DEBE realizar la lectura de inicialización de FC1 y FC2
  al inicio de cada ciclo usando el mismo filtrado de 5 ms, no una lectura
  instantánea, para garantizar que el estado inicial `fc1_vio_etiqueta` y
  `fc2_vio_etiqueta` se inicializa correctamente.

**Máquina de estados HOME→ETIQUETA→HOME:**

- **FR-003**: Si FC1 está en HOME (HIGH estable) al inicio del ciclo, el sistema
  DEBE esperar a detectar primero la posición ETIQUETA (LOW estable ≥5 ms) antes
  de poder parar el motor al detectar HOME. No puede parar en el primer HIGH que
  lea sin haber visto antes un LOW.

- **FR-004**: Si FC1 está en ETIQUETA (LOW estable) al inicio del ciclo, el sistema
  DEBE considerar la etiqueta como "ya vista" (`fc1_vio_etiqueta = true` desde el
  inicio) y parar el motor en el primer HIGH estable que detecte, sin necesitar
  pasar por un LOW previo adicional.

- **FR-005**: Los requisitos FR-003 y FR-004 aplican de forma idéntica e independiente
  para FC2 / motor CON con sus propias variables de estado (`fc2_vio_etiqueta`,
  `mover2`, `contrapuesta`).

- **FR-006**: Al detectar HOME tras haber visto ETIQUETA, el motor DEBE pararse
  inmediatamente (señal a variador = LOW) en el mismo ciclo de firmware en que
  se confirma el HIGH estable.

**Temporización y secuenciación:**

- **FR-007**: El motor ETI DEBE arrancarse solo después de que el actuador esté
  completamente extendido y haya transcurrido un mínimo de `DELAY_POST_ACTUADOR_MS`
  (50 ms) para asegurar que el aplicador está en contacto con la botella antes
  de mover el rollo.

- **FR-008**: El motor CON DEBE arrancarse solo después de que el motor ETI complete
  su ciclo (`etiquetapuesta = true`) y haya transcurrido el tiempo `delay_etiqueta_contra`
  configurable por el operario (rango 0–2000 ms).

- **FR-009**: El actuador DEBE retraerse solo después de que ambos motores completen
  sus ciclos (`etiquetapuesta = true` y `contrapuesta = true`) y haya transcurrido
  `tiempo_parada_actuador` (300 ms fijo) desde el último motor que terminó.

**Protección por timeout:**

- **FR-010**: Si el motor ETI lleva activo más de 10 segundos sin que FC1 complete
  el ciclo HOME→ETIQUETA→HOME, el sistema DEBE:
  a) Parar el motor ETI (GPIO26 → LOW)
  b) Retraer el actuador (GPIO16 → LOW)
  c) Reiniciar todas las variables de estado del ciclo activo
  d) Activar la bandera `error_fc1_timeout = true`
  e) Actualizar la LCD con mensaje de error descriptivo

- **FR-011**: Si el motor CON lleva activo más de 10 segundos sin que FC2 complete
  el ciclo, el sistema DEBE ejecutar la misma secuencia con `error_fc2_timeout = true`.

- **FR-012**: Mientras `error_fc1_timeout` o `error_fc2_timeout` estén activos,
  el sistema NO DEBE iniciar ningún nuevo ciclo de etiquetado aunque el sensor IR
  detecte una nueva botella.

- **FR-013**: La condición de error DEBE limpiarse únicamente cuando el operario
  pulse el botón AJUSTES (GPIO19). No debe limpiarse automáticamente.

**Seguridad de los pines de motor:**

- **FR-014**: El sistema DEBE garantizar que GPIO26 (motor ETI) y GPIO27 (motor CON)
  están a LOW en todo momento fuera de un ciclo activo (`detectada_botella = false`).
  Esta escritura es explícita en cada iteración del firmware para el caso de GPIO26
  (DAC2 que puede producir nivel analógico residual).

- **FR-015**: El sistema DEBE llamar a `dacDisable()` sobre GPIO26 en el arranque
  (`setup()`) antes de configurarlo como salida digital, para eliminar el riesgo
  de que el DAC produzca tensión que arranque el variador en la inicialización.

**Conteo y métricas:**

- **FR-016**: El sistema DEBE incrementar el contador de botellas etiquetadas
  (`botellas_etiquetadas`) solo cuando el ciclo complete correctamente (actuador
  retrae tras ambas etiquetas puestas). Un ciclo cancelado por timeout NO cuenta.

- **FR-017**: El sistema DEBE registrar el tiempo total del ciclo (desde detección
  IR hasta retracción actuador) en segundos con 2 decimales, y mantener la media
  de velocidad de producción (Bot/h) sobre las últimas 30 botellas completadas.

### Key Entities

- **Ciclo de etiquetado**: Unidad completa de trabajo. Inicia con `detectada_botella = true`
  y termina con el actuador retrayendo. Contiene como sub-pasos: detección IR,
  actuador, motor ETI (con FC1), motor CON (con FC2), retracción. Solo se cuenta
  como botella etiquetada si completa correctamente.

- **Estado FC (`fc_vio_etiqueta`)**: Variable booleana por sensor. `false` = aún no
  hemos visto la posición ETIQUETA en este ciclo (o ya estamos en HOME al inicio).
  `true` = hemos visto ETIQUETA y el motor puede parar al llegar a HOME. Se reinicia
  a `false` al final de cada ciclo.

- **Error de timeout**: Estado de fallo que bloquea la producción hasta intervención
  del operario. Distingue entre fallo de FC1 (motor ETI) y fallo de FC2 (motor CON).
  Persiste hasta confirmación manual.

- **Señal FC (fotocelda)**: Señal digital binaria de nivel industrial. Nivel HIGH = sin
  material (GAP / HOME). Nivel LOW = material reflectante detectado (etiqueta). No
  hay nivel analógico intermedio. Cualquier lectura que no sea estable durante ≥5 ms
  se trata como ruido y se ignora.

---

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: En el 100% de los ciclos donde el FC está en HOME (HIGH) al arrancar,
  el motor para exactamente en el siguiente GAP tras haber pasado la etiqueta completa.
  El rollo no debe quedar a mitad de etiqueta ni avanzar más de un GAP.

- **SC-002**: En el 100% de los ciclos donde el FC está sobre una etiqueta (LOW) al
  arrancar, el motor para en el primer GAP que encuentre, sin avanzar etiquetas adicionales
  ni desperdiciar material del rollo.

- **SC-003**: El sistema no debe parar ningún motor de forma prematura como consecuencia
  de ruido eléctrico en los cables de FC1/FC2, incluso durante la operación simultánea
  de ambos variadores de frecuencia. Tasa de falsos paros por ruido: 0% en operación
  continua de producción.

- **SC-004**: Un fallo de sensor (FC nunca cambia de estado) debe activar la protección
  por timeout en ≤10 segundos y dejar todos los motores y actuadores en estado seguro
  (LOW), sin requerir reinicio del sistema.

- **SC-005**: El contador de botellas etiquetadas refleja únicamente ciclos completados
  con éxito. Los ciclos cancelados por timeout no aparecen en el contador. La discrepancia
  entre botellas contadas y botellas físicamente etiquetadas debe ser 0.

- **SC-006**: La velocidad de producción (Bot/h) mostrada en pantalla es una media
  real sobre las últimas 30 botellas completadas. El valor mostrado no debe incluir
  ciclos fallidos ni tiempo de parada del operario como parte del cálculo.

- **SC-007**: Desde que la botella es detectada por el IR hasta que el actuador retrae
  (ciclo completo), el tiempo medido debe corresponder con el tiempo real observado
  físicamente. El T.Ciclo mostrado en LCD debe coincidir con el tiempo cronometrado
  manualmente con una tolerancia de ±0.1 segundos.
