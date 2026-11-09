#include "pattern.h"

#include <limits.h>

#define SITE_SIZE 12
#define CALL_OFFSET 4

typedef struct {
  uint8_t value[SITE_SIZE];
  uint8_t mask[SITE_SIZE];
} SitePattern;

static const SitePattern site_patterns[NOLOCKSCREEN_PATCH_COUNT] = {
  {
    {0x10, 0xF0, 0x7F, 0x05, 0, 0, 0, 0, 0x01, 0x28, 0, 0xD0},
    {0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0xFF, 0xFF, 0, 0xFF},
  },
  {
    {0x00, 0x28, 0x0B, 0xD1, 0, 0, 0, 0, 0x01, 0x28, 0x09, 0xD1},
    {0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF},
  },
};

static int is_thumb_blx(const uint8_t *code) {
  uint16_t first = (uint16_t)code[0] | ((uint16_t)code[1] << 8);
  uint16_t second = (uint16_t)code[2] | ((uint16_t)code[3] << 8);

  return (first & 0xF800) == 0xF000 && (second & 0xD001) == 0xC000;
}

static int matches(const uint8_t *data, const SitePattern *pattern) {
  size_t i;

  for (i = 0; i < SITE_SIZE; i++) {
    if ((data[i] & pattern->mask[i]) != pattern->value[i])
      return 0;
  }

  return is_thumb_blx(data + CALL_OFFSET);
}

static int find_unique(const uint8_t *text, size_t text_size,
                       const SitePattern *pattern, uint32_t *offset) {
  size_t i;
  size_t found = 0;
  unsigned int count = 0;

  for (i = 0; i + SITE_SIZE <= text_size; i += 2) {
    if (!matches(text + i, pattern))
      continue;

    found = i + CALL_OFFSET;
    count++;
    if (count > 1)
      return -1;
  }

  if (count != 1 || found > UINT32_MAX)
    return -1;

  *offset = (uint32_t)found;
  return 0;
}

int nolockscreen_find_patch_offsets(const uint8_t *text, size_t text_size,
                                    uint32_t offsets[NOLOCKSCREEN_PATCH_COUNT]) {
  uint32_t found[NOLOCKSCREEN_PATCH_COUNT];
  size_t i;

  if (text == NULL || offsets == NULL)
    return -1;

  for (i = 0; i < NOLOCKSCREEN_PATCH_COUNT; i++) {
    if (find_unique(text, text_size, &site_patterns[i], &found[i]) < 0)
      return -1;
  }

  for (i = 0; i < NOLOCKSCREEN_PATCH_COUNT; i++)
    offsets[i] = found[i];

  return 0;
}
