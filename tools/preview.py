#!/usr/bin/env python3
"""Render a 320x240 PNG of what the display would show now, using live prices.

Mirrors the layout in spot-price-display.yaml and the formula in spot_price_logic.h (keep them in sync).
Usage: .venv/bin/python tools/preview.py [out.png] [--scale 3]
"""
import argparse
import json
import urllib.request
from datetime import datetime, timezone
from zoneinfo import ZoneInfo

from PIL import Image, ImageDraw, ImageFont

URL = "https://api.porssisahko.net/v2/latest-prices.json"
TZ = ZoneInfo("Europe/Helsinki")
SLOT = 15 * 60

# Same placeholders as the firmware's number entities.
SETTINGS = dict(margin=0.49, transfer=4.0, tax=2.83,
                green_below=10.0, red_from=20.0)

GREEN, YELLOW, RED, GREY = (0x1E, 0xA0, 0x46), (0xF5, 0xC5, 0x18), (0xD6, 0x2A, 0x2A), (0x60, 0x60, 0x60)
WHITE, DARK, LINE = (255, 255, 255), (0x20, 0x20, 0x20), (0x90, 0x90, 0x90)


def total(s, start, spot):
    return spot + s["margin"] + s["transfer"] + s["tax"]


def colour(s, v):
    return GREEN if v < s["green_below"] else YELLOW if v < s["red_from"] else RED


def font(size, bold=False):
    for name in (["DejaVuSans-Bold.ttf", "LiberationSans-Bold.ttf"] if bold else
                 ["DejaVuSans.ttf", "LiberationSans-Regular.ttf"]):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default(size)


def fetch():
    req = urllib.request.Request(URL, headers={"User-Agent": "spot-price-display-preview"})
    with urllib.request.urlopen(req, timeout=15) as r:
        data = json.load(r)
    slots = {}
    for p in data["prices"]:
        start = int(datetime.fromisoformat(p["startDate"].replace("Z", "+00:00")).timestamp())
        slots[start] = float(p["price"])
    return sorted(slots.items())


def render(slots, now, s=SETTINGS):
    img = Image.new("RGB", (320, 240), "black")
    d = ImageDraw.Draw(img)
    idx = next((i for i, (st, _) in enumerate(slots) if st <= now < st + SLOT), -1)
    top_h = 130
    if idx < 0:
        d.rectangle([0, 0, 319, top_h - 1], fill=GREY)
        d.text((160, 50), "No price data", font=font(22), fill=WHITE, anchor="mm")
        return img
    v_now = total(s, *slots[idx])
    c = colour(s, v_now)
    text = DARK if c == YELLOW else WHITE
    d.rectangle([0, 0, 319, top_h - 1], fill=c)
    big, mid, small = font(60, True), font(22), font(13)
    num = f"{v_now:.1f}"
    w = d.textlength(num, font=big)
    x = (320 - w - 62) / 2
    d.text((x, 8), num, font=big, fill=text, anchor="la")
    d.text((x + w + 6, 70), "c/kWh", font=mid, fill=text, anchor="ls")
    if c == GREEN:
        line = "Cheap now"
    else:
        g = next((max(st, now) for st, sp in slots if st + SLOT > now and total(s, st, sp) < s["green_below"]), 0)
        if not g:
            line = "No green in known prices"
        else:
            gt, nt = datetime.fromtimestamp(g, TZ), datetime.fromtimestamp(now, TZ)
            line = f"Green from {'tomorrow ' if gt.date() != nt.date() else ''}{gt:%H:%M}"
    d.text((160, 100), line, font=mid, fill=text, anchor="mm")

    n, ch_top, ch_bot = 48, 138, 222
    window = slots[idx:idx + n]
    maxv = max([s["red_from"] + 2] + [total(s, st, sp) for st, sp in window])

    def y_of(v):
        return ch_bot - int((ch_bot - ch_top) * max(v, 0) / maxv)

    for i, (st, sp) in enumerate(window):
        v = total(s, st, sp)
        x0, x1 = i * 320 // n, (i + 1) * 320 // n - 1
        y = min(y_of(v), ch_bot - 1)
        d.rectangle([x0, y, x1 - 1, ch_bot - 1], fill=colour(s, v))
        if i == 0:
            d.rectangle([x0, y - 1, x1 - 1, ch_bot - 1], outline=WHITE)
        lt = datetime.fromtimestamp(st, TZ)
        if lt.minute == 0 and lt.hour % 3 == 0:
            d.line([x0, ch_bot, x0, ch_bot + 3], fill=LINE)
            d.text((x0, ch_bot + 4), f"{lt.hour:02d}", font=small, fill=LINE, anchor="ma")
    for lim in (s["green_below"], s["red_from"]):
        y = y_of(lim)
        for x in range(0, 320, 6):
            d.line([x, y, x + 2, y], fill=LINE)
    return img


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out", nargs="?", default="preview.png")
    ap.add_argument("--scale", type=int, default=3)
    a = ap.parse_args()
    now = int(datetime.now(timezone.utc).timestamp())
    img = render(fetch(), now)
    img.resize((320 * a.scale, 240 * a.scale), Image.NEAREST).save(a.out)
    print(f"wrote {a.out}")


if __name__ == "__main__":
    main()
