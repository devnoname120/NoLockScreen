/*
  VitaTweaks: NoLockScreen
  Copyright (C) 2018, TheFloW

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <psp2/kernel/modulemgr.h>
#include <taihen.h>

#include "pattern.h"

#ifdef NOLOCKSCREEN_SELF_DISARM
#include "self_disarm.h"
#endif

static SceUID hooks[NOLOCKSCREEN_PATCH_COUNT] = {-1, -1};

static void release_hooks(void) {
  int i;

  for (i = NOLOCKSCREEN_PATCH_COUNT - 1; i >= 0; i--) {
    if (hooks[i] >= 0) {
      taiInjectRelease(hooks[i]);
      hooks[i] = -1;
    }
  }
}

int module_start(SceSize args, void *argp);
int _start(SceSize args, void *argp)
  __attribute__ ((weak, alias("module_start")));

int module_start(SceSize args, void *argp) {
  tai_module_info_t info;
  SceKernelModuleInfo module_info;
  uint32_t offsets[NOLOCKSCREEN_PATCH_COUNT];
  static const uint8_t force_true[] = {0x01, 0x20, 0x00, 0xBF};
  int ret;
  int i;

  (void)args;
  (void)argp;

#ifdef NOLOCKSCREEN_SELF_DISARM
  if (nolockscreen_self_disarm() < 0)
    return SCE_KERNEL_START_SUCCESS;
#endif

  info.size = sizeof(info);
  ret = taiGetModuleInfo("SceShell", &info);
  if (ret < 0)
    return SCE_KERNEL_START_SUCCESS;

  module_info.size = sizeof(module_info);
  ret = sceKernelGetModuleInfo(info.modid, &module_info);
  if (ret < 0 || module_info.segments[0].vaddr == NULL ||
      module_info.segments[0].filesz == 0) {
    return SCE_KERNEL_START_SUCCESS;
  }

  ret = nolockscreen_find_patch_offsets(module_info.segments[0].vaddr,
                                         module_info.segments[0].filesz,
                                         offsets);
  if (ret < 0)
    return SCE_KERNEL_START_SUCCESS;

  for (i = 0; i < NOLOCKSCREEN_PATCH_COUNT; i++) {
    hooks[i] = taiInjectData(info.modid, 0, offsets[i], force_true,
                             sizeof(force_true));
    if (hooks[i] < 0) {
      release_hooks();
      break;
    }
  }

  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
  (void)args;
  (void)argp;

  release_hooks();
  return SCE_KERNEL_STOP_SUCCESS;
}
