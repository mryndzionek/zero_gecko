#include <stdint.h>

#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_rtc.h"
#include "bsp.h"

#include "display.h"
#include "displaypal.h"
#include "retargettextdisplay.h"

#include "qpn_port.h"
#include "bsp_.h"

#include "image.h"
#include "display_test.h"

#define BYTES_PER_LINE        ( LS013B7DH03_WIDTH / 8 )
#define BYTES_PER_FRAME       ( LS013B7DH03_HEIGHT * BYTES_PER_LINE )

/* Variables used to display pictures on display */
static int numImages = sizeof (image_bits) / BYTES_PER_FRAME;
static int newImage = 1, oldImage = 0, step  = 0;

static volatile bool      displayEnabled = false; /* Status of LCD display. */
static DISPLAY_Device_t displayDevice;    /* Display device handle.         */

/* RTC callback parameters. */
static void (*rtcCallback)(void*) = 0;
static void * rtcCallbackArg         = 0;

volatile bool pb1Pressed;


void GPIO_ODD_IRQHandler(void)
{
    /* Acknowledge interrupt */
    GPIO_IntClear(1 << 9);
    pb1Pressed = true;
}

static void gpioSetup(void)
{
    /* Enable GPIO in CMU */
    CMU_ClockEnable(cmuClock_GPIO, true);

    /* Configure PC8 and PC9 as input */
    GPIO_PinModeSet(gpioPortC, 9, gpioModeInput, 0);

    /* Set falling edge interrupt for both ports */
    GPIO_IntConfig(gpioPortC, 9, false, true, true);

    /* Enable interrupt in core for even and odd gpio interrupts */
    NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);

    NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
    NVIC_EnableIRQ(GPIO_ODD_IRQn);

    return;
}

static void MemlcdScrollLeft(const char *pOldImg, const char *pNewImg, int step)
{
    int x, y;
    char line[BYTES_PER_LINE];
    const char *pNewLine, *pOldLine;


    /* Iterate over each line */
    for (y = 0; y < LS013B7DH03_HEIGHT; y++)
        {
            pNewLine = &pNewImg[BYTES_PER_LINE * y];
            pOldLine = &pOldImg[BYTES_PER_LINE * y];

            /* Iterate over each byte of the line */
            for (x = 0; x < BYTES_PER_LINE; x++)
                {
                    if (x < (BYTES_PER_LINE - step))
                        {
                            line[x] = pOldLine[x + step];
                        }
                    else
                        {
                            line[x] = pNewLine[x - (BYTES_PER_LINE - step)];
                        }
                }
            displayDevice.pPixelMatrixDraw( &displayDevice,
                                            line,
                                            /* start coloumn, width */
                                            0, displayDevice.geometry.width,
                                            /* start row, height */
                                            y, 1 );
        }
}

bool BSP_drawPicture(void)
{
    MemlcdScrollLeft(&image_bits[oldImage*BYTES_PER_FRAME], &image_bits[newImage*BYTES_PER_FRAME], step);

    ++step;
    if(step > BYTES_PER_LINE)
        {
            /* Output new image on Memory LCD */
            oldImage = newImage;

            if (++newImage >= numImages)
                {
                    newImage = 0;
                }

            step = 0;
            return true;
        } else
            return false;
}

/*..........................................................................*/
int RepeatCallbackRegister(void (*pFunction)(void*),
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

    if (pb1Pressed)
        {
            QACTIVE_POST_ISR((QActive *)&AO_Display, SWITCH_SIG,0);
            pb1Pressed = false;
        }
}

/*..........................................................................*/
void BSP_init(void) {

    char *pFrame;

    CHIP_Init();

    BSP_LedsInit();

    /* Initialize the display module. */
    displayEnabled = true;
    DISPLAY_Init();

    gpioSetup();

    /* Retrieve the properties of the display. */
    if ( DISPLAY_DeviceGet( 0, &displayDevice ) != DISPLAY_EMSTATUS_OK )
        {
            /* Unable to get display handle. */
            while( 1 );
        }

    /* Load pointer to picture buffor */
    pFrame= (char *) &image_bits[LS013B7DH03_WIDTH * BYTES_PER_LINE * oldImage];

    /* Write to LCD */
    displayDevice.pPixelMatrixDraw( &displayDevice, pFrame,
                                    /* start coloumn, width */
                                    0, displayDevice.geometry.width,
                                    /* start row, height */
                                    0, displayDevice.geometry.height);


}
/*..........................................................................*/
void QF_onStartup(void) {

    /* Enable HFXO as the main clock */
    CMU_ClockSelectSet( cmuClock_HF, cmuSelect_HFXO );

    RtcInit();

}
/*..........................................................................*/
void QF_onIdle(void) {        /* entered with interrupts LOCKED, see NOTE01 */

    BSP_LedSet(1);
    BSP_LedClear(1);
    EMU_EnterEM2(true);
    QF_INT_ENABLE();

}
/*..........................................................................*/
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {

    (void)file;                                   /* avoid compiler warning */
    (void)line;                                   /* avoid compiler warning */
    QF_INT_DISABLE();
    BSP_LedsSet(0xFF);                                       /* all LEDs on */

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
/*..........................................................................*/
