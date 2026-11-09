#include "config_io.h"

#include "config.h"

int nolockscreen_disarm_with_io(const NolockscreenConfigIo *io,
                                uint8_t *before, uint8_t *after,
                                size_t capacity) {
  size_t before_size = 0;
  size_t after_size = 0;
  size_t changed_offset = 0;
  size_t offset;

  if (io == NULL || io->read == NULL || io->patch == NULL ||
      io->sync == NULL || before == NULL || after == NULL || capacity == 0) {
    return -1;
  }

  if (io->read(io->context, before, capacity, &before_size) < 0 ||
      before_size > capacity) {
    return -1;
  }

  for (offset = 0; offset < before_size; offset++)
    after[offset] = before[offset];
  if (nolockscreen_disarm_config(after, before_size, &changed_offset) < 0)
    return -1;
  if (io->patch(io->context, changed_offset, 'u', '#') < 0)
    return -1;
  if (io->sync(io->context) < 0)
    return -1;
  if (io->read(io->context, after, capacity, &after_size) < 0 ||
      after_size != before_size) {
    return -1;
  }

  return nolockscreen_verify_disarmed_config(before, after, before_size,
                                              changed_offset);
}
