#include "em_device.h"
#include <stdint.h>
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "bsp.h"

#include "qpn_port.h"
#include "bsp_.h"

/*..........................................................................*/
void SysTick_Handler(void) __attribute__((__interrupt__));
void SysTick_Handler(void)
{
    QF_tickXISR(0U);
}
/*..........................................................................*/
void BSP_init(void) {

  CHIP_Init();

  BSP_LedsInit();

}
/*..........................................................................*/
void QF_onStartup(void) {

  /* Setup SysTick Timer for 1 msec interrupts  */
  SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / BSP_TICKS_PER_SEC);

}
/*..........................................................................*/
void QF_onIdle(void) {        /* entered with interrupts LOCKED, see NOTE01 */

    BSP_LedSet(1);
    BSP_LedClear(1);
    EMU_EnterEM1();
    QF_INT_ENABLE();

}
/*..........................................................................*/
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {

    (void)file;                                   /* avoid compiler warning */
    (void)line;                                   /* avoid compiler warning */
    QF_INT_DISABLE();
    BSP_LedsSet(0xFF);                                            /* all LEDs on */

    for (;;) {       /* NOTE: replace the loop with reset for final version */
    }
}
/*..........................................................................*/
void BSP_ledOn(void) {
    BSP_LedSet(0);
}
/*..........................................................................*/
void BSP_ledOff(void) {
    BSP_LedClear(0);
}
