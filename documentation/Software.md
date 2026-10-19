# Software (MVP)

Part of the MVP technical plan. See also: [Overview and Product](Overview%20and%20Product.md) · [Hardware](Hardware.md)

## Goal
The idea is that we take the data saved locally on the device and somehow turn it into something the user can read.
- We need a way to transport/display this data for the end user. I don't know if this should be a mobile application, which is just an endpoint for us.
- We ideally also want a recap, either through email or a website/app someone can look at to see their history. Then they can export their files to a format they want.

## MVP plan: laptop script first
The first version is a simple script on a laptop, not a mobile or web app. It proves the whole pipeline works before we spend time building an app. A Python script runs on Windows, Mac, and Linux, so it works on whatever laptop the team has.

We connect to the device, extract the recordings, use a speech-to-text model (free model), and create a document that has that day's notes.

1. **Get the recordings** and their metadata off the device (folder layout: [Storage format](Hardware.md#storage-format)). How depends on the transfer method, see [Hardware](Hardware.md#getting-recordings-off-the-device).
2. **Transcribe** each recording with a free speech-to-text model. Suggestion: [Whisper](https://github.com/openai/whisper), which is open-source and runs on a laptop at no cost.
3. **Build the day's document.** This will include the following:
   - Date
   - Timestamps
   - "Recording in text format"
4. **Stretch: AI recap.** This "raw" format could potentially be the free version, and then we can have an AI model parse it and take out important findings, urgent items, etc. That would be the pro feature (see [Money](Overview%20and%20Product.md#money)).

The script keeps the same folder layout as the device and adds its output next to it:

```text
device.json
2026-10-19/
  daily-notes.md      <- the day's document (see example below)
  note001/
    metadata.json
    audio.wav
    transcript.txt    <- added by the script
```

Every recording makes it into the recap, even without a timestamp. Notes from `no-date/` (recorded while the clock wasn't set) and note folders with no `metadata.json` (cut off) still get transcribed. They go at the end under "Time unknown", in the order they were recorded.

### Example output
```markdown
# Field notes: 2026-10-19

## Recap (AI, pro)
Urgent:
- Call the supplier about the rebar shortage (08:14)

Highlights:
- East wall pour moved to Thursday (10:52)

## All notes
- 08:14: "Rebar delivery for the east wall is short about 20 pieces, need to call the supplier today."
- 10:52: "Talked to the contractor, east wall pour is moving to Thursday."
- 15:30: "Inspector came by, all good on the footings."

## Time unknown
- "Need to order more caulk for the second floor windows."
```

## After the MVP
- Once the script works, turn it into a minimal application, mobile or web based.
- Send the recap through email, or show it in a website/app where someone can see their history.
- Let users export their files to the formats they want.

## Open questions
- Which platform for the real app: iOS, Android, web, or cross-platform (e.g. React Native, Flutter)? This depends on the transfer method. For example, a web page can't talk to Bluetooth devices on iPhones.
- Where do transcription and AI processing happen long term: on the user's phone, or on our server? A server is simpler and works with any phone, but it costs money per user and means we hold their recordings.
- Do we keep the audio after it's transcribed, or delete it? (Privacy and storage cost.)
- Who sees the recap: just the wearer, or also their manager? (Being asked in [Need finding](Overview%20and%20Product.md#need-finding).)
- How do notes get tied to a job site or project if someone visits several in a day? (E.g. say the site name at the start of a note.)

## TODO
- Write the first script: one recording in, a timestamped transcript out.
- Try Whisper on a few real recordings from the prototype and see how accurate it is.
