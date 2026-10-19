# FieldNotes

A small, wearable name-tag type device that one can use when "in the field" to quickly capture notes by pressing a button and talking. At the end of the day, the user will be able to receive a timestamped recap about everything that happened throughout the day. It helps organize what happened and highlights any urgent things they need to take care of.

Technical details: [Hardware](Hardware.md) · [Software](Software.md)

## How it works (MVP)
1. Clip the device on.
2. Press the button and say the note.
3. The device saves the audio with a timestamp when you click the button again.
4. At the end of the day, move the recordings off the device (method still open, see [Hardware](Hardware.md#getting-recordings-off-the-device)).
5. A laptop script turns the recordings into one timestamped text document for the day.
6. Stretch goal: an AI model reads that document and pulls out the important and urgent items.

## MVP scope
This is an MVP, not the final product. The goal is to prove the core loop works: **press, talk, get a timestamped daily recap.**

In the MVP:
- Breadboard prototype (ESP32, microphone, 2 buttons)
- Recording with timestamps
- Getting recordings off the device
- A laptop script that transcribes them into a daily notes document

Not in the MVP (later):
- Case/enclosure and clips
- Battery and charging
- Mobile or web app
- Accounts, teams, subscriptions
- Email recaps and export formats

**The MVP is done when** someone can wear the prototype for a day, record notes, and get back an accurate, timestamped document of what they said.

## Decisions so far
- Prototype on a breadboard with an ESP32, a small microphone, and 2 buttons.
- MVP software is a laptop script first; a mobile or web app comes after (decided 2026-10-19).

## Product ideas

### Why a name tag?
- Easy to clip on and off
- Close to the mouth, so you don't have to talk into your wrist, for example
- Small and easy to give to others; we want a minimal change in workflow

We also discussed modular hardware. Maybe the device can also be attached to a clip that can be clipped onto a hard hat.

### How is this different from what exists?
There are already wearable AI recorders (e.g. Plaud NotePin, Limitless Pendant, Bee; check what's currently on the market). Worth looking at what they do well and why they don't fit field work. Possible angles to validate:
- Quick push-to-talk notes instead of recording everything all day
- Built for field teams (one person can hand devices out to others)
- Cheap per-device pricing
- Could potentially target construction and trade jobs. Smaller businesses and teams.

### Later (not MVP)
- Durability: dust, rain, drops
- Buttons that work with gloves
- Battery that lasts a full shift
- Privacy: making it obvious when it's recording, and deciding where recordings are stored

## Money
- Subscription model? Has to be cheap.
- Charge per device? Buy 3 devices for $99 and have a free version, then pay an extra subscription for the AI pro features, something like an extra $10 a month.
- One idea from [Software](Software.md): the raw timestamped transcript is the free version, and the AI recap is the pro feature. 

## Need finding
- Who is actually wearing these? Project managers on a site/in the field. Not only construction. Some workers.
- What type of things would people say into this device?

Questions to ask potential customers:
- How do you capture notes in the field today (notebook, phone notes, texting yourself, voice memos)? What's annoying about it?
- Why wouldn't a voice memo app on your phone be enough?
- Where do your notes need to end up (daily report, email to your boss, project software like Procore)?
- Who should see the recap: just you, your manager, or both? (This decides whether we need accounts and a team view.)
- What does your end-of-day routine look like? Would you plug a device in, or does it need to sync on its own?
- How many notes do you take in a day, and how long is a shift?
- How loud is your work site? Do you wear gloves or a hard hat?
- Are there rules about recording on your sites?

## TODO
- Build the breadboard prototype and record a first .wav file (see [Hardware](Hardware.md)).
- Get the laptop script working end to end (see [Software](Software.md)).
- Talk to 5–10 potential customers using the questions above.
