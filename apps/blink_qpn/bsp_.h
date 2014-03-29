#ifndef bsp__h
#define bsp__h

/* Sys timer tick per seconds */
#define BSP_TICKS_PER_SEC    10

void BSP_init(void);
void BSP_ledOn(void);
void BSP_ledOff(void);

#define BSP_showState(state_) ((void)0)

#endif                                                             /* bsp__h */

