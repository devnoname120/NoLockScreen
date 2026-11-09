#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "../pattern.h"

static const uint8_t first_site[] = {
  0x10, 0xF0, 0x7F, 0x05,
  0x23, 0xF2, 0x98, 0xEF,
  0x01, 0x28, 0x14, 0xD0,
};

static const uint8_t second_site[] = {
  0x00, 0x28, 0x0B, 0xD1,
  0x23, 0xF2, 0x1A, 0xE9,
  0x01, 0x28, 0x09, 0xD1,
};

static void place(uint8_t *text, size_t offset, const uint8_t *site, size_t size) {
  memcpy(text + offset, site, size);
}

static void test_finds_both_call_sites(void) {
  uint8_t text[256] = {0};
  uint32_t offsets[NOLOCKSCREEN_PATCH_COUNT] = {0};

  place(text, 0x20, first_site, sizeof(first_site));
  place(text, 0x80, second_site, sizeof(second_site));

  assert(nolockscreen_find_patch_offsets(text, sizeof(text), offsets) == 0);
  assert(offsets[0] == 0x24);
  assert(offsets[1] == 0x84);
}

static void test_rejects_missing_site(void) {
  uint8_t text[256] = {0};
  uint32_t offsets[NOLOCKSCREEN_PATCH_COUNT] = {0};

  place(text, 0x20, first_site, sizeof(first_site));

  assert(nolockscreen_find_patch_offsets(text, sizeof(text), offsets) < 0);
}

static void test_rejects_duplicate_site(void) {
  uint8_t text[256] = {0};
  uint32_t offsets[NOLOCKSCREEN_PATCH_COUNT] = {0};

  place(text, 0x20, first_site, sizeof(first_site));
  place(text, 0x40, first_site, sizeof(first_site));
  place(text, 0x80, second_site, sizeof(second_site));

  assert(nolockscreen_find_patch_offsets(text, sizeof(text), offsets) < 0);
}

static void test_rejects_non_blx_candidate(void) {
  uint8_t text[256] = {0};
  uint32_t offsets[NOLOCKSCREEN_PATCH_COUNT] = {0};
  uint8_t invalid_first[sizeof(first_site)];

  memcpy(invalid_first, first_site, sizeof(first_site));
  invalid_first[4] = 0x01;
  invalid_first[5] = 0x20;
  invalid_first[6] = 0x00;
  invalid_first[7] = 0xBF;

  place(text, 0x20, invalid_first, sizeof(invalid_first));
  place(text, 0x80, second_site, sizeof(second_site));

  assert(nolockscreen_find_patch_offsets(text, sizeof(text), offsets) < 0);
}

static void test_rejects_unaligned_candidate(void) {
  uint8_t text[256] = {0};
  uint32_t offsets[NOLOCKSCREEN_PATCH_COUNT] = {0};

  place(text, 0x21, first_site, sizeof(first_site));
  place(text, 0x80, second_site, sizeof(second_site));

  assert(nolockscreen_find_patch_offsets(text, sizeof(text), offsets) < 0);
}

int main(void) {
  test_finds_both_call_sites();
  test_rejects_missing_site();
  test_rejects_duplicate_site();
  test_rejects_non_blx_candidate();
  test_rejects_unaligned_candidate();
  return 0;
}
