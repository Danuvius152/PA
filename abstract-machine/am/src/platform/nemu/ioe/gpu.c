#include <am.h>
#include <nemu.h>
#include <klib.h>
#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  int i;
  int w = 0;  // TODO: get the correct width
  int h = 0;  // TODO: get the correct height
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i ++) fb[i] = i;
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  int tmp = inl(VGACTL_ADDR);
  int width = (tmp >> 16), height = (tmp & 0xFFFF);
  *cfg = (AM_GPU_CONFIG_T){
      .present = true, .has_accel = false, .width = width, .height = height, .vmemsz = width * height
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
  AM_GPU_CONFIG_T info = io_read(AM_GPU_CONFIG);
  int w = info.width;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t* pixels = ctl->pixels;
  uint32_t copy_size = ctl->w *sizeof(uint32_t);
  for (int i = 0; i < ctl->h; ++i){
    memcpy(&fb[ctl->x + (ctl->y + i) * w], pixels, copy_size);
    pixels += ctl->w;
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}