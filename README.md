# lpc40xx_FreeRTOS_MP3_Project

A real-time **MP3 player system** built on FreeRTOS for the LPC4078F microcontroller. This embedded system integrates file system management, multitasking audio decoding, and interactive user control through a matrix keypad and OLED display—demonstrating complete software and hardware co-design.

To access the project source code, please click [here](https://github.com/CMPE146-MP3-PROJECT/FreeRTOS_MP3_Project/tree/ZhaoqinLi/projects/lpc40xx_freertos).

---

## 🧠 Features

- Multi-threaded **FreeRTOS** task design for audio decoding, UI display, and user control
- **VS1053 MP3 Decoder** integration via SPI and streaming via I2S
- **SD card (FAT32)** access for audio files using FatFS
- **OLED UI (SSD1306)** to show current playback status
- **Matrix Keypad** input for play/pause, track navigation, and volume adjustment
- Clean modular software architecture with separate responsibilities for each layer

---

## 📦 Software Info

- **RTOS:** FreeRTOS v8.2.0  
- **Language:** C17 (ISO/IEC 9899:2018)  
- **CPU:** LPC4078F (ARM Cortex-M4)  
- **Build System:** SCons v4.1.0  
- **File System:** FatFS with SPI SD card interface  
- **Key Libraries:**  
  - OLED Display Driver (I2C, SSD1306)
  - MP3 Decoder Driver (SPI, VS1053)
  - Keypad Matrix Scanning (GPIO)

---

## 🔧 Hardware Info

- **SoC Board:** SJTwo Board (SJSU Custom Dev Board)  
- **MP3 Decoder:** Adafruit VS1053 Codec v4  
  - [Datasheet here](https://cdn-shop.adafruit.com/datasheets/vs1053.pdf)  
- **Display:** 128×64 OLED using SSD1306  
- **Input:** 4×4 Matrix Keypad  
- **Audio Output:** 3.5mm headphone jack via VS1053 I2S

---

## 🗂️ Software Architecture

The system is broken into several independent FreeRTOS tasks:

- **MP3 Playback Task:** Streams decoded audio via VS1053
- **Display Task:** Shows current status (track name, progress)
- **User Input Task:** Scans matrix keypad for playback control
- **File Manager Task:** Parses SD card and tracks folder contents

All tasks interact via shared queues and semaphores for thread-safe data exchange.

![software_level_design](https://user-images.githubusercontent.com/60235970/118579435-f15c1380-b742-11eb-9877-57b4fff06f68.png)

---

## 🧰 Hardware Design Overview

![hw_level_design](https://user-images.githubusercontent.com/60235970/143958514-da04ff2e-c7f7-4b98-bc2d-e45016bf42fe.png)

---

## 🧪 System Assembly

Final prototype includes fully wired system with keypad input, SD-based MP3 loading, and real-time playback.

![mp3-assemble](https://user-images.githubusercontent.com/60235970/137086012-dfc3f29b-78d3-43da-a9a3-04417e4c72a5.png)

---

## 🚀 How to Build and Run

1. Clone the repository:
   ```bash
   git clone https://github.com/CMPE146-MP3-PROJECT/FreeRTOS_MP3_Project.git
   ```
2. Navigate to the project directory:
   ```bash
   cd projects/lpc40xx_freertos
   ```
3. Compile using SCons:
   ```bash
   scons
   ```
4. Flash to SJTwo board and connect peripherals (OLED, VS1053, keypad).
5. Insert SD card with MP3 files and enjoy playback.

---

## 👤 Authors

- **Zhaoqin Li** — Team Lead, System Architecture  
- Collaborators: SJSU CMPE146 Spring 2021 Team

---

## 📝 License

This project is released under the MIT License.
