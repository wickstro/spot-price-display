# Notes for Claude: spot price display

Owner: Lotta Hardman. A small Wi-Fi unit with a screen in a room at home that shows the electricity price, so people
know when not to run the washing machine or take a shower. Started 2026-11-18.

## Decisions so far
- **Display only.** It reads prices and shows them; it never switches anything. Powered from a USB charger.
- **Screen only**, no extra LED/light. The colour must be readable across the room, so the whole screen
  (or a large area) takes the colour, not just a small dot.
- **Price shown = electricity bill only**, c/kWh incl. VAT 25.5 %: `spot (incl. VAT) + seller's margin 0.49`.
  Transfer fee and electricity tax are left out on purpose: they're on the separate grid company bill (decided
  2026-11-18).
- **Colours:** green below 10 c/kWh, yellow 10–20, red from 20 c/kWh (price shown). Limits are settings.
- **Look** (decided 2026-11-18): top half filled with the colour now, big price, "green from HH:MM"; bottom half
  a bar chart of the next 12 h (48 quarter-hours) coloured green/yellow/red.
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
  the work address, never use it). Remote: github.com/wickstro/spot-price-display (private), via the SSH host
  alias `github-wickstro` (wickstro key only). Never push to jcodigital / the JCO Digital organisation.
