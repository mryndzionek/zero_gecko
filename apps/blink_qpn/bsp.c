#include "em_device.h"
#include <stdint.h>
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_rtc.h"
#include "bsp.h"

#include "qpn_port.h"
#include "bsp_.h"

/* RTC callback parameters. */
static void (*rtcCallback)(void*) = 0;
static void * rtcCallbackArg         = 0;

/*..........................................................................*/
int RtcIntCallbackRegister(void (*pFunction)(void*),
                           void* argument,
                           unsigned int frequency)
{
  /* Verify that the requested frequency is the same as the RTC frequency.*/
  if (BSP_TICKS_PER_SEC != frequency)
    return -1;

  rtcCallback    = pFunction;
  rtcCallbackArg = argument;

  return 0;
}
/*..........................................................................*/
void RtcInit(void)
{
  RTC_Init_TypeDef rtcInit = RTC_INIT_DEFAULT;

  /* Enable LE domain registers */
  CMU_ClockEnable(cmuClock_CORELE, true);

  /* Enable LFXO as LFACLK in CMU. This will also start LFXO */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

  /* Enable RTC clock */
  CMU_ClockEnable(cmuClock_RTC, true);

  /* Initialize RTC */
  rtcInit.enable   = false;  /* Do not start RTC after initialization is complete. */
  rtcInit.debugRun = false;  /* Halt RTC when debugging. */
  rtcInit.comp0Top = true;   /* Wrap around on COMP0 match. */
  RTC_Init(&rtcInit);

  /* Interrupt at specified frequency. */
  RTC_CompareSet(0, (CMU_ClockFreqGet(cmuClock_RTC) /  BSP_TICKS_PER_SEC) - 1);

  /* Enable interrupt */
  NVIC_EnableIRQ(RTC_IRQn);
  RTC_IntEnable(RTC_IEN_COMP0);

  /* Start counter */
  RTC_Enable(true);
}

void RTC_IRQHandler(void)
{
  RTC_IntClear(RTC_IF_COMP0);

  /* Execute callback function if registered. */
  if (rtcCallback)
    (*rtcCallback)(rtcCallbackArg);

  QF_tickXISR(0U);
}

/*..........................................................................*/
void BSP_init(void) {

  CHIP_Init();

  BSP_LedsInit();

}
/*..........................................................................*/
void QF_onStartup(void) {

  RtcInit();

}
/*..........................................................................*/
void QF_onIdle(void) {        /* entered with interrupts LOCKED, see NOTE01 */

    BSP_LedSet(1);
    BSP_LedClear(1);
    EMU_EnterEM3(true);
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
