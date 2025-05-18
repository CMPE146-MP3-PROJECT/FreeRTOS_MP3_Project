# Software Architecture Document  
**Project:** lpc40xx_freertos  
**Path:** lpc40xx_freertos  

---

## 1. Overview  
The `lpc40xx_freertos` project is a FreeRTOS-based firmware for the NXP LPC40xx microcontroller. It follows a strict layered architecture (L0–L5) to separate concerns and enable testability.  

### 1.1 Goals  
- Provide a minimal bootloader and hardware bring-up.  
- Integrate the FreeRTOS kernel with Cortex-M4 port.  
- Supply reusable utilities, drivers, and I/O modules.  
- Host application code (e.g., MP3 playback) in isolated tasks.  
- Support unit testing via header overrides.  

---

## 2. Folder Structure  
```
lpc40xx_freertos/
├─ entry_point.c
├─ interrupt_vector_table.c
├─ lpc40xx.h
├─ FreeRTOSConfig.h
├─ SConscript
├─ subsidiary_scons/
├─ README.md
├─ l0_lowlevel/
├─ l1_freertos/
├─ l2_utils/
├─ l3_drivers/
├─ l4_io/
└─ l5_application/
```  

- **entry_point.c** – Startup code (clock, peripherals, crash report)  
- **interrupt_vector_table.c** – Cortex-M interrupt vector definitions  
- **lpc40xx.h** – MCU register and IRQn definitions  
- **FreeRTOSConfig.h** – Kernel configuration for this project  
- **`subsidiary_scons/`** – Build fragments for submodules  
- **Layer directories** (L0–L5) described below  

---

## 3. Layer Descriptions  

### 3.1 L0 – Low-Level (Boot & Crash)  
Path: `l0_lowlevel/`  
- **crash.c** / **`crash.h`** – Persistent RAM crash recorder and printer on startup  
- **`vector table`** – Defined in interrupt_vector_table.c, maps reset, fault and peripheral ISRs  
- **entry_point.c** –  
  - Calls `peripherals_init()`  
  - Prints any previous crash via `crash__report_if_occurred()`  
  - Initializes RTOS trace and jumps to `main()`  

### 3.2 L1 – FreeRTOS Kernel  
Path: `l1_freertos/`  
- Upstream FreeRTOS source (portable layer, queue, tasks, timers, stream/message buffers)  
- **Port files**:  
  - port.c – Cortex-M4 port including `vPortValidateInterruptPriority()`  
  - port.c – x86 simulator for host testing  
- **Kernel hooks**: freertos_interrupt_handlers.c wraps SVC/PendSV/SysTick  
- **Trace**: Percepio Tracealyzer port in `trace/` with `trcKernelPort.*`  
- **Unit testing**: Header overrides in `unittest_header_overrides/`  

### 3.3 L2 – Utilities  
Path: `l2_utils/`  
- Standalone libraries (e.g., ring buffers, string helpers) with no RTOS dependency  
- Can be used by drivers or higher layers  

### 3.4 L3 – Drivers (Hardware Abstraction Layer)  
Path: `l3_drivers/`  
Each peripheral driver lives in its own subfolder (`gpio/`, `uart/`, `spi/`, `i2c/`, `ssp/`, `dma/`, `timer/`, `pwm/`).  

#### 3.4.1 Module Structure & Init  
- Driver exposes `xxx_driver.h` with `xxx_init()` + APIs; implements register-level access in `xxx_driver.c`.  
- Common init sequence:  
  1. `lpc_peripheral__enable_power()`  
  2. `lpc_peripheral__set_clock()`  
  3. `pinmux_set_function()`  
  4. Configure NVIC (priority ≤ `configMAX_SYSCALL_INTERRUPT_PRIORITY`)  

#### 3.4.2 Driver APIs  
- **GPIO**: `gpio__init(pin, dir)`, `gpio__set()`, `gpio__get()`, `gpio__attach_interrupt()` (debounce via L2 ring buffer)  
- **UART**: `uart__init(port, baud)`, `uart__write()`, `uart__read()` (ISR‐driven RX with FreeRTOS mutex)  
- **SPI/SSP**: `spi__init()`, `spi__transfer()`; DMA support via stream buffers  
- **I2C**: Blocking `i2c__write_read()` with timeout flags  
- **DMA**: `dma__init()`, `dma__request_channel()`, `dma__start()`; ISR callbacks use `xTaskNotifyFromISR()`  
- **Timer/PWM**: `timer__init()/start()/stop()`, `pwm__init()/set_duty()`  

#### 3.4.3 Interactions & Dependencies  
- Uses L2 utilities (ring buffers, CRC)  
- FreeRTOS semaphores/mutexes for thread safety  
- Stream buffers for DMA data streams  
- Event groups for multi-event sync  

### 3.5 L4 – I/O Services  
Path: `l4_io/`  
- Middleware for CLI, file system (MP3 file reader)  
- Abstracts driver interfaces into higher-level APIs  

### 3.6 L5 – Application (User‐Level Tasks)  
Path: `l5_application/`  

#### 3.6.1 Task Definitions  
- **sd_card_task.c**  
  - Mounts FAT FS, monitors media-detect GPIO  
  - Enqueues file handles to `xQueueSdFiles`  
- **mp3_decoder_task.c**  
  - Dequeues handles, parses MP3 frames (L4), decodes (L2/libMAD)  
  - Streams PCM blocks to `xStreamBufPcm`  
- **audio_output_task.c**  
  - Consumes `xStreamBufPcm`, submits PCM to `ssp` via DMA  
  - Signals underrun/overflow via `egAudioStatus`  
- **cli_task.c**  
  - Shell over UART (`ls`, `play`, `stop`, `volume`)  
  - Sends commands via `xQueueCliCommands`  
- **system_monitor_task.c**  
  - Periodically logs heap/stack/CPU load to CLI/display  

#### 3.6.2 Shared RTOS Objects  
- Queues: `xQueueSdFiles`, `xQueueCliCommands`  
- Stream Buffers: `xStreamBufPcm`  
- Event Groups: `egAudioStatus` (underrun/overflow flags)  
- Mutexes: e.g., protect SD‐card SPI  

#### 3.6.3 Task Configuration & Priorities  
Defined in `main()` before `vTaskStartScheduler()`:  

| Task                   | Priority | Stack (words) |
|------------------------|----------|---------------|
| system_monitor_task    | 1        | 256           |
| cli_task               | 2        | 384           |
| sd_card_task           | 3        | 512           |
| mp3_decoder_task       | 4        | 1024          |
| audio_output_task      | 5        | 512           |

#### 3.6.4 Fault Handling & Recovery  
- `configASSERT()` for API misuse  
- Critical errors set a watchdog flag; on reset, `crash__report_if_occurred()` prints last faulting task & SP  

#### 3.6.5 Extension Guidelines  
- **New Task:** add `.c/.h` under `l5_application/`, create RTOS objects, register with `xTaskCreate()` in `main()`  
- **New CLI Command:** extend `cli_task.c` parser, update `control_command_t` and target task handling  

---

## 4. Initialization & Execution Flow  
1. **Reset Handler** ([interrupt_vector_table.c] vector index 1)  
2. **`entry_point()`** (entry_point.c)  
   - `peripherals_init()` (clocks, pinmux)  
   - `entry_point__handle_crash_report()`  
   - `entry_point__rtos_trace_init_after_mounting_sd_card()`  
   - `main()` in L5 begins  
3. **`main()`**  
   - Creates RTOS objects (tasks, queues, event groups)  
   - Starts scheduler via `vTaskStartScheduler()`  
4. **FreeRTOS Scheduler**  
   - PendSV, SysTick drive context switching  
   - Application tasks run in user-defined priorities  

---

## 5. Build System  
- **SCons scripts**:  
  - Top‐level `SConscript` in project root  
  - `lpc40xx_freertos/SConscript` includes source, include paths  
  - `subsidiary_scons/` contains common definitions (compiler flags, linker scripts)  
- **Build commands** (in project root):  
  ```bash
  scons --project=lpc40xx_freertos         # Full build
  scons --project=lpc40xx_freertos --no-unit-test
  scons --project=lpc40xx_freertos --no-float-format
  ```  
- **Unit tests** compile with header overrides in `unittest_header_overrides/`  

---

## 6. Configuration  
- **FreeRTOSConfig.h**: Task priorities, hooks, trace, run‐time stats  
- **`configASSERT_callback`** reports assertion line and message  
- **`configMAX_SYSCALL_INTERRUPT_PRIORITY`** set to enforce ISR API safety  

---

## 7. Key Components & Links  
- Startup & crash:  
  - entry_point.c  
  - crash.c  
- ISR vector:  
  - interrupt_vector_table.c  
- FreeRTOS port:  
  - portable/gcc/arm_cm4f/port.c  
- Trace integration:  
  - trcKernelPort.h  
- Application entry:  
  - `main()` in [l5_application/]  

---

## 8. Extension & Testing  
- To add a new driver, place code in `l3_drivers/`, use L2 utilities & L1 RTOS services.  
- To add a task, implement in `l5_application/`, register in `main()` before `vTaskStartScheduler()`.  
- For unit tests, add tests under `l1_freertos/unittest_header_overrides/` and adjust SCons to include `unity`.  

---

*This document should serve as a reference for developers navigating and extending the `lpc40xx_freertos` project.*