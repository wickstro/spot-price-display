# Spot price display

A small Wi-Fi screen that shows the electricity price right now, green / yellow / red, so everyone in the room knows
when to wait with the washing machine or the shower. It only shows the price; it doesn't control anything.

## Plan
1. **Price:** fetch the 15-minute spot prices for Finland (porssisahko.net, today and tomorrow) a few times a day,
   add the seller's margin, transfer fee and electricity tax, and colour each slot:
   green under 10 c/kWh, yellow 10–20, red from 20 c/kWh.
2. **Screen:** the colour of the price now, big enough to see across the room, the price in c/kWh, a bar chart of
   the coming hours, and when it's cheap again.
3. **Hardware:** an ESP32 board with a built-in 2.8" colour screen, in a small case, on a USB charger.

## To fill in
| Setting | Value | From |
|---|---|---|
| Seller's margin | ? c/kWh | electricity contract |
| Transfer fee (day / night?) | ? c/kWh | grid company bill |
| Electricity tax | ~2.8 c/kWh incl. VAT (check) | bill |
| Green below / red from | 10 / 20 c/kWh | chosen 2026-11-18 |
