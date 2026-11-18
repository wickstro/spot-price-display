# Spot price display

A small Wi-Fi screen that shows the electricity price right now, green / yellow / red, so everyone in the room knows
when to wait with the washing machine or the shower. It only shows the price; it doesn't control anything.

## Plan
1. **Price:** fetch the 15-minute spot prices for Finland (porssisahko.net, today and tomorrow) a few times a day,
   add the seller's margin (transfer fee and electricity tax are on the separate grid bill and left out), and
   colour each slot:
   green under 10 c/kWh, yellow 10–20, red from 20 c/kWh.
2. **Screen:** top half filled with the colour of the price now, the price in c/kWh in big digits and when it's
   green again; bottom half a bar chart of the next 12 h (48 quarter-hours) in green/yellow/red. Grey = no data.
3. **Hardware:** an ESP32 board with a built-in 2.8" colour screen, in a small case, on a USB charger.

## Files
| File | What |
|---|---|
| `spot-price-display.yaml` | ESPHome firmware for the ESP32-2432S028R ("Cheap Yellow Display") |
| `spot_price_logic.h` | Price logic (total price, colours, next green), plain C++ |
| `spot_state.h` | The price tables used by the firmware |
| `test/test_logic.cpp` | Laptop tests for the logic |
| `tools/preview.py` | Renders a PNG of the screen with today's real prices |

## Build
```sh
python3 -m venv .venv && .venv/bin/pip install esphome pillow
cp secrets.example.yaml secrets.yaml        # fill in Wi-Fi etc.
g++ -std=c++17 -Wall -o /tmp/test_logic test/test_logic.cpp && /tmp/test_logic
.venv/bin/python tools/preview.py preview.png
.venv/bin/esphome run spot-price-display.yaml   # first time over USB, later over Wi-Fi
```
The fees and colour limits are settings on the board's web page (`http://spot-price-display.local/`), stored in
flash. Without Wi-Fi it opens a hotspot "Spot price display" for setting up the network.

Board variants: if the colours are inverted or the picture is mirrored, change `invert_colors` / `mirror_x` /
`mirror_y` at the top of the YAML. Some CYD variants use an ST7789 instead of an ILI9341 (`display_model`).

## To fill in
| Setting | Value | From |
|---|---|---|
| Seller's margin | 0.49 c/kWh incl. VAT 25.5 % | electricity contract, given 2026-11-18 |
| Green below / red from | 10 / 20 c/kWh | chosen 2026-11-18 |
