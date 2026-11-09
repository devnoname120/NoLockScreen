#ifndef NOLOCKSCREEN_CONFIG_H
#define NOLOCKSCREEN_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#define NOLOCKSCREEN_CONFIG_PATH "ur0:/tai/config.txt"
#define NOLOCKSCREEN_CONFIG_ENTRY "ur0:/tai/nolockscreen.suprx"

int nolockscreen_disarm_config(uint8_t *config, size_t size,
                               size_t *changed_offset);
int nolockscreen_verify_disarmed_config(const uint8_t *before,
                                        const uint8_t *after, size_t size,
                                        size_t changed_offset);

#endif
