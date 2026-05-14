# Embedded Media Centre

A bare-metal embedded multimedia system built on the **NXP LPC1768 ARM Cortex-M3** microcontroller using the **Keil MCB1700** evaluation board. The project integrates graphical rendering, USB audio streaming, real-time joystick interaction, and embedded game logic into a compact multimedia application — all running without an operating system.

---

## Screenshots

| Main Menu | Photo Gallery | MP3 Player |
|:---------:|:-------------:|:----------:|
| ![Main Menu](result/Main%20Menu.png) | ![Photo Gallery](result/Photo%20Gallery.png) | ![MP3 Player](result/MP3%20Player.png) |

| Snake | Pong |
|:-----:|:----:|
| ![Snake](result/Snake%20Game.png) | ![Pong](result/Pong%20Game.png) |

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware](#hardware)
- [Architecture](#architecture)
- [Control Logic](#control-logic)
- [Modules](#modules)
- [Image Conversion Workflow](#image-conversion-workflow)
- [Project Structure](#project-structure)
- [Testing](#testing)
- [Key Technical Concepts](#key-technical-concepts)
- [Future Improvements](#future-improvements)

---

## Overview

The Embedded Media Centre is a standalone application that runs directly on the LPC1768 microcontroller. It provides a graphical menu interface for navigating between three main modules: a Photo Gallery, an MP3 Player, and a Game Module.

The application uses a modular, event-driven architecture to manage transitions between modules while sharing hardware peripherals — the GLCD, joystick, DAC, ADC, Timer0, and USB interface. There is no OS, scheduler, or dynamic memory allocation; all logic runs in a cooperative super-loop with interrupt-driven audio.

---

## Features

- Joystick-navigated menu system with instant, flicker-free mode switching
- Full-colour photo gallery — images stored as RGB565 C arrays in flash
- USB Audio Device — streams audio from a host PC to the on-board DAC in real time
- Snake game with joystick control, food generation, growth, and collision detection
- Pong game with CPU-controlled opponent and win/lose detection
- Potentiometer-controlled volume via ADC
- No RTOS — fully cooperative bare-metal super-loop

---

## Hardware

| Component | Detail |
|-----------|--------|
| Microcontroller | NXP LPC1768 — ARM Cortex-M3 @ 100 MHz |
| Board | Keil MCB1700 |
| Display | GLCD 240×320 via SPI |
| Audio output | On-board DAC driven by Timer0 IRQ @ 32 kHz |
| Audio input | USB Audio Device Class — Windows generic driver |
| User input | On-board 5-way joystick |
| Volume control | On-board potentiometer via ADC |
| Timer | Timer0 — audio sample timing |
| Development environment | Keil uVision 5 |

---

## Architecture

The firmware is structured as a **bare-metal cooperative super-loop**. There is no scheduler — all mode logic runs sequentially in `main()`, with audio handled entirely through hardware interrupts.

```
main()
 ├─ Hardware init: ADC, GLCD, Joystick
 └─ while(1)
     ├─ Read joystick state
     ├─ Mode changed?
     │   └─ GLCD_Clear() → Init*()   ← runs once on entry
     └─ Run*()                        ← runs every iteration
```

### Mode Switching

A single global `AppMode currentMode` variable controls which subsystem is active. When `currentMode` changes, the loop detects the transition, clears the screen, and calls the incoming module's `Init*()` function exactly once. The `Run*()` function then handles all ongoing logic — joystick polling, state updates, and display redraws — on every tick. This design limits screen flicker and keeps transitions clean.

```c
if (currentMode != previousMode) {
    GLCD_Clear(White);
    Init*();                 // one-time setup
    previousMode = currentMode;
}
Run*(stickDir, &prevStickDir, &currentMode);  // continuous update
```

### USB Audio (MP3 Player)

The MP3 Player is interrupt-driven rather than polled. Two hardware interrupt handlers work as a producer/consumer pair:

- **USB IRQ** — fires when a new isochronous packet arrives from the host PC and writes incoming PCM data into a circular DMA buffer in USB RAM.
- **Timer0 IRQ** — fires at **32 kHz** (one tick per audio sample). On each tick it reads the next sample from the buffer, applies potentiometer volume scaling, and writes the result to the DAC.

A scatter file (`DMA.sct`) explicitly places the DMA buffer in the dedicated USB RAM region (`0x2007C000`) required by the LPC1768 USB peripheral. When the user exits the MP3 Player, the system cleanly disables both IRQs, stops audio output, and disconnects USB before returning to the main menu.

### Display

All rendering goes through `GLCD_SPI_LPC1700.c`, which communicates with the LCD controller over SPI. Text is drawn using two embedded bitmap fonts (6×8 and 16×24 pixels). Images are stored as **RGB565 C arrays** compiled directly into flash and rendered with a single `GLCD_Bitmap()` burst call.

### Games

Both games run self-contained internal `while` loops rather than returning to `main()` on each tick:

- **Snake** — moves on a 20×10 character grid. The body is stored as an array of `{x, y}` structs. Each tick the tail cell is erased, all segments shift forward, and the head advances in the last joystick direction. Collision with walls or self ends the loop.
- **Pong** — tracks ball position as `double {x, y}` with `{dx, dy}` velocity. The CPU paddle follows the ball's y-position with a randomness factor to keep gameplay competitive. First to 3 points wins.

---

## Control Logic

Joystick input uses **edge detection** — the current state is compared against the previous state so actions fire only when the direction *changes*, not while it is held. This prevents repeated triggers.

The joystick is used to:

- Move through menu options (up/down)
- Confirm selections (centre)
- Navigate between images in the Photo Gallery (left/right)
- Control Snake movement (all four directions)
- Control the Pong player paddle (up/down)
- Return to the previous menu or the Main Menu (centre)

All menus use **circular navigation** — moving past the first or last option wraps around without interruption.

---

## Modules

### Main Menu

The central navigation hub. Displays three options — Photo Gallery, MP3 Player, and Games — with the active selection highlighted on the GLCD. The menu updates only when a new joystick movement is detected.

### Photo Gallery

Displays full-screen images stored as RGB565 C arrays in flash. Images are referenced through a pointer array, allowing the system to cycle through them without dynamic memory allocation. The screen redraws only when the image index changes, minimising flicker.

- Joystick right → next image
- Joystick left → previous image
- Joystick centre → return to Main Menu

### MP3 Player

Enumerates as a **USB Audio Device** to the host OS — no custom driver required. Windows, macOS, and Linux all recognise it as a standard audio output. PCM audio is streamed over USB isochronous transfers, buffered in USB RAM, and output through the DAC at 32 kHz. The potentiometer controls the final output volume. Pressing centre cleanly shuts down USB and returns to the menu.

### Game Module

A secondary menu where the user selects Snake, Pong, or returns to the Main Menu. Each game is an independent submodule with its own menu, gameplay loop, input handling, score tracking, and exit logic.

#### Snake

- Joystick-controlled movement in four directions
- Continuous movement based on last known direction
- Random food generation within bounds
- Snake grows by one segment on food collection
- Score increments on each food collected
- Wall and self-collision detection end the game
- Game-over screen with final score

#### Pong

- Joystick controls the left (player) paddle
- CPU controls the right paddle and tracks the ball with randomised lag
- Ball resets to centre after each point
- First to 3 points wins
- Win/lose result screen before returning to the game menu

---

## Image Conversion Workflow

The Photo Gallery images are stored as RGB565 C arrays compiled directly into flash.

1. Select a source image
2. Resize or crop to match the GLCD resolution (240×320)
3. Convert to RGB565 format
4. Export as a C array
5. Place the generated `.c` file in `firmware/images/`
6. Add a pointer to the image in the `gallery[]` array in `PhotoGallery.c`

---

## Project Structure

```
.
├── firmware/                       Keil uVision 5 project
│   ├── usbaudio.uvprojx            Project file
│   ├── DMA.sct                     Linker scatter file — places USB DMA buffer in USB RAM
│   ├── FLASH.ini                   Flash programming configuration
│   ├── DebugConfig/                ULINK debug probe configuration
│   ├── RTE/                        Keil Run-Time Environment (device pack managed files)
│   │
│   ├── src/                        Application logic
│   │   ├── main.c                  Entry point — hardware init and super-loop
│   │   ├── MainMenu.c / .h         Main menu UI and navigation
│   │   ├── PhotoGallery.c / .h     Photo viewer — image loading and navigation
│   │   ├── MP3Player.c / .h        USB audio player — IRQ handlers and DAC output
│   │   ├── GameMenu.c / .h         Game selection menu
│   │   └── ModeEnum.h              AppMode enum shared across all modules
│   │
│   ├── games/                      Game implementations
│   │   ├── snake.c / .h            Snake — grid logic, collision, scoring
│   │   └── pong.c / .h             Pong — ball physics, CPU AI, scoring
│   │
│   ├── display/                    LCD driver and font data
│   │   ├── GLCD_SPI_LPC1700.c      Low-level SPI LCD driver
│   │   ├── GLCD.h                  Public LCD API (init, clear, draw, text)
│   │   ├── Font_6x8_h.h            6×8 pixel bitmap font
│   │   └── Font_16x24_h.h          16×24 pixel bitmap font
│   │
│   ├── images/                     Image data compiled into flash
│   │   ├── barcelona.c             RGB565 pixel array — FC Barcelona logo
│   │   ├── messi.c                 RGB565 pixel array — Messi photo
│   │   └── worldcup.c              RGB565 pixel array — World Cup photo
│   │
│   ├── usb/                        USB Audio Class stack
│   │   ├── usbhw.c / .h            USB hardware layer (LPC1768 USB peripheral)
│   │   ├── usbcore.c / .h          USB protocol core (enumeration, control transfers)
│   │   ├── usbdesc.c / .h          USB descriptors (Audio Class, isochronous endpoint)
│   │   ├── usbuser.c / .h          USB event callbacks
│   │   ├── usbdmain.c              USB DMA interrupt handler
│   │   ├── usbaudio.h              Audio class constants and buffer sizing
│   │   ├── usbcfg.h                USB stack configuration
│   │   ├── usbreg.h                USB peripheral register definitions
│   │   └── usb.h                   USB standard descriptor and request types
│   │
│   ├── drivers/                    Board peripheral drivers
│   │   ├── adcuser.c / .h          USB Audio Device volume/mute control callbacks
│   │   └── audio.h                 USB Audio class type definitions
│   │
│   └── device/                     CMSIS and NXP device support
│       ├── core_cm3.c / .h         CMSIS Cortex-M3 core peripheral access
│       ├── LPC17xx.h               LPC17xx peripheral register map
│       ├── system_LPC17xx.c / .h   PLL and system clock initialisation
│       ├── startup_LPC17xx.s       Reset handler, stack setup, vector table
│       └── type.h                  Primitive type aliases
│
├── docs/                           Documentation and diagrams
│   ├── Project Flow Chart.png
│   └── architecture.drawio
│
├── assets/                         Source images (JPEG originals for image conversion)
│   ├── barcelona.jpg
│   ├── messi.jpg
│   └── worldcup.jpg
│
└── result/                         Hardware demo photos
```

---

## Testing

The full system was deployed on the LPC1768 board and tested across all modules.

| Area | Result |
|------|--------|
| Main Menu | Smooth joystick navigation and stable mode transitions |
| Photo Gallery | Images displayed correctly with minimal flicker |
| MP3 Player | Audio streamed over USB with responsive volume control |
| Snake | Movement, scoring, food generation, and collision handling all correct |
| Pong | Paddle movement, CPU behaviour, collision, scoring, and reset all correct |
| Mode transitions | All modules returned cleanly to the menu without conflicts |

---

## Key Technical Concepts

- Bare-metal embedded programming (no OS)
- Modular firmware architecture
- Event-driven control flow with edge-detected joystick input
- GLCD graphics rendering over SPI
- RGB565 image storage and display
- USB Audio Device Class integration
- Interrupt-driven DAC audio output at 32 kHz
- Circular DMA buffer for USB audio streaming
- Timer0 interrupt for sample-accurate audio playback
- ADC-based potentiometer volume control
- Real-time game logic (Snake, Pong)

---

## Future Improvements

- Refine Pong ball physics and CPU paddle behaviour
- Optimise screen redraw performance to reduce flicker further
- Expand the Photo Gallery with additional images
- Add more games or interactive modules
- Improve USB audio error handling for unexpected disconnects
