#!/usr/bin/env python3
"""Save recordings from the FieldNotes ESP32 as .wav files in hardware/test-recordings/.
"""

import argparse
import array
import errno
import math
import struct
import wave
from datetime import datetime
from pathlib import Path

import serial

BAUD = 921600  # must match CONFIG_ESP_CONSOLE_UART_BAUDRATE in sdkconfig.defaults
OUT_DIR = Path(__file__).resolve().parent.parent / "test-recordings"

# Frames from main.c: "FNAU", type, payload length (little-endian), payload
MAGIC = b"FNAU"
HEADER = struct.Struct("<4scH")
MAX_AUDIO = 1024  # audio frames are 640 bytes


def save(pcm, rate, started, skipped):
    OUT_DIR.mkdir(exist_ok=True)
    path = OUT_DIR / f"{started:%Y-%m-%d_%H-%M-%S}.wav"
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(rate)
        w.writeframes(pcm)

    samples = array.array("h", pcm)
    seconds = len(samples) / rate
    if not samples or min(samples) == max(samples):
        print(f"saved {path} ({seconds:.1f} s) but there's no signal, check the mic wiring")
    else:
        peak = max(-min(samples), max(samples))
        print(f"saved {path} ({seconds:.1f} s, peak {20 * math.log10(peak / 32768):.0f} dBFS)")
    if skipped:
        print(f"  skipped {skipped} bytes of bad data, so the audio has gaps")


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("-p", "--port", default="/dev/cu.usbserial-0001")
    args = parser.parse_args()

    ser = serial.Serial()
    ser.port = args.port
    ser.baudrate = BAUD
    ser.dtr = ser.rts = False  # don't trigger the board's auto-reset when the port opens
    # Take the same lock idf.py monitor and flash use
    ser.exclusive = True
    try:
        ser.open()
    except serial.SerialException as e:
        if e.errno != errno.EWOULDBLOCK:  # someone else holds the lock
            raise
        raise SystemExit(f"{args.port} is in use by another program, probably the ESP-IDF monitor. ")
    print(f"listening on {args.port}")

    pcm = None
    rate = 0
    started = None
    skipped = 0
    buf = bytearray()
    try:
        while True:
            # Read whatever has arrived
            buf += ser.read(ser.in_waiting or 1)
            while True:
                # Drop anything before the next frame
                # If there's no marker yet, keep the last 3 bytes in case it's split across reads.
                i = buf.find(MAGIC)
                if i < 0:
                    i = max(len(buf) - len(MAGIC) + 1, 0)
                if pcm is not None:
                    skipped += i
                del buf[:i]
                if len(buf) < HEADER.size:
                    break

                _, kind, length = HEADER.unpack_from(buf)
                if not ((kind == b"S" and length == 4) or (kind == b"E" and length == 0)
                        or (kind == b"A" and length <= MAX_AUDIO and length % 2 == 0)):
                    # "FNAU" turned up inside other data, keep looking past it
                    if pcm is not None:
                        skipped += 1
                    del buf[:1]
                    continue
                if len(buf) < HEADER.size + length:
                    break
                payload = bytes(buf[HEADER.size:HEADER.size + length])
                del buf[:HEADER.size + length]

                if kind == b"S":
                    (rate,) = struct.unpack("<I", payload)
                    started = datetime.now()
                    pcm = bytearray()
                    skipped = 0
                    print("recording...")
                elif kind == b"A":
                    if pcm is not None:
                        pcm += payload
                elif pcm is not None:
                    save(pcm, rate, started, skipped)
                    pcm = None
    except KeyboardInterrupt:
        if pcm:
            save(pcm, rate, started, skipped)


if __name__ == "__main__":
    main()
