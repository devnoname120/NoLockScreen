#include "config.h"

static int matches_active_entry(const uint8_t *config, size_t size,
                                size_t offset) {
  static const uint8_t entry[] = NOLOCKSCREEN_CONFIG_ENTRY;
  const size_t entry_size = sizeof(entry) - 1;
  const size_t end = offset + entry_size;
  size_t index;

  if (offset >= size || (offset != 0 && config[offset - 1] != '\n'))
    return 0;
  if (entry_size > size - offset)
    return 0;
  for (index = 0; index < entry_size; index++) {
    if (config[offset + index] != entry[index])
      return 0;
  }
  if (end == size || config[end] == '\n')
    return 1;
  return config[end] == '\r' && end + 1 < size && config[end + 1] == '\n';
}

int nolockscreen_disarm_config(uint8_t *config, size_t size,
                               size_t *changed_offset) {
  size_t found = 0;
  unsigned int count = 0;
  size_t offset;

  if (config == NULL || changed_offset == NULL)
    return -1;

  for (offset = 0; offset < size; offset++) {
    if (!matches_active_entry(config, size, offset))
      continue;

    found = offset;
    count++;
    if (count > 1)
      return -1;
  }

  if (count != 1)
    return -1;

  config[found] = '#';
  *changed_offset = found;
  return 0;
}

int nolockscreen_verify_disarmed_config(const uint8_t *before,
                                        const uint8_t *after, size_t size,
                                        size_t changed_offset) {
  size_t offset;

  if (before == NULL || after == NULL || changed_offset >= size)
    return -1;
  if (!matches_active_entry(before, size, changed_offset))
    return -1;
  if (before[changed_offset] != 'u' || after[changed_offset] != '#')
    return -1;

  for (offset = 0; offset < size; offset++) {
    if (offset != changed_offset && before[offset] != after[offset])
      return -1;
  }

  return 0;
}
