# Hardware (MVP)

Part of the MVP technical plan. See also: [Overview and Product](Overview%20and%20Product.md) · [Software](Software.md)

## MVP device
- A small device prototyped on a breadboard, using an ESP32 and a small microphone.
- 2 buttons:
  - Button 1: start recording and stop recording.
  - Button 2: initiate a Bluetooth connection to send recordings to another device. (The transfer method is still open, see [Getting recordings off the device](#getting-recordings-off-the-device).)
- A status LED so the wearer knows when it's recording or transferring. Without it there's no way to tell if a note was captured.
- **Recording always comes first.** Pressing the button always records, even if the clock isn't set or something else is wrong. A missing timestamp is OK; a missing note is not.

## First step: record audio
As a first step, see if we can just record audio in .wav or .mp3 format. Ideally whichever is the smaller file format and has clear enough audio, but I would not worry about audio clarity too much right now.

Notes:
- Start with .wav. The ESP32 can write it directly. MP3 is smaller, but it has to be encoded on the device, which is a lot harder on an ESP32.
- Record at 16 kHz, mono, 16-bit. That's plenty for speech, and speech-to-text models like Whisper work at 16 kHz anyway. That comes to about 1.9 MB per minute (about 115 MB per hour).
- If files are too big later, simple compression like ADPCM (about 4x smaller) is much easier than MP3 on an ESP32. We can also convert to MP3 after the files are off the device.

## Proposed parts list (confirm after prototyping)
| Part | Suggestion | Why |
| --- | --- | --- |
| Microcontroller | ESP32 dev board (ESP32-S3 if USB transfer is a contender) | WiFi and Bluetooth built in; the S3 also has native USB |
| Microphone | I2S MEMS mic breakout (e.g. INMP441) | Digital mic, common with ESP32, cleaner than analog mics |
| Storage | microSD card module | Built-in flash only fits about a minute of audio |
| Buttons | 2 push buttons | Record, transfer |
| Indicator | 1 LED + resistor | Shows recording/transferring |
| Power | USB from a laptop or power bank | Battery is post-MVP |

All-in-one option worth a look: Seeed Studio XIAO ESP32S3 Sense. It's a tiny board with a microphone, microSD slot, USB-C, and battery charging built in, so there are fewer parts on the breadboard and it's already close to name-tag size.

## Storage format
Everything uses **local time**. One settings file for the whole device, one folder per day, and inside it one folder per note. Each note folder holds a `metadata.json` (timestamps and any other data) and, next to it, the audio file.

```text
device.json            <- settings for the whole device
2026-10-19/            <- local date, year-month-day
  note001/
    metadata.json      <- info about this one note
    audio.wav
  note002/
    metadata.json
    audio.wav
2026-10-20/
  note001/
    ...
no-date/               <- notes recorded while the clock wasn't set
  note001/
    metadata.json
    audio.wav
```

`device.json`, which is set up once and only changes if the device's settings change:
```json
{
  "device_id": "fn-001",
  "timezone": "EST5EDT,M3.2.0,M11.1.0"
}
```

`metadata.json`, which is written once per note:
```json
{
  "start_time": "2026-10-19T14:32:05-04:00",
  "duration_s": 42,
  "audio_file": "audio.wav"
}
```

How local time works:
- The device reads `timezone` from `device.json` when it starts up. Changing the timezone means editing that file, not reflashing the firmware. For the MVP we write it by hand. Later, the laptop script or app can update it when syncing.
- `start_time` is local time plus the UTC offset (`-04:00`), in ISO 8601 format. It shows the time the user actually saw and still converts exactly to UTC. UTC only (`2026-10-20T01:00:00Z`) would put a 9 pm note in the next day's folder, and a Unix timestamp (`1792434725`) isn't readable when you open the file.
- The offset is saved with every note, so old notes stay correct even if the timezone or daylight saving changes later.
- A note goes in the folder for the local date it *started* on, even if it runs past midnight.

If the clock isn't set (e.g. right after losing power):
- The device still records. Recording never waits on the clock.
- How the device can tell: an ESP32 with no time set thinks it's 1970, so any year before 2026 means "not set".
- The note goes in `no-date/` instead of a date folder, and `start_time` is `null`:
  ```json
  {
    "start_time": null,
    "boot": 12,
    "uptime_s": 3605,
    "duration_s": 42,
    "audio_file": "audio.wav"
  }
  ```
- `boot` counts how many times the device has turned on, and `uptime_s` is the number of seconds since it last turned on. Together they keep the notes in order, and let us estimate the real time later if we want to.

Other rules:
- Dates are written year-month-day (`2026-10-19`) so the folders sort in order.
- Note numbers are zero-padded (`note001`) so `note10` doesn't sort before `note2`. Each folder starts at `note001`, and the next note is always the highest existing number + 1, so a restart never overwrites a note.
- Write `metadata.json` last, when the recording stops. A note folder without one means the recording got cut off (e.g. power loss). The audio is still kept and transcribed.
- Fields we might add later: battery level, firmware version, a project/site tag.

## Open questions

### Timestamps and metadata
Can we save timestamps when each audio is recorded, or extract that somehow? We should have some specific metadata format.

Notes:
- The ESP32 has a clock, but it doesn't know the real date/time on its own, and it resets when it loses power. Options: set the clock from the laptop/phone every time it syncs (or from the internet over WiFi), or add a small real-time clock module (e.g. DS3231) with its own backup battery.
- Timezone: the ESP32 can't work out the timezone on its own, so it has to be told. It's stored in `device.json` as a POSIX timezone string (US Eastern is `EST5EDT,M3.2.0,M11.1.0`), see [Storage format](#storage-format). The ESP32's time library then handles daylight saving automatically.
- Timestamps are saved in each note's `metadata.json`, see [Storage format](#storage-format).
- A device ID matters once there's more than one device (e.g. the 3-pack), so recordings get matched to the right person.

### Storage
How much data can we save locally on a device like this? Minutes or hours? Do they have to offload data every day?

Notes:
- At about 1.9 MB per minute, the ESP32's built-in flash (usually 4 MB, partly used by the program) only holds about a minute. A microSD card holds far more: 8 GB is about 70 hours.
- With a microSD card, storage isn't a reason to offload every day. Getting the daily recap is.

### Getting recordings off the device
Instead of Bluetooth, could we just have a USB-C port on the device that we plug into a phone and transfer data that way? We should talk about that, and would likely need to talk with actual potential customers: what does the end-of-day workflow look like? This also depends on the software endpoint we decide on (more about that in [Software](Software.md)).

Notes: a rough comparison for one day of audio (about 30–115 MB, i.e. 15–60 minutes of talking):

| Method | Time for a day's audio | Pros | Cons |
| --- | --- | --- | --- |
| Bluetooth (BLE) | Slow: roughly 10–40 min | Wireless, works with phones | Slow, uses more battery. iPhones only allow BLE for custom devices, not classic Bluetooth |
| WiFi | Fast: a couple of minutes or less | Built into the ESP32; could sync on its own when back on office WiFi | Needs a network; connecting to new networks is fiddly |
| USB-C | A few minutes | The same port can charge the battery; shows up as a USB drive | Needs a board with native USB (e.g. ESP32-S3); someone has to plug it in |
| Pull the microSD card | Seconds | Easiest to build | Fine for testing, not for customers |

For testing the laptop script, USB or pulling the microSD card is the quickest. The real method can be picked once customers tell us what their end of day looks like.

### Button behavior
- Press once to start and again to stop, or hold to talk (like a walkie-talkie)? Hold-to-talk means you can't forget to stop, but it's harder with full hands.
- Should a recording auto-stop after a max length (e.g. 5 minutes), so a forgotten recording doesn't fill up storage?

### Noise
Not a priority for the MVP, but field sites can be loud. One early test with background noise (traffic, power tools) will show whether transcription still works.

### Post-MVP: battery and charging
Charging this device / battery powered. These are questions for after the MVP, but putting them here for now. Lowkey, we should just be able to use one of those small disk (coin cell) batteries.

Notes:
- A coin cell (like a CR2032) probably won't work. The ESP32 draws over 100 mA when WiFi or Bluetooth is on, and coin cells are made for a few mA. A small rechargeable LiPo battery with USB-C charging is the usual setup.
- For the MVP, just power it over USB.

## TODO
- Specify the hardware we are using for the MVP (e.g. microcontroller, buttons, etc.). This should be done after some prototyping, but can be changed in the future. Starting point: the [proposed parts list](#proposed-parts-list-confirm-after-prototyping) above.
