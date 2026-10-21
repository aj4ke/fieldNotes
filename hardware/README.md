# FieldNotes firmware

FieldNotes hardware using esp32 and an INMP441 microphone.

## Setup

1. Install ESP-IDF v6.0.2 with the ESP-IDF Installation Manager (EIM):

   macOS:

   ```sh
   brew install libgcrypt glib pixman sdl2 libslirp dfu-util cmake python
   brew tap espressif/eim
   brew install eim
   eim install -i v6.0.2
   ```

   Linux (Debian):

   ```sh
   echo "deb [trusted=yes] https://dl.espressif.com/dl/eim/apt/ stable main" | sudo tee /etc/apt/sources.list.d/espressif.list
   sudo apt update
   sudo apt install eim-cli
   eim install -i v6.0.2
   ```

   For other distros, see [Espressif's Linux setup](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32/get-started/linux-setup.html).

   The Makefile expects the activation script at `~/.espressif/tools/activate_idf_v6.0.2.sh`. If yours is somewhere else, add `IDF_ACTIVATE=/path/to/activate_idf_v6.0.2.sh` to the make commands.

2. Wire the board:

| Part | Pin | esp32 |
|---|---|---|
| INMP441 | VDD | 3.3V |
| INMP441 | GND | GND |
| INMP441 | SD | GPIO18 |
| INMP441 | WS | GPIO19 |
| INMP441 | SCK | GPIO21 |
| INMP441 | L/R | GND |
| LED | anode | GPIO15 |
| LED | cathode | GND |
| Button | one side | GPIO23 |
| Button | other side | GND |

4. Plug the board in over USBc and check which port its on `ls /dev/cu.*` and configure that in the makefile line 1. 

   On Linux the port is usually `/dev/ttyUSB0` (`ls /dev/ttyUSB*`). Your user also needs access to serial ports: run `sudo usermod -aG dialout $USER`, then log out and back in.

## Run

From ~/FieldNotes/hardware:

```sh
make build
make flash
make capture
```
