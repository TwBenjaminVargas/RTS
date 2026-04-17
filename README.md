# RTS — Sistemas en Tiempo Real (IUA, 1° semestre 2026)

Este repositorio contiene la resolución de los prácticos de la materia **RTS**.  
Cada carpeta `P#` incluye:

- el archivo `assignment.pdf` con la consigna oficial,
- uno o más programas en C desarrollados para resolver el práctico.

Además, se incluyen los **trabajos prácticos de las clases teóricas** (`TP1`, `TP2`, `TP3`), con ejercicios y material complementario.

## Árbol del proyecto

```text
RTS/
├── README.md
├── TP1/
│   ├── assembly_float_to_short.c
│   ├── float_to_short.c
│   ├── fastfurier.m
│   ├── frecuenciamuestreo.m
│   ├── muestreo.m
│   ├── resolucion.m
│   ├── senal_datos.h
│   ├── signalsadded.m
│   └── output/
├── TP2/
│   ├── e2_propuesta_prioridad.txt
│   ├── ejecutivo_ciclico.c
│   ├── ejecutivo_ciclico_leds.c
│   └── test/
├── TP3/
│   ├── e3
│   ├── e3.c
│   ├── e4
│   ├── e4.c
│   ├── pthread resume.txt
│   └── class material/
├── P1/
│   ├── assignment.pdf
│   ├── simple_calc.c
│   ├── sort.c
│   ├── time.c
│   ├── traffic_lights.c
│   └── word_counter.c
├── P2/
│   ├── assignment.pdf
│   ├── blink_led_blocking_delay.c
│   ├── blink_led_kernel_timer.c
│   ├── blink_led_polling_timer.c
│   ├── interrupt_button_led.c
│   ├── polling_button_led.c
│   └── statistical_measurements.c
├── P3/
│   ├── assignment.pdf
│   └── sg90_state_machine.c
├── P4/
│   ├── assignment.pdf
│   └── sg90_barrier_control.c
├── P5/
│   ├── assignment.pdf
│   ├── fly_control_sistem.c
│   └── flycontrol_mutex.c
└── P6/
    ├── assignment.pdf
    └── climate_control.c
```

## Descripción de los prácticos

### TP1 — Muestreo, representación y análisis de señales
En este práctico se trabajó con conversión de datos, muestreo y procesamiento básico de señales.

- **`float_to_short.c` / `assembly_float_to_short.c`**: conversión de valores `float` a `short`.
- **Archivos `.m`**: scripts de MATLAB/Octave para muestreo, resolución, frecuencia de muestreo y análisis de señales.
- **`senal_datos.h`**: datos de entrada para las pruebas.
- **`output/`**: resultados generados por los ejercicios.

---

### TP2 — Planificación cíclica y prioridades
En este práctico se abordó la planificación de tareas periódicas y el uso de prioridades.

- **`ejecutivo_ciclico.c`**: implementación de un ejecutivo cíclico.
- **`ejecutivo_ciclico_leds.c`**: versión aplicada al control de LEDs.
- **`e2_propuesta_prioridad.txt`**: propuesta y análisis de asignación de prioridades.
- **`test/`**: pruebas y validaciones del práctico.

---

### TP3 — Hilos POSIX y sincronización
En este práctico se trabajó con creación de hilos y material de apoyo sobre `pthread`.

- **`e3.c` / `e4.c`**: ejercicios de concurrencia con hilos POSIX.
- **`pthread resume.txt`**: resumen teórico sobre `pthread`.
- **`class material/`**: ejercicios adicionales de clase.

---

### P1 — Fundamentos de C y concurrencia inicial
[Ver consigna](./P1/assignment.pdf)

Se implementaron programas base de lógica y manejo de argumentos:

- **`simple_calc.c`**: calculadora por CLI con validación de operandos.
- **`sort.c`**: parseo de cadena numérica y ordenamiento ascendente/descendente.
- **`word_counter.c`**: conteo de palabras desde string o archivo.
- **`time.c`**: reloj digital en consola con actualización en la misma línea.
- **`traffic_lights.c`**: simulación de semáforo con hilos, mutex y señal.

---

### P2 — GPIO, temporización e interrupciones con pigpio
[Ver consigna](./P2/assignment.pdf)

Se trabajó control de LED/botón y medición temporal en Raspberry Pi:

- **`blink_led_blocking_delay.c`**: parpadeo con demora bloqueante.
- **`blink_led_polling_timer.c`**: parpadeo no bloqueante con polling.
- **`blink_led_kernel_timer.c`**: parpadeo con timer POSIX.
- **`polling_button_led.c`**: lectura de botón por polling con debounce por software.
- **`interrupt_button_led.c`**: lectura por interrupción con debounce.
- **`statistical_measurements.c`**: medición de latencias y estadísticas.

---

### P3 — Máquina de estados + ISR para servo SG90
[Ver consigna](./P3/assignment.pdf)

- **`sg90_state_machine.c`**: control de servo SG90 con botón por interrupción.
- El botón incrementa la posición en pasos de 0° a 180°.
- Se usa anti-rebote por glitch filter y estado compartido atómico.

---

### P4 — Control de barrera con prioridades en tiempo real
[Ver consigna](./P4/assignment.pdf)

- **`sg90_barrier_control.c`**: sistema con dos hilos `SCHED_FIFO` de distinta prioridad.
- Incluye parada de emergencia, señalización con LED y reporte periódico.

---

### P5 — Planificación y análisis de tareas periódicas
[Ver consigna](./P5/assignment.pdf)

- **`fly_control_sistem.c`**: tareas de estabilidad, navegación y telemetría.
- **`flycontrol_mutex.c`**: versión con recurso compartido protegido por mutex.
- Se analizan prioridades, tiempos de ejecución y efecto de la sincronización.

---

### P6 — Control climático en tiempo real con sensor AHT10
[Ver consigna](./P6/assignment.pdf)

- **`climate_control.c`**: adquisición periódica de temperatura por I2C.
- Control lógico con máquina de estados.
- Activación de ventilación por GPIO con umbrales y tiempos de seguridad.
- Hilos con prioridades `SCHED_FIFO` para adquisición y control.

---

## Tecnologías usadas

- **Lenguaje:** C
- **Concurrencia:** POSIX Threads (`pthread`)
- **Tiempo real / scheduling:** `SCHED_FIFO`, `SCHED_OTHER`, timers POSIX, señales
- **Hardware:** Raspberry Pi 3 + `pigpio`
- **Periféricos:** botón, LED, servo SG90, sensor AHT10 (I2C)