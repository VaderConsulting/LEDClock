# LEDClock

Arduino clock that shows time on a TM1637 4-digit display, keeps time with a DS1307 RTC (`RTClib`), and uses a notched-shaft encoder to set hours and minutes. MsTimer2 runs the encoder button task. Written for Dave Robinson's bench hardware; comments point at Lester Lo's encoder, TM1637TinyDisplay, and Arduino forum notes.

**Source last updated:** 2021-02-01  
**Language:** C++ / Arduino  
**Target:** Arduino AVR with I2C RTC, TM1637, rotary encoder  
**Output:** Arduino sketch

## Solution structure

| Project | Language | Type | Purpose |
|---------|----------|------|---------|
| `LED_Clock` | C++ / Arduino | sketch | TM1637 clock with DS1307 RTC and encoder set-time modes |

## How to open

Open `LED_Clock/LED_Clock.ino` in the Arduino IDE. Libraries used include `RTClib`, `MsTimer2`, `NSEncoder`, and `TM1637TinyDisplay`.

## Requirements

- Arduino IDE

## Attribution and provenance

Dave Robinson / VaderConsulting sketch from the Arduino archive. Depends on third-party libraries (Adafruit RTClib, MsTimer2, Lester Lo Notched Shaft Encoder, Jason Cox TM1637TinyDisplay) which live in sibling repos, not this tree.

## License

MIT © 2026 VaderConsulting for Dave's sketch. See `LICENSE`. Third-party libraries keep their own licences in their repos.
