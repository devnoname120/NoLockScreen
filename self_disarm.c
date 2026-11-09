#include "self_disarm.h"

#include "config.h"
#include "config_io.h"

#include <psp2/io/fcntl.h>

#define NOLOCKSCREEN_CONFIG_MAX_SIZE (64 * 1024)

typedef struct {
  const char *path;
  const char *mount;
} ConfigFile;

static uint8_t config_before[NOLOCKSCREEN_CONFIG_MAX_SIZE];
static uint8_t config_after[NOLOCKSCREEN_CONFIG_MAX_SIZE];

static int read_exact_config(void *context, uint8_t *buffer, size_t capacity,
                             size_t *size) {
  const ConfigFile *config = context;
  SceUID fd;
  SceOff length;
  size_t total = 0;
  uint8_t tail;
  int result = -1;

  fd = sceIoOpen(config->path, SCE_O_RDONLY, 0);
  if (fd < 0)
    return fd;

  length = sceIoLseek(fd, 0, SCE_SEEK_END);
  if (length < 0 || (uint64_t)length > capacity)
    goto done;
  if (sceIoLseek(fd, 0, SCE_SEEK_SET) != 0)
    goto done;

  while (total < (size_t)length) {
    int read_size = sceIoRead(fd, buffer + total,
                              (SceSize)((size_t)length - total));
    if (read_size <= 0)
      goto done;
    total += (size_t)read_size;
  }

  if (sceIoRead(fd, &tail, 1) != 0)
    goto done;

  *size = total;
  result = 0;

done:
  if (sceIoClose(fd) < 0)
    result = -1;
  return result;
}

static int patch_config_byte(void *context, size_t offset, uint8_t expected,
                             uint8_t replacement) {
  const ConfigFile *config = context;
  SceUID fd;
  uint8_t actual = 0;
  int result = -1;

  fd = sceIoOpen(config->path, SCE_O_RDWR, 0);
  if (fd < 0)
    return fd;

  if (sceIoLseek(fd, (SceOff)offset, SCE_SEEK_SET) != (SceOff)offset)
    goto done;
  if (sceIoRead(fd, &actual, 1) != 1 || actual != expected)
    goto done;
  if (sceIoLseek(fd, (SceOff)offset, SCE_SEEK_SET) != (SceOff)offset)
    goto done;
  if (sceIoWrite(fd, &replacement, 1) != 1)
    goto done;
  if (sceIoSyncByFd(fd, 0) < 0)
    goto done;

  result = 0;

done:
  if (sceIoClose(fd) < 0)
    result = -1;
  return result;
}

static int sync_config_mount(void *context) {
  const ConfigFile *config = context;

  return sceIoSync(config->mount, 0);
}

int nolockscreen_self_disarm(void) {
  static const ConfigFile config = {
    .path = NOLOCKSCREEN_CONFIG_PATH,
    .mount = "ur0:",
  };
  const NolockscreenConfigIo io = {
    .read = read_exact_config,
    .patch = patch_config_byte,
    .sync = sync_config_mount,
    .context = (void *)&config,
  };

  return nolockscreen_disarm_with_io(&io, config_before, config_after,
                                      sizeof(config_before));
}
