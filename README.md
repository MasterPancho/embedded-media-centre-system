# LPC1768 Embedded Media Centre System

A bare-metal embedded multimedia system built on the **NXP LPC1768** (ARM Cortex-M3) microcontroller for the **Keil MCB1700** evaluation board. Developed for COE718 — Embedded Systems at Toronto Metropolitan University (Fall 2024).

---

## Screenshots

| Main Menu | Photo Gallery | MP3 Player |
|:---------:|:-------------:|:----------:|
| ![Main Menu](result/Main%20Menu.png) | ![Photo Gallery](result/Photo%20Gallery.png) | ![MP3 Player](result/MP3%20Player.png) |

| Snake | Pong |
|:-----:|:----:|
| ![Snake](result/Snake%20Game.png) | ![Pong](result/Pong%20Game.png) |

---

## Features

| Mode | Description |
|------|-------------|
| Main Menu | Joystick-navigated top-level interface |
| Photo Gallery | Displays full-color images on the GLCD |
| MP3 Player | USB Audio Device — streams audio from a host PC |
| Snake | Classic snake game |
| Pong | Two-player pong game |

---

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | NXP LPC1768 — ARM Cortex-M3 @ 100 MHz |
| Board | Keil MCB1700 |
| Display | GLCD via SPI (Nokia 6610 compatible) |
| Audio | USB Audio Device (Windows generic driver) |
| Input | On-board 5-way joystick + potentiometer (ADC) |

---

## Project Structure

```
.
├── firmware/                       Keil uVision 5 project (open usbaudio.uvprojx)
│   ├── usbaudio.uvprojx            Project definition
│   ├── DMA.sct                     Linker scatter file — allocates USB RAM for DMA
│   ├── FLASH.ini                   Flash programming script
│   ├── Abstract.txt                Project description
│   ├── DebugConfig/                ULINK debug configuration
│   ├── RTE/                        Keil Run-Time Environment (device pack files)
│   │
│   ├── src/                        Application source
│   │   ├── main.c                  Entry point and mode dispatcher
│   │   ├── MainMenu.c / .h         Top-level navigation menu
│   │   ├── PhotoGallery.c / .h     Photo viewer
│   │   ├── MP3Player.c / .h        USB audio player
│   │   ├── GameMenu.c / .h         Game selection menu
│   │   └── ModeEnum.h              App mode enum (MENU, PHOTO, MP3, GAME)
│   │
│   ├── games/                      Game implementations
│   │   ├── snake.c / .h            Snake game
│   │   └── pong.c / .h             Pong game
│   │
│   ├── display/                    LCD driver and fonts
│   │   ├── GLCD_SPI_LPC1700.c      SPI LCD driver
│   │   ├── GLCD.h                  LCD interface
│   │   ├── Font_6x8_h.h            Small font bitmap
│   │   └── Font_16x24_h.h          Large font bitmap
│   │
│   ├── images/                     Image pixel data (RGB565 C arrays)
│   │   ├── barcelona.c             FC Barcelona logo
│   │   ├── messi.c                 Messi photo
│   │   └── worldcup.c              World Cup photo
│   │
│   ├── usb/                        USB Audio stack
│   │   ├── usbcore.c / .h          USB protocol core
│   │   ├── usbdesc.c / .h          USB descriptors (Audio Class)
│   │   ├── usbdmain.c              USB DMA handler
│   │   ├── usbhw.c / .h            USB hardware abstraction
│   │   ├── usbuser.c / .h          USB user callbacks
│   │   ├── usbaudio.h              USB Audio class definitions
│   │   ├── usbcfg.h                USB configuration
│   │   └── usbreg.h                USB register map
│   │
│   ├── drivers/                    Board-level drivers
│   │   ├── adcuser.c / .h          ADC / potentiometer driver
│   │   └── audio.h                 Audio type definitions
│   │
│   └── device/                     CMSIS + NXP device files
│       ├── core_cm3.c / .h         CMSIS Cortex-M3 core
│       ├── LPC17xx.h               LPC17xx peripheral register map
│       ├── system_LPC17xx.c / .h   System clock initialization
│       ├── startup_LPC17xx.s       Reset handler and vector table
│       └── type.h                  Primitive type definitions
│
├── docs/                           Project documentation
│   ├── Project Interim Report.pdf
│   ├── Project Summary.pdf
│   ├── Project Flow Chart.png
│   └── Coe718 Project.drawio
│
└── assets/                         Source images (used to generate images/ pixel data)
    ├── barcelona.jpg
    ├── messi.jpg
    └── worldcup.jpg
```

---

## How It Works

`main.c` runs a bare-metal super-loop. A global `currentMode` variable drives which subsystem is active. On every mode change, the screen is cleared and the new mode's `Init*()` function runs exactly once. The corresponding `Run*()` function is then called every loop iteration to handle joystick input and update the display.

```
main()
 └─ loop
     ├─ mode changed? → GLCD_Clear() → Init*()
     └─ always        → Run*()
```

The USB MP3 Player is special: it runs entirely from `InitMP3Player()` via USB interrupt handlers and does not need a `Run*()` poll. The potentiometer (ADC) controls playback volume.

The image files (`barcelona.c`, `messi.c`, `worldcup.c`) are C arrays of 16-bit RGB565 pixel data generated from the source JPEGs in `assets/`.

A scatter file (`DMA.sct`) is required because the USB peripheral uses DMA and its RAM region must be explicitly placed — do not remove it from the Keil project settings.

---

## Building

1. Install **Keil MDK-ARM v5** and the **Keil LPC1700_DFP** device pack (v2.6.0+)
2. Open `firmware/usbaudio.uvprojx` in Keil uVision
3. Select the **Flash** target
4. **Build** → **Download** to flash the LPC1768

> The `Obj/` and `Lst/` build output folders are excluded from this repository via `.gitignore`. They are regenerated automatically on first build.

---

## USB Audio Setup

1. Connect the MCB1700 USB port to a Windows PC
2. Windows will detect a generic USB Audio device and add a new speaker
3. Set that speaker as the audio output in Windows Sound settings
4. Audio played on the PC is streamed to the board over USB and output via the on-board DAC
5. Use the potentiometer to adjust volume
