#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../config_io.h"

typedef struct {
  uint8_t data[256];
  size_t size;
  int read_count;
  int patch_count;
  int sync_count;
  int patch_result;
  int sync_result;
  int corrupt_reread;
  int shrink_reread;
  char order[16];
  size_t order_size;
} FakeConfig;

static void record(FakeConfig *config, char operation) {
  config->order[config->order_size++] = operation;
  config->order[config->order_size] = '\0';
}

static int fake_read(void *context, uint8_t *buffer, size_t capacity,
                     size_t *size) {
  FakeConfig *config = context;

  record(config, 'r');
  if (config->size > capacity)
    return -1;
  memcpy(buffer, config->data, config->size);
  *size = config->size;
  if (config->read_count > 0 && config->corrupt_reread && *size > 0)
    buffer[*size - 1] ^= 1;
  if (config->read_count > 0 && config->shrink_reread && *size > 0)
    (*size)--;
  config->read_count++;
  return 0;
}

static int fake_patch(void *context, size_t offset, uint8_t expected,
                      uint8_t replacement) {
  FakeConfig *config = context;

  record(config, 'p');
  config->patch_count++;
  if (config->patch_result < 0)
    return config->patch_result;
  if (offset >= config->size || config->data[offset] != expected)
    return -1;
  config->data[offset] = replacement;
  return 0;
}

static int fake_sync(void *context) {
  FakeConfig *config = context;

  record(config, 's');
  config->sync_count++;
  return config->sync_result;
}

static NolockscreenConfigIo fake_io(FakeConfig *config) {
  NolockscreenConfigIo io = {
    .read = fake_read,
    .patch = fake_patch,
    .sync = fake_sync,
    .context = config,
  };
  return io;
}

static FakeConfig fake_config(const char *text) {
  FakeConfig config = {0};

  config.size = strlen(text);
  memcpy(config.data, text, config.size);
  return config;
}

static void test_disarms_then_syncs_and_verifies(void) {
  FakeConfig config = fake_config(
    "*main\n"
    "ur0:/tai/nolockscreen.suprx\n"
    "ur0:tai/other.suprx\n");
  NolockscreenConfigIo io = fake_io(&config);
  uint8_t before[256];
  uint8_t after[256];

  assert(nolockscreen_disarm_with_io(&io, before, after, sizeof(before)) == 0);
  assert(strcmp(config.order, "rpsr") == 0);
  assert(config.patch_count == 1);
  assert(config.sync_count == 1);
  assert(memcmp(config.data + 6, "#r0:/tai/nolockscreen.suprx",
                sizeof("#r0:/tai/nolockscreen.suprx") - 1) == 0);
}

static void test_rejects_missing_entry_before_writing(void) {
  FakeConfig config = fake_config("*main\nur0:tai/other.suprx\n");
  NolockscreenConfigIo io = fake_io(&config);
  uint8_t before[256];
  uint8_t after[256];

  assert(nolockscreen_disarm_with_io(&io, before, after, sizeof(before)) < 0);
  assert(strcmp(config.order, "r") == 0);
  assert(config.patch_count == 0);
  assert(config.sync_count == 0);
}

static void test_stops_after_patch_failure(void) {
  FakeConfig config = fake_config("ur0:/tai/nolockscreen.suprx\n");
  NolockscreenConfigIo io = fake_io(&config);
  uint8_t before[256];
  uint8_t after[256];

  config.patch_result = -77;
  assert(nolockscreen_disarm_with_io(&io, before, after, sizeof(before)) < 0);
  assert(strcmp(config.order, "rp") == 0);
  assert(config.sync_count == 0);
}

static void test_stops_after_sync_failure(void) {
  FakeConfig config = fake_config("ur0:/tai/nolockscreen.suprx\n");
  NolockscreenConfigIo io = fake_io(&config);
  uint8_t before[256];
  uint8_t after[256];

  config.sync_result = -88;
  assert(nolockscreen_disarm_with_io(&io, before, after, sizeof(before)) < 0);
  assert(strcmp(config.order, "rps") == 0);
}

static void test_rejects_unexpected_reread_change(void) {
  FakeConfig config = fake_config(
    "ur0:/tai/nolockscreen.suprx\n"
    "ur0:tai/other.suprx\n");
  NolockscreenConfigIo io = fake_io(&config);
  uint8_t before[256];
  uint8_t after[256];

  config.corrupt_reread = 1;
  assert(nolockscreen_disarm_with_io(&io, before, after, sizeof(before)) < 0);
  assert(strcmp(config.order, "rpsr") == 0);
}

static void test_rejects_changed_file_size(void) {
  FakeConfig config = fake_config("ur0:/tai/nolockscreen.suprx\n");
  NolockscreenConfigIo io = fake_io(&config);
  uint8_t before[256];
  uint8_t after[256];

  config.shrink_reread = 1;
  assert(nolockscreen_disarm_with_io(&io, before, after, sizeof(before)) < 0);
  assert(strcmp(config.order, "rpsr") == 0);
}

static void test_rejects_invalid_arguments(void) {
  FakeConfig config = fake_config("ur0:/tai/nolockscreen.suprx\n");
  NolockscreenConfigIo io = fake_io(&config);
  uint8_t before[256];
  uint8_t after[256];

  assert(nolockscreen_disarm_with_io(NULL, before, after, sizeof(before)) < 0);
  assert(nolockscreen_disarm_with_io(&io, NULL, after, sizeof(before)) < 0);
  assert(nolockscreen_disarm_with_io(&io, before, NULL, sizeof(before)) < 0);
  assert(nolockscreen_disarm_with_io(&io, before, after, 0) < 0);

  io.read = NULL;
  assert(nolockscreen_disarm_with_io(&io, before, after, sizeof(before)) < 0);
}

int main(void) {
  test_disarms_then_syncs_and_verifies();
  test_rejects_missing_entry_before_writing();
  test_stops_after_patch_failure();
  test_stops_after_sync_failure();
  test_rejects_unexpected_reread_change();
  test_rejects_changed_file_size();
  test_rejects_invalid_arguments();
  return 0;
}
