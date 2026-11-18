# Notes for Claude: spot price display

Owner: Lotta Hardman. A small Wi-Fi unit with a screen in a room at home that shows the electricity price, so people
know when not to run the washing machine or take a shower. Started 2026-11-18.

## Decisions so far
- **Display only.** It reads prices and shows them; it never switches anything. Powered from a USB charger.
- **Screen only**, no extra LED/light. The colour must be readable across the room, so the whole screen
  (or a large area) takes the colour, not just a small dot.
- **Total price, with transfer fees**, in c/kWh incl. VAT:
  `spot (incl. VAT) + seller's margin + transfer fee + electricity tax`. The fee values come from the user's
  bills (not filled in yet; the transfer fee may differ day/night).
- **Colours:** green below 10 c/kWh, yellow 10–20, red from 20 c/kWh (total price). Limits are settings.
- **Look:** "like the Fingrid app": the user wants display options similar to it. Not yet clarified what exactly
  (probably a bar chart of the coming prices coloured green/yellow/red + the price now). Ask before designing.
- Show when it's cheap again ("green from 14:15"). No data (Wi-Fi/API down, stale prices) → grey, never green.

## Price data
- `https://api.porssisahko.net/v2/latest-prices.json`: 15-minute prices (Nord Pool day-ahead, area FI) in c/kWh
  **incl. VAT**, ~192 slots (48 h), times in UTC. Tomorrow's prices appear around 14:00 Finnish time.
  Checked working 2026-11-18. v1 of the same API gives hourly prices.
- Alternative: `https://api.spot-hinta.fi/TodayAndDayForward` (15-min, €/kWh, with and without VAT, local times).
- Since 2025-10 the Nordic day-ahead market uses 15-minute slots: handle 15-min prices, not hourly.

## Hardware (proposed, not bought)
- ESP32 board with a built-in colour screen. For readability across a room prefer the 2.8" 320×240
  "Cheap Yellow Display" (ESP32-2432S028R, ~15 €) over the 1.9" LilyGO T-Display-S3.
- Firmware: ESPHome (YAML + small C++ lambdas, Wi-Fi updates) unless that gets in the way; Arduino is the fallback.

## Git
- Commit as Lotta Hardman <lotta.hardman@gmail.com> (set in this repo's config; the laptop's global identity is
  the work address, never use it). No remote yet. Never push to the JCO Digital organisation.
