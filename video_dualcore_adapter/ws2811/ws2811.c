#include "board.h"
#include "embedul.ar/source/core/cc.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/video_dualcore_adapter/adapter.h"
#include "embedul.ar/source/arch/arm-cortex/m0_instdelay.h"
#include <stdnoreturn.h>


#define WS2811_FRAME_RATE           60
#define WS2811_CH1_DOUT             IO_STACKPORT_IO7
//#define WS2811_CH2_DOUT             IO_STACKPORT_IO4
#define WS2811_CH1_COMP             936     // 312 * 3 = 936 (RGB)
#define WS2811_DESCRIPTION          "WS2811 LED strip"
#define WS2811_MODELINE             CC_ExpStr(WS2811_CH1_COMP) " leds @ " \
                                    CC_ExpStr(WS2811_FRAME_RATE) " hz"
// Hand-tuned M0_instDelay()
// Zero
#define WS2811_T0H_ID               30u     // 30u              // 250 ns
#define WS2811_T0L_ID               150u    // 190u             // 1000 ns
// One
#define WS2811_T1H_ID               150u    // 105u             // 600 ns
#define WS2811_T1L_ID               150u    // 115u             // 650 ns
// Intercomponent delay
#define WS2811_IC_ID                0u      // 1000u            // ~5000 ns
// Reset
#define WS2811_RET_ID               70000u  // 50200u //10200u  // 50000 ns


// Initial status
static uint8_t      * s_vFramebuffer    = (uint8_t *) g_framebufferA;
static uint32_t     s_vFrameCount       = 0;
static uint32_t     s_vFrameComp        = 0;
static uint32_t     s_vFrameData        = 0;


static const uint32_t s_VIcWaitH[2] = { WS2811_T0H_ID, WS2811_T1H_ID };
static const uint32_t s_VIcWaitL[2] = { WS2811_T0L_ID, WS2811_T1L_ID };


void HardFault_Handler (void)
{
    while (1)
    {
        BOARD_LED_TOGGLE (M0_HARDFAULT);
        for (uint32_t i = 0; i < 9000000; ++i);
    }
}


static void initFrameTimer (void)
{
    // -------------------------------------------------------------------------
    // Frame start interrupt
    // -------------------------------------------------------------------------
    // Activate Repetitive Interrupt Timer (RIT) for periodic IRQs
    Chip_RIT_Init       (LPC_RITIMER);

    // CLK_MX_RITIMER Clock Rate = 204000000
    const uint32_t Cmp_value = (uint32_t)
            ((int32_t)(Chip_Clock_GetRate(CLK_MX_RITIMER) / WS2811_FRAME_RATE));

    Chip_RIT_SetCOMPVAL (LPC_RITIMER, Cmp_value);
    Chip_RIT_EnableCTRL (LPC_RITIMER, RIT_CTRL_ENCLR);
    Chip_RIT_Enable     (LPC_RITIMER);

    // Enable IRQ for RIT
    NVIC_SetPriority    (RITIMER_IRQn, 0);
    NVIC_EnableIRQ      (RITIMER_IRQn);
}


static bool init (void)
{
    BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 1);

    // -------------------------------------------------------------------------
    // Memory exchange area initialization
    // -------------------------------------------------------------------------
    // The M4 core will not update vdrvExchange before initialization procedures
    // and M0 bootup, so it is safe to set default driver values.
    g_videoExchange.framebuffer = (uint32_t) s_vFramebuffer;
    g_videoExchange.frameNumber = s_vFrameCount;

    // Error reported on adapter initialization
    g_videoExchange.errorCode   = 0;

    // Driver information
    g_videoExchange.description = WS2811_DESCRIPTION,
    g_videoExchange.signal      = NULL,
    g_videoExchange.modeline    = WS2811_MODELINE,
    g_videoExchange.build       = CC_BuildInfoStr;

    // Generic adapter Input/Output
    g_videoExchange.d           = 0;

    initFrameTimer ();

    return true;
}


void RIT_IRQHandler (void)
{
//    __DSB ();
//    __ISB ();

    // if (Chip_RIT_GetIntStatus(LPC_RITIMER) == SET)
//    if (!(LPC_RITIMER->CTRL & RIT_CTRL_INT))
//    {
 //       return;
 //   }

    Chip_RIT_ClearInt       (LPC_RITIMER);
    NVIC_ClearPendingIRQ    (RITIMER_IRQn);
}


noreturn
static void signalLoop ()
{
    while (1)
	{
        // Begin a new frame
        __WFI ();

        for (s_vFrameComp = 0;
             s_vFrameComp < WS2811_CH1_COMP;
             ++ s_vFrameComp)
        {
            s_vFrameData = s_vFramebuffer[s_vFrameComp];

            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 1);
            M0_InstDelay (s_VIcWaitH[(s_vFrameData & 1 << 7)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 0);
            M0_InstDelay (s_VIcWaitL[(s_vFrameData & 1 << 7)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 1);
            M0_InstDelay (s_VIcWaitH[(s_vFrameData & 1 << 6)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 0);
            M0_InstDelay (s_VIcWaitL[(s_vFrameData & 1 << 6)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 1);
            M0_InstDelay (s_VIcWaitH[(s_vFrameData & 1 << 5)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 0);
            M0_InstDelay (s_VIcWaitL[(s_vFrameData & 1 << 5)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 1);
            M0_InstDelay (s_VIcWaitH[(s_vFrameData & 1 << 4)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 0);
            M0_InstDelay (s_VIcWaitL[(s_vFrameData & 1 << 4)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 1);
            M0_InstDelay (s_VIcWaitH[(s_vFrameData & 1 << 3)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 0);
            M0_InstDelay (s_VIcWaitL[(s_vFrameData & 1 << 3)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 1);
            M0_InstDelay (s_VIcWaitH[(s_vFrameData & 1 << 2)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 0);
            M0_InstDelay (s_VIcWaitL[(s_vFrameData & 1 << 2)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 1);
            M0_InstDelay (s_VIcWaitH[(s_vFrameData & 1 << 1)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 0);
            M0_InstDelay (s_VIcWaitL[(s_vFrameData & 1 << 1)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 1);
            M0_InstDelay (s_VIcWaitH[(s_vFrameData & 1 << 0)? 1 : 0]);
            BOARD_GPIO_FAST_WRITE (WS2811_CH1_DOUT, 0);
            M0_InstDelay (s_VIcWaitL[(s_vFrameData & 1 << 0)? 1 : 0]);

            M0_InstDelay (WS2811_IC_ID);
        }

        // Signals a new frame to the M4 core
        g_videoExchange.frameNumber = ++ s_vFrameCount;
        // __DSB ();
        __SEV ();

        // RESET: Sets frame to led stripe
        // Also let the M4 core update g_videoExchange
        M0_InstDelay (WS2811_RET_ID);

        s_vFramebuffer = (uint8_t *) g_videoExchange.framebuffer;
	}
}


int main ()
{
	if (init ())
    {
        signalLoop ();
    }

    return 0;
}
