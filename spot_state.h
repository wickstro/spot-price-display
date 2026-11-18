// Firmware state for ESPHome (included after spot_price_logic.h). ESPHome emits globals before `includes`,
// so the price tables live here instead of in a `globals:` entry.
#pragma once
#include "spot_price_logic.h"

namespace spot {
inline PriceTable table_a, table_b;
inline PriceTable *current = &table_a;  // what the display reads
inline PriceTable *spare = &table_b;    // filled by a fetch, then swapped in
}  // namespace spot
