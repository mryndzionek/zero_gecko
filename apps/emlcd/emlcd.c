/**************************************************************************//**
 * @file
 * @brief Energy Mode demo for EFM32ZG_STK3200
 * @brief Demo for energy mode current consumption testing.
 * @author Energy Micro AS
 * @version 3.20.3
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2013 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#include <stdio.h>

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_rtc.h"

#include "display.h"
#include "textdisplay.h"
#include "retargettextdisplay.h"

/* Frequency of RTC clock. */
#define RTC_FREQUENCY   (64)
#define SLEEP_TIME      (1)

/* RTC callback parameters. */
static void (*rtcCallback)(void*) = NULL;
static void*  rtcCallbackArg      = 0;

static volatile bool      displayEnabled = false; /* Status of LCD display. */
static volatile bool      enterEM4 = false;
static volatile uint32_t  seconds = 0;     /* Seconds elapsed since reset.  */
static volatile int       rtcIrqCount = 0; /* RTC interrupt counter         */

static DISPLAY_Device_t displayDevice;    /* Display device handle.         */

static void CheckEM4Entry( void );
static void EnterEMode( int mode, uint32_t secs );
static void GpioSetup( void );
static void RtcInit( void );

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main( void )
{
  /* Chip errata */
  CHIP_Init();

  /* Setup GPIO for pushbuttons. */
  GpioSetup();

  /* Initialize the display module. */
  displayEnabled = true;
  DISPLAY_Init();

  /* Retrieve the properties of the display. */
  if ( DISPLAY_DeviceGet( 0, &displayDevice ) != DISPLAY_EMSTATUS_OK )
  {
    /* Unable to get display handle. */
    while( 1 );
  }

  /* Retarget stdio to the display. */
  if ( TEXTDISPLAY_EMSTATUS_OK != RETARGET_TextDisplayInit() )
  {
    /* Text display initialization failed. */
    while( 1 );
  }

  /* Set RTC to generate an interrupt every second. */
  RtcInit();

  printf( "\n\n Cycling through"
          "\n energy modes"
          "\n EM0-EM3"
          "\n\n Push PB0 to"
          "\n enter EM4\n\n\n\n" );

  /* Turn on LFXO to be able to see the difference between EM2 and EM3. */
  CMU_OscillatorEnable(cmuOsc_LFXO, true, false );

  for (;;)
  {
    printf( "\r      EM0" );
    EnterEMode( 0, SLEEP_TIME );
    CheckEM4Entry();

    printf( "\r      EM1" );
    EnterEMode( 1, SLEEP_TIME );
    CheckEM4Entry();

    printf( "\r      EM2" );
    EnterEMode( 2, SLEEP_TIME );
    CheckEM4Entry();

    printf( "\r      EM3" );
    EnterEMode( 3, SLEEP_TIME );
    CheckEM4Entry();
  }
}

/**************************************************************************//**
 * @brief  Check if PB0 is pushed and EM4 entry is due.
 *****************************************************************************/
static void CheckEM4Entry( void )
{
  if ( enterEM4 )
  {
    enterEM4 = false;

    printf( "\f\n\n Ready to enter"
            "\n energy mode"
            "\n EM4"
            "\n\n Push PB0 to"
            "\n enter EM4"
            "\n\n Wakeup from"
            "\n EM4 by pushing"
            "\n PB1 or the"
            "\n reset button" );

    while ( enterEM4 == false );
    enterEM4 = false;

    /* Disable the RTC. */
    printf( "\f" );
    displayEnabled = false;
    NVIC_DisableIRQ( RTC_IRQn );

    /* Power down the display. */
    displayDevice.pDisplayPowerOn( &displayDevice, false );

    /* Enable wakeup from EM4 on PB1 GPIO pin. */
    GPIO_EM4EnablePinWakeup( GPIO_EM4WUEN_EM4WUEN_C9, 0 );
    EMU_EnterEM4();
  }
}

/**************************************************************************//**
 * @brief   Enter a Energy Mode for a given number of seconds.
 *
 * @param[in] mode  Energy Mode to enter (0..3).
 * @param[in] secs  Time to stay in Energy Mode <mode>.
 *****************************************************************************/
static void EnterEMode( int mode, uint32_t secs )
{
  if ( secs )
  {
    uint32_t startTime = seconds;

    if ( mode == 0 )
    {
      int cnt = 0;

      while ((seconds - startTime) < secs)
      {
        if      ( cnt == 0 ) printf( "\r  - - EM0 - -" );
        else if ( cnt == 1 ) printf( "\r  \\ \\ EM0 / /" );
        else if ( cnt == 2 ) printf( "\r  | | EM0 | |" );
        else if ( cnt == 3 ) printf( "\r  / / EM0 \\ \\" );
        cnt = (cnt + 1) % 4;
        if ( enterEM4 )
        {
          printf( "\r      EM0    " );
          return;
        }
      }
      printf( "\r      EM0    " );
    }
    else
    {
      while ((seconds - startTime) < secs)
      {
        switch ( mode )
        {
          case 1: EMU_EnterEM1(); break;
          case 2: EMU_EnterEM2( true ); break;
          case 3: EMU_EnterEM3( true ); break;
        }
        if ( enterEM4 )
        {
          return;
        }
      }
    }
  }
}

/**************************************************************************//**
 * @brief Setup GPIO interrupt for pushbuttons.
 *****************************************************************************/
static void GpioSetup( void )
{
  /* Enable GPIO clock. */
  CMU_ClockEnable( cmuClock_GPIO, true );

  /* Configure PC8 as input and enable interrupt. */
  GPIO_PinModeSet( gpioPortC, 8, gpioModeInputPull, 1 );
  GPIO_IntConfig( gpioPortC, 8, false, true, true );

  NVIC_ClearPendingIRQ( GPIO_EVEN_IRQn );
  NVIC_EnableIRQ( GPIO_EVEN_IRQn );

  /* Configure PC9 as input. */
  GPIO_PinModeSet( gpioPortC, 9, gpioModeInputPull, 1 );
}

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB0)
 *        Sets next energy mode test number.
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  /* Acknowledge interrupt */
  GPIO_IntClear( 1 << 8 );

  enterEM4 = true;
}

/**************************************************************************//**
 * @brief   Set up RTC to generate an interrupt every second.
 *****************************************************************************/
static void RtcInit(void)
{
  RTC_Init_TypeDef rtcInit = RTC_INIT_DEFAULT;

  /* Enable LE domain registers */
  CMU_ClockEnable( cmuClock_CORELE, true );

  /* Enable ULFRCO as LFACLK in CMU. */
  CMU_ClockSelectSet( cmuClock_LFA, cmuSelect_ULFRCO );

  /* Set the prescaler. */
  CMU_ClockDivSet( cmuClock_RTC, cmuClkDiv_1 );

  /* Enable RTC clock */
  CMU_ClockEnable( cmuClock_RTC, true );

  /* Initialize RTC */
  rtcInit.enable   = false;  /* Do not start RTC after initialization is complete. */
  rtcInit.debugRun = false;  /* Halt RTC when debugging. */
  rtcInit.comp0Top = true;   /* Wrap around on COMP0 match. */
  RTC_Init( &rtcInit );

  /* Interrupt at specified frequency. */
  RTC_CompareSet( 0, (SystemULFRCOClockGet() / RTC_FREQUENCY) - 1 );

  /* Enable interrupt */
  NVIC_EnableIRQ( RTC_IRQn );
  RTC_IntEnable( RTC_IEN_COMP0 );

  /* Start counter */
  RTC_Enable( true );
}


/**************************************************************************//**
 * @brief   Register a callback function at the given frequency.
 *
 * @param[in] pFunction  Pointer to function that should be called at the
 *                       given frequency.
 * @param[in] argument   Argument to be given to the function.
 * @param[in] frequency  Frequency at which to call function at.
 *
 * @return  0 for successful or
 *         -1 if the requested frequency does not match the RTC frequency.
 *****************************************************************************/
int RtcIntCallbackRegister (void(*pFunction)(void*),
                            void* argument,
                            unsigned int frequency)
{
  /* Verify that the requested frequency is the same as the RTC frequency.*/
  if ( RTC_FREQUENCY != frequency )
    return -1;

  rtcCallback    = pFunction;
  rtcCallbackArg = argument;

  return 0;
}

/**************************************************************************//**
 * @brief   This interrupt is triggered every second by the RTC.
 *****************************************************************************/
void RTC_IRQHandler( void )
{
  RTC_IntClear( RTC_IF_COMP0 );

  /* Execute callback function if registered. */
  if ( rtcCallback && displayEnabled )
    rtcCallback( rtcCallbackArg );

  if (RTC_FREQUENCY == ++rtcIrqCount)
  {
    /* One second reached, reset irqCount and increment second counter. */
    rtcIrqCount = 0;
    
    seconds++;
  }
}
