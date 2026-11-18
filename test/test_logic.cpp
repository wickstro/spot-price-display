// Laptop tests for spot_price_logic.h:  g++ -std=c++17 -Wall -o /tmp/test_logic test/test_logic.cpp && /tmp/test_logic
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "../spot_price_logic.h"

using namespace spot;

static int failures = 0;
#define CHECK(cond)                                                \
  do {                                                             \
    if (!(cond)) {                                                 \
      printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);       \
      failures++;                                                  \
    }                                                              \
  } while (0)

static bool near(float a, float b) { return std::fabs(a - b) < 1e-3f; }

int main() {
  // ISO parsing
  CHECK(parse_iso_utc("1970-01-01T00:00:00.000Z") == 0);
  CHECK(parse_iso_utc("2026-11-18T22:45:00.000Z") == 1795041900);
  CHECK(parse_iso_utc("2028-02-29T12:00:00Z") == 1835438400);
  CHECK(parse_iso_utc("garbage") == -1);
  CHECK(parse_iso_utc(nullptr) == -1);

  // Colour limits: green < 10 <= yellow < 20 <= red
  Settings s;
  CHECK(colour_for(s, 9.99f) == GREEN);
  CHECK(colour_for(s, 10.0f) == YELLOW);
  CHECK(colour_for(s, 19.99f) == YELLOW);
  CHECK(colour_for(s, 20.0f) == RED);
  CHECK(colour_for(s, -3.0f) == GREEN);

  // Total = spot + margin
  time_t d0 = parse_iso_utc("2026-11-18T00:00:00Z");
  Slot day_slot{d0 + 10 * 3600, 5.0f};
  CHECK(near(total_price(s, day_slot), 5.0f + s.margin));

  // Table: added newest first (as the API lists them), stored sorted, duplicates replaced
  PriceTable t;
  time_t base = d0 + 10 * 3600;  // total = spot + 0.49
  float spots[] = {15.0f, 15.0f, 20.0f, 8.0f, 9.0f};  // totals 15.49, 15.49, 20.49, 8.49, 9.49
  for (int i = 4; i >= 0; i--)
    CHECK(t.add(base + i * SLOT_SECONDS, spots[i]));
  CHECK(t.count == 5);
  for (int i = 1; i < t.count; i++)
    CHECK(t.slots[i - 1].start < t.slots[i].start);
  CHECK(t.add(base, 15.0f) && t.count == 5);
  CHECK(!t.add(-1, 1.0f));

  CHECK(t.index_at(base - 1) == -1);
  CHECK(t.index_at(base) == 0);
  CHECK(t.index_at(base + SLOT_SECONDS - 1) == 0);
  CHECK(t.index_at(base + 4 * SLOT_SECONDS + 899) == 4);
  CHECK(t.index_at(base + 5 * SLOT_SECONDS) == -1);
  CHECK(t.known_until() == base + 5 * SLOT_SECONDS);

  // Colour now, and grey when no price covers now
  CHECK(t.colour_now(s, base + 60) == YELLOW);
  CHECK(t.colour_now(s, base + 2 * SLOT_SECONDS) == RED);
  CHECK(t.colour_now(s, base + 3 * SLOT_SECONDS) == GREEN);
  CHECK(t.colour_now(s, base - 60) == GREY);
  CHECK(t.colour_now(s, base + 5 * SLOT_SECONDS) == GREY);
  CHECK(t.colour_now(s, 0) == GREY);
  PriceTable empty;
  CHECK(empty.colour_now(s, base) == GREY);

  // Next green: from the first slot -> slot 3; while in slot 3 -> now; after the last -> none
  CHECK(t.next_green_start(s, base + 10) == base + 3 * SLOT_SECONDS);
  CHECK(t.next_green_start(s, base + 3 * SLOT_SECONDS + 100) == base + 3 * SLOT_SECONDS + 100);
  CHECK(t.next_green_start(s, base + 5 * SLOT_SECONDS) == 0);
  s.green_below = 5.0f;  // nothing is green any more
  CHECK(t.next_green_start(s, base) == 0);
  s.green_below = 10.0f;

  // Fetch staleness
  CHECK(t.fetch_stale(base));  // never fetched
  t.last_fetch = base;
  CHECK(!t.fetch_stale(base + 6 * 3600));
  CHECK(t.fetch_stale(base + 6 * 3600 + 1));

  // Table full
  PriceTable full;
  for (int i = 0; i < MAX_SLOTS; i++)
    full.add(base + i * SLOT_SECONDS, 1.0f);
  CHECK(full.count == MAX_SLOTS);
  CHECK(!full.add(base + MAX_SLOTS * SLOT_SECONDS, 1.0f));

  if (failures) {
    printf("%d check(s) failed\n", failures);
    return 1;
  }
  printf("all tests passed\n");
  return 0;
}
