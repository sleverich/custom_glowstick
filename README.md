# custom_glowstick
Custom glowstick firmware based on the ATTiny85

This firmware is for a glowstick using an ATTiny85 as the microcontroller. Usually, I replace the unknown PIC microcontroller in an [iSiLER 15 Color Glowstick](https://www.amazon.com/dp/B07BFT2FK2/ref=nosim) with my own microcontroller. This is primarily because I lack the tools or knowledge to identify or program a PIC microcontroller, but I do have tools for Arduino.

This firmware implements the sleep functionality of the ATTiny85, which drastically extends the idle battery life of the glowstick. Despite this reduction in battery draw while off, I would still recommend removing the batteries or putting a plastic tab in when storing the glowstick.

# Why?!
I am a fan of the concerts presented by [Hololive](https://hololivepro.com/en/), and LED glowsticks are a staple at their concerts. During the concerts, each performer has a designated color that attendees are encouraged to set their glowstick to. Although Hololive usually sells their own glowsticks with all of the correct colors for that concert's performers, they can be expensive and, for oversees fans and online-only attendees, hard to get a hold of.

Fortunately, you can buy generic glowsticks on Amazon. Unfortunately, they don't always have the *right* colors. Notably, one of the most common colors in Hololive concerts is Light Pink, which most generic glowsticks lack. They might get close, but they're not actually correct.

With this firmware, any arbitrary set of colors can be programmed into the glowstick! Is it a rediculous effort to swap a non-pin-compatible microcontroller onto the glowstick board just to change a few colors? Absolutely! Is it worth it? That is an exercise for the reader to decide.

# Features
- Low battery usage while off
- 12 colors matching the Hololive standard colors
    - Red
    - Orange
    - Yellow
    - Light Green
    - Green
    - Blue
    - Light Blue
    - Purple
    - Violet
    - Pink
    - Light Pink
    - White
- 2 Color Cycles
    - 2 seconds per color
    - 0.5 seconds per color

# Requirements
- A glowstick
    - Recommend 4.5v - 5v power. 3x AA or AAA batteries should suffice.
- An ATTiny85
- An ATTiny programmer or Ardino programmed as a programmer
- Soldering supplies
- Thin, enameled wire
- Resin (Optional. I usually use UV-curing resin)
- Heat shrink
- Arduino IDE
    - Requires the "ATtiny25/45/85" board definition
    - Requires the following libraries:
        - Debounce


# Installation
1. Download the project folder
2. Use Arduino IDE to open glowstick_85\glowstick_85.ino
3. Connect the ATTiny85 to the programmer
4. Click "Tools > Programmer" and select your programmer
5. Click "Sketch > Upload Using Programmer" to compile the firmware and upload it to the ATTiny85
6. Disassemble your glowstick and remove the existing microcontroller
7. Wire the ATTiny85 pins to the glowstick
    - If there is any chance you may want to reprogram this microcontroller, I recommend putting in an ICP connection, too.

| ATTiny85 Pin | Glowstick Function | ICP Header |
| ------------ | ------------------ | ---------- |
| 1            |                    | RST        |
| 2            | Button 1           |            |
| 3            | Blue LED           |            |
| 4            | GND                | GND        |
| 5            | Red LED            | MOSI       |
| 6            | Green LED          | MISO       |
| 7            | Button 2           | SCK        |
| 8            | Vcc (+4.5-5v)      | Vcc        |

# Usage
Press the buttons to cycle through the modes and colors.