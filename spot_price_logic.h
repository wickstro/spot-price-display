// Price logic for the spot price display. Plain C++ (no ESPHome), so it can be tested on the laptop.
// Prices are c/kWh incl. VAT; times are UTC epoch seconds.
#pragma once

#include <cstdlib>
#include <cstring>
#include <ctime>

namespace spot {

static const int MAX_SLOTS = 200;      // porssisahko v2 gives ~192 quarter-hours (48 h)
static const int SLOT_SECONDS = 15 * 60;

enum Colour { GREEN, YELLOW, RED, GREY };

struct Settings {
  float margin = 0.49f;         // seller's margin, c/kWh incl. VAT (contract)
  float transfer = 4.0f;        // transfer fee, same day and night (placeholder)
  float tax = 2.83f;            // electricity tax incl. VAT (check from the bill)
  float green_below = 10.0f;
  float red_from = 20.0f;
};

struct Slot {
  time_t start;
  float spot;
};

// Days since 1970-01-01 for a proleptic Gregorian date (Howard Hinnant's algorithm). Avoids timegm(),
// which isn't available everywhere.
inline long days_from_civil(int y, unsigned m, unsigned d) {
  y -= m <= 2;
  const long era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = (unsigned) (y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (long) doe - 719468;
}

// "2026-11-18T22:45:00.000Z" -> epoch seconds. Returns -1 if malformed.
inline time_t parse_iso_utc(const char *s) {
  if (s == nullptr || strlen(s) < 19 || s[4] != '-' || s[7] != '-' || s[10] != 'T' || s[13] != ':' ||
      s[16] != ':')
    return -1;
  int y = atoi(s), mo = atoi(s + 5), d = atoi(s + 8), h = atoi(s + 11), mi = atoi(s + 14), se = atoi(s + 17);
  if (mo < 1 || mo > 12 || d < 1 || d > 31 || h > 23 || mi > 59 || se > 60)
    return -1;
  return (time_t) days_from_civil(y, mo, d) * 86400 + h * 3600 + mi * 60 + se;
}

inline float total_price(const Settings &s, const Slot &slot) {
  return slot.spot + s.margin + s.transfer + s.tax;
}

inline Colour colour_for(const Settings &s, float total) {
  if (total < s.green_below)
    return GREEN;
  if (total < s.red_from)
    return YELLOW;
  return RED;
}

class PriceTable {
 public:
  Slot slots[MAX_SLOTS];
  int count = 0;
  time_t last_fetch = 0;  // when prices were last fetched successfully (0 = never)

  void clear() { count = 0; }

  // Add one slot; keeps the table sorted by start (the API lists newest first) and ignores duplicates.
  bool add(time_t start, float spot) {
    if (start < 0 || count >= MAX_SLOTS)
      return false;
    int i = count;
    while (i > 0 && slots[i - 1].start > start)
      i--;
    if (i > 0 && slots[i - 1].start == start) {
      slots[i - 1].spot = spot;
      return true;
    }
    memmove(&slots[i + 1], &slots[i], (count - i) * sizeof(Slot));
    slots[i] = {start, spot};
    count++;
    return true;
  }

  // Index of the slot covering t, or -1.
  int index_at(time_t t) const {
    for (int i = 0; i < count; i++)
      if (t >= slots[i].start && t < slots[i].start + SLOT_SECONDS)
        return i;
    return -1;
  }

  // Colour now; GREY when the clock isn't set or no price covers now.
  Colour colour_now(const Settings &s, time_t now) const {
    int i = index_at(now);
    if (i < 0)
      return GREY;
    return colour_for(s, total_price(s, slots[i]));
  }

  // Start of the first green slot at or after now (the current slot counts). 0 = no green in the known prices.
  time_t next_green_start(const Settings &s, time_t now) const {
    for (int i = 0; i < count; i++) {
      if (slots[i].start + SLOT_SECONDS <= now)
        continue;
      if (total_price(s, slots[i]) < s.green_below)
        return slots[i].start < now ? now : slots[i].start;
    }
    return 0;
  }

  // Latest time covered by the table (end of the last slot), 0 if empty.
  time_t known_until() const { return count ? slots[count - 1].start + SLOT_SECONDS : 0; }

  // True when the last successful fetch is older than max_age seconds (shown as a small "offline" note;
  // prices already fetched are still correct, so the colour stays as long as a slot covers now).
  bool fetch_stale(time_t now, long max_age = 6 * 3600) const {
    return last_fetch == 0 || now - last_fetch > max_age;
  }
};

}  // namespace spot
