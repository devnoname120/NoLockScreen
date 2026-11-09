#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../config.h"

static void test_disarms_unique_lf_line(void) {
  static const char original[] =
    "*main\n"
    "ur0:/tai/nolockscreen.suprx\n"
    "ur0:tai/other.suprx\n";
  static const char expected[] =
    "*main\n"
    "#r0:/tai/nolockscreen.suprx\n"
    "ur0:tai/other.suprx\n";
  uint8_t config[sizeof(original)];
  size_t offset = SIZE_MAX;

  memcpy(config, original, sizeof(config));
  assert(nolockscreen_disarm_config(config, sizeof(config) - 1, &offset) == 0);
  assert(offset == 6);
  assert(memcmp(config, expected, sizeof(expected)) == 0);
}

static void test_disarms_crlf_line(void) {
  static const char original[] =
    "*main\r\n"
    "ur0:/tai/nolockscreen.suprx\r\n";
  uint8_t config[sizeof(original)];
  size_t offset = SIZE_MAX;

  memcpy(config, original, sizeof(config));
  assert(nolockscreen_disarm_config(config, sizeof(config) - 1, &offset) == 0);
  assert(offset == 7);
  assert(config[offset] == '#');
  assert(config[offset + 1] == 'r');
}

static void test_disarms_line_at_eof(void) {
  static const char original[] =
    "*main\n"
    "ur0:/tai/nolockscreen.suprx";
  uint8_t config[sizeof(original)];
  size_t offset = SIZE_MAX;

  memcpy(config, original, sizeof(config));
  assert(nolockscreen_disarm_config(config, sizeof(config) - 1, &offset) == 0);
  assert(offset == 6);
  assert(config[offset] == '#');
}

static void test_rejects_missing_or_non_exact_line(void) {
  static const char missing[] = "*main\nur0:tai/nolockscreen.suprx\n";
  static const char suffix[] = "ur0:/tai/nolockscreen.suprx.backup\n";
  static const char prefixed[] = "xur0:/tai/nolockscreen.suprx\n";
  uint8_t config[64];
  size_t offset = 0;

  memcpy(config, missing, sizeof(missing));
  assert(nolockscreen_disarm_config(config, sizeof(missing) - 1, &offset) < 0);
  assert(memcmp(config, missing, sizeof(missing)) == 0);

  memcpy(config, suffix, sizeof(suffix));
  assert(nolockscreen_disarm_config(config, sizeof(suffix) - 1, &offset) < 0);
  assert(memcmp(config, suffix, sizeof(suffix)) == 0);

  memcpy(config, prefixed, sizeof(prefixed));
  assert(nolockscreen_disarm_config(config, sizeof(prefixed) - 1, &offset) < 0);
  assert(memcmp(config, prefixed, sizeof(prefixed)) == 0);
}

static void test_rejects_duplicate_without_mutating(void) {
  static const char original[] =
    "ur0:/tai/nolockscreen.suprx\n"
    "ur0:/tai/nolockscreen.suprx\n";
  uint8_t config[sizeof(original)];
  size_t offset = 0;

  memcpy(config, original, sizeof(config));
  assert(nolockscreen_disarm_config(config, sizeof(config) - 1, &offset) < 0);
  assert(memcmp(config, original, sizeof(original)) == 0);
}

static void test_verifies_exact_one_byte_change(void) {
  static const uint8_t before[] =
    "*main\n"
    "ur0:/tai/nolockscreen.suprx\n";
  uint8_t after[sizeof(before)];
  size_t offset = 6;

  memcpy(after, before, sizeof(after));
  after[offset] = '#';
  assert(nolockscreen_verify_disarmed_config(before, after,
                                              sizeof(before) - 1, offset) == 0);

  after[offset + 1] = 'x';
  assert(nolockscreen_verify_disarmed_config(before, after,
                                              sizeof(before) - 1, offset) < 0);
}

static void test_rejects_invalid_arguments(void) {
  uint8_t config[] = "ur0:/tai/nolockscreen.suprx\n";
  size_t offset = 0;

  assert(nolockscreen_disarm_config(NULL, sizeof(config) - 1, &offset) < 0);
  assert(nolockscreen_disarm_config(config, sizeof(config) - 1, NULL) < 0);
  assert(nolockscreen_verify_disarmed_config(NULL, config,
                                              sizeof(config) - 1, 0) < 0);
  assert(nolockscreen_verify_disarmed_config(config, NULL,
                                              sizeof(config) - 1, 0) < 0);
}

int main(void) {
  test_disarms_unique_lf_line();
  test_disarms_crlf_line();
  test_disarms_line_at_eof();
  test_rejects_missing_or_non_exact_line();
  test_rejects_duplicate_without_mutating();
  test_verifies_exact_one_byte_change();
  test_rejects_invalid_arguments();
  return 0;
}
