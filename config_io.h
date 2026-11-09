#ifndef NOLOCKSCREEN_CONFIG_IO_H
#define NOLOCKSCREEN_CONFIG_IO_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  int (*read)(void *context, uint8_t *buffer, size_t capacity, size_t *size);
  int (*patch)(void *context, size_t offset, uint8_t expected,
               uint8_t replacement);
  int (*sync)(void *context);
  void *context;
} NolockscreenConfigIo;

int nolockscreen_disarm_with_io(const NolockscreenConfigIo *io,
                                uint8_t *before, uint8_t *after,
                                size_t capacity);

#endif
