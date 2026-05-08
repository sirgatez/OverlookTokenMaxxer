# Overlook Token Maxxer

## Pitch
Big Tech insisting on Token Maxxing to grind that promo?

Reliving the days of lines of code being the measurement of success? 

Show your friends the absurdity of it all with this fun Halloween demo!

## Overview
A USB HID demo for an ATmega32U4 board such as an Arduino Leonardo, Micro, or
USB Beetle. After a startup delay, it types notes into a local chat window as
Jack works on his book while caretaking an empty mansion in the off season.

Jack cannot read the response. The device guesses what might have been said and
keeps typing. It can recover for a while, then slide back into concern, paranoia,
or manic repetition. The repeated phrase only appears after the demo has already
started to unravel.

Use only on machines you own or where you have explicit permission.

## Hardware

- ATmega32U4 board with native USB HID support
- Arduino Leonardo, Arduino Micro, or compatible USB Beetle-style board
- USB cable
- A local chat app or web UI already open and focused

## Arduino IDE Setup

1. Open `OverlookTokenMaxxer/OverlookTokenMaxxer.ino` in Arduino IDE.
2. Select `Tools > Board > Arduino AVR Boards > Arduino Leonardo`.
3. Select the correct port.
4. Upload the sketch.
5. Open a local chat UI and click inside its input box.
6. Plug in or reset the board. It waits 15 seconds before typing.

Unplug the board to stop it immediately.

## Arduino CLI Setup

On macOS with Homebrew:

```sh
brew install arduino-cli
arduino-cli core update-index
arduino-cli core install arduino:avr
arduino-cli lib install Keyboard Mouse
```

Confirm the attached board and port:

```sh
arduino-cli board list
```

This project was flashed to an attached Arduino Micro. Before upload it was
detected as:

```text
/dev/cu.usbmodemHIDPC1 arduino:avr:micro
```

After upload and reset, it reconnected as:

```text
/dev/cu.usbmodemHIDFG1 arduino:avr:micro
```

Compile for Arduino Micro:

```sh
arduino-cli compile --fqbn arduino:avr:micro OverlookTokenMaxxer
```

Upload to the current port from `arduino-cli board list`:

```sh
arduino-cli upload -p <PORT> --fqbn arduino:avr:micro OverlookTokenMaxxer
```

For an Arduino Leonardo, use `arduino:avr:leonardo` instead:

```sh
arduino-cli compile --fqbn arduino:avr:leonardo OverlookTokenMaxxer
arduino-cli upload -p <PORT> --fqbn arduino:avr:leonardo OverlookTokenMaxxer
```

After upload, the board resets. Keep focus away from terminals, email, chat, or
admin tools. The sketch waits 15 seconds before typing.

## Demo Modes

The sketch loops forever until unplugged. Each mode chooses the next mode with
odds that favor escalation, recovery, or fixation.

- Sane: caretaking routines, quiet halls, and blocked writing
- Measuring: page counts, rounds through the hotel, and strained order
- Concerned: noises, music, changing rooms, and growing uncertainty
- Paranoid: the house, the maze, and the book begin to feel aware
- Manic: repeated slogans, broken phrasing, and Overlook phrases
- Recovery: slower notes and walk-away pauses

Notes come from written phrases plus rotating subjects: rooms, objects, chores,
sounds, signs, weather, and fears. While Jack is sane, several submissions
usually stay on the same subject. As he deteriorates, subjects can change
abruptly or become a fixation.

Each submitted prompt may contain several sentences. Sane prompts are usually
short and consistent, while paranoid and manic prompts may be a single fragment,
a long paragraph, or a repeated fixation. Sentence joins usually use a normal
space, with missing or doubled spaces becoming more likely as Jack deteriorates.

## Timing

The sketch defaults to short demo timing:

- Startup delay: 15 seconds
- Mode durations: roughly 30-95 seconds each
- Normal note pauses: sub-second to 9 seconds depending on mode
- Walk-away pauses: 12-90 seconds in short timing

For a longer version with multi-minute walk-away breaks, edit:

```cpp
const bool USE_SHORT_DEMO_TIMING = true;
```

to:

```cpp
const bool USE_SHORT_DEMO_TIMING = false;
```

## Chat vs Editor Mode

By default the sketch submits each note to the focused chat box:

```cpp
const bool SUBMIT_PROMPTS_TO_CHAT = true;
```

For a safer dry run in a blank editor, set it to `false`. In editor mode the
sketch leaves blank lines between notes and does not require a local chat app.

## Safety Limits

The sketch intentionally avoids:

- Terminal interaction
- Command execution
- Opening apps
- System shortcuts
- Modifier-key combinations
- Mouse clicks by default

Recommended test targets are local chat windows, Notepad, TextEdit, VS Code empty
documents, or another safe local editor. Do not use this in terminals, email
clients, admin tools, shared production systems, or anything connected to real
work.

## Files

- `AI Instructions.txt`: original Halloween project brief
- `OverlookTokenMaxxer/OverlookTokenMaxxer.ino`: Arduino HID sketch
- `docs/test_plan.md`: upload and demo checklist
