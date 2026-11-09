#ifndef NOLOCKSCREEN_PATTERN_H
#define NOLOCKSCREEN_PATTERN_H

#include <stddef.h>
#include <stdint.h>

#define NOLOCKSCREEN_PATCH_COUNT 2

int nolockscreen_find_patch_offsets(const uint8_t *text, size_t text_size,
                                    uint32_t offsets[NOLOCKSCREEN_PATCH_COUNT]);

#endif
