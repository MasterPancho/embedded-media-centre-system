# Embedded Media Centre

A bare-metal multimedia system for the **NXP LPC1768** (ARM Cortex-M3) microcontroller. Built entirely without an RTOS, it runs a joystick-driven interface with four independent modes: a photo gallery, a USB audio player, a snake game, and a pong game — all displayed on an SPI-driven GLCD.

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

- [Features](#features)
- [Hardware](#hardware)
- [Architecture](#architecture)
- [Modes](#modes)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [USB Audio Setup](#usb-audio-setup)

---

## Features

- Joystick-navigated menu system with instant mode switching
- Full-colour photo gallery with left/right navigation
- USB Audio Device — streams PC audio to the on-board DAC in real time
- Snake game with collision detection, growing body, and score tracking
- Pong game with CPU-controlled opponent and win/lose detection
- Potentiometer-controlled volume via ADC
- No RTOS — fully cooperative bare-metal super-loop

---

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | NXP LPC1768 — ARM Cortex-M3 @ 100 MHz |
| Board | Keil MCB1700 |
| Display | GLCD 240×320 via SPI (Nokia 6610 compatible) |
| Audio output | On-board DAC driven by Timer0 IRQ @ 32 kHz |
| Audio input | USB Audio Device Class (Windows generic driver) |
| User input | On-board 5-way joystick |
| Volume control | Potentiometer read via ADC |

---

## Architecture

The firmware is structured as a **bare-metal cooperative super-loop**. There is no scheduler or RTOS — all mode logic runs sequentially in `main()`.

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

A single global `AppMode currentMode` variable controls which subsystem is active. When `currentMode` changes, the loop detects the transition, clears the screen, and calls the incoming mode's `Init*()` function exactly once to set up its UI. The `Run*()` function then handles all ongoing logic — joystick polling, state updates, and display redraws — on every loop tick.

```c
// Simplified from main.c
if (currentMode != previousMode) {
    GLCD_Clear(White);
    Init*();                 // one-time setup
    previousMode = currentMode;
}
Run*(stickDir, &prevStickDir, &currentMode);  // continuous update
```

Joystick input uses **edge detection** — actions only fire when the direction *changes*, not while it is held. This prevents unintended repeated inputs.

### USB Audio (MP3 Player)

The MP3 Player mode is interrupt-driven rather than polled. When selected, `RunMP3Player()` configures the hardware and hands control to two interrupt handlers:

- **Timer0 IRQ** fires at **32 kHz** (one tick per audio sample). On each tick it reads the next sample from a circular DMA buffer, applies the potentiometer volume, and writes the result to the DAC.
- **USB IRQ** fires when a new USB isochronous packet arrives from the host PC. It writes the incoming PCM audio data into the DMA buffer.

The two handlers work as a producer/consumer pair — USB fills the buffer, Timer0 drains it to the DAC. A scatter file (`DMA.sct`) explicitly places the USB DMA buffer in the dedicated USB RAM region (`0x2007C000`) required by the LPC1768 hardware.

Pressing the joystick centre while in MP3 mode disables both IRQs, disconnects USB, and returns to the main menu.

### Display

All rendering goes through the GLCD driver (`GLCD_SPI_LPC1700.c`), which communicates with the LCD controller over SPI. Text is drawn using two embedded bitmap fonts (6×8 and 16×24 pixels). Images are stored as **RGB565 C arrays** (`images/*.c`) compiled directly into flash and blasted to the screen with a single `GLCD_Bitmap()` call.

### Games

Both games run their own internal loops (blocking `while` loops) rather than returning to `main()`. This keeps their logic self-contained:

- **Snake** — moves on a 20×10 character grid. The snake body is stored as an array of `{x, y}` structs. Each tick the tail cell is erased, all segments shift forward, and the head advances in the last joystick direction. Collision with walls or self sets `isAlive = false` and ends the loop.
- **Pong** — tracks ball position as `double {x, y}` with `{dx, dy}` velocity. The CPU paddle follows the ball's y-position with randomised lag to make it beatable. Scoring resets the ball; first to 3 points wins.

---

## Modes

### Main Menu
The entry point after power-on. Displays three options (Photo Gallery, MP3 Player, Games). Joystick up/down moves the highlighted selection; centre confirms. The selected option sets `currentMode` and the super-loop handles the transition.

### Photo Gallery
Loads one of three images stored as RGB565 pixel arrays in flash. Joystick left/right cycles through the gallery; centre returns to the main menu. Images are rendered instantly via DMA-backed SPI burst transfer.

### MP3 Player
Enumerates as a **USB Audio Device** to the host OS — no custom driver required. Windows, macOS, and Linux all recognise it as a standard audio output. PCM audio is streamed over USB isochronous transfers, buffered in USB RAM, and output through the DAC at 32 kHz. The potentiometer sets the final output volume. Press centre to disconnect and return to the menu.

### Snake
Classic snake on a bordered grid. The snake grows by one segment each time it eats food, which respawns at a random position. The game ends on wall or self-collision and shows a game-over screen with the final score before returning to the game menu.

### Pong
Single-player vs CPU pong. The player controls the left paddle with up/down joystick. The CPU paddle tracks the ball's position but misses occasionally. First to 3 points wins. After the game, the result screen is shown briefly before returning to the game menu.

---

## Project Structure

```
.
├── firmware/                       Keil uVision 5 project
│   ├── usbaudio.uvprojx            Project file (open this in Keil)
│   ├── DMA.sct                     Linker scatter file — places USB DMA buffer in USB RAM
│   ├── FLASH.ini                   Flash programming configuration
│   ├── DebugConfig/                ULINK2 debug probe configuration
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
│   │   ├── usbuser.c / .h          USB event callbacks (SOF, EP1 out)
│   │   ├── usbdmain.c              USB DMA interrupt handler
│   │   ├── usbaudio.h              Audio class constants and buffer sizing
│   │   ├── usbcfg.h                USB stack configuration (DMA, endpoint count)
│   │   ├── usbreg.h                USB peripheral register definitions
│   │   └── usb.h                   USB standard descriptor and request types
│   │
│   ├── drivers/                    Board peripheral drivers
│   │   ├── adcuser.c / .h          USB Audio Device volume/mute control callbacks
│   │   └── audio.h                 USB Audio class constants (mute, volume controls)
│   │
│   └── device/                     CMSIS and NXP device support
│       ├── core_cm3.c / .h         CMSIS Cortex-M3 core peripheral access
│       ├── LPC17xx.h               LPC17xx peripheral register map
│       ├── system_LPC17xx.c / .h   PLL and system clock initialisation
│       ├── startup_LPC17xx.s       Reset handler, stack setup, vector table
│       └── type.h                  Primitive type aliases (uint8_t, uint32_t, etc.)
│
├── docs/                           Documentation and diagrams
│   ├── Project Flow Chart.png      System flow chart
│   └── architecture.drawio         Architecture diagram (draw.io)
│
├── assets/                         Source images (JPEG originals)
│   ├── barcelona.jpg
│   ├── messi.jpg
│   └── worldcup.jpg
│
└── result/                         Photos of the system running on hardware
```

---

## Getting Started

### Prerequisites

- [Keil MDK-ARM v5](https://www.keil.com/download/product/) (free for LPC1768 with size limit)
- Keil **LPC1700_DFP** device pack v2.6.0+ (install via Pack Installer in Keil)
- Keil MCB1700 evaluation board
- ULINK2 or compatible debug probe (for flashing)

### Build

1. Open `firmware/usbaudio.uvprojx` in Keil uVision 5
2. Select the **Flash** target from the target dropdown
3. Click **Build** (F7)

> `Obj/` and `Lst/` are excluded via `.gitignore` and are regenerated on first build.

### Flash

1. Connect the ULINK2 probe to the MCB1700 JTAG header
2. Power the board
3. Click **Download** (F8) in Keil uVision

The board boots directly into the main menu.

---

## USB Audio Setup

1. Connect the MCB1700 **USB** port (not the JTAG port) to a Windows PC with a USB-A cable
2. Windows automatically installs a generic USB Audio driver — no custom driver needed
3. Open **Sound Settings** → set the new speaker as the default output device
4. Play audio on the PC — it streams over USB to the board and outputs through the DAC
5. Rotate the potentiometer to adjust volume
6. Press the joystick **centre** button to disconnect USB and return to the main menu
