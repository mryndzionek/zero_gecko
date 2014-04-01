#ifndef bsp__h
#define bsp__h
#include <stdbool.h>

/* Sys timer tick per seconds */
#define BSP_TICKS_PER_SEC    (64)                                  /* 64 - for PAL driver */

void BSP_init(void);
void BSP_ledOn(void);
void BSP_ledOff(void);
bool BSP_drawPicture(void);

#endif                                                             /* bsp__h */

