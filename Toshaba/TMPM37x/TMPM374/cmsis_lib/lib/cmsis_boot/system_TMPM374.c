/**
 ****************************************************************************
 * @file     system_TMPM374.c
 * @brief    CMSIS Cortex-M3 Device Peripheral Access Layer Source File for the
 *           TOSHIBA 'TMPM374' Device Series 
 * @version  V2.2.0
 * @date:    2011/03/21
 * 
 * THE SOURCE CODE AND ITS RELATED DOCUMENTATION IS PROVIDED "AS IS". TOSHIBA
 * CORPORATION MAKES NO OTHER WARRANTY OF ANY KIND, WHETHER EXPRESS, IMPLIED OR,
 * STATUTORY AND DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * SATISFACTORY QUALITY, NON INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * THE SOURCE CODE AND DOCUMENTATION MAY INCLUDE ERRORS. TOSHIBA CORPORATION
 * RESERVES THE RIGHT TO INCORPORATE MODIFICATIONS TO THE SOURCE CODE IN LATER
 * REVISIONS OF IT, AND TO MAKE IMPROVEMENTS OR CHANGES IN THE DOCUMENTATION OR
 * THE PRODUCTS OR TECHNOLOGIES DESCRIBED THEREIN AT ANY TIME.
 * 
 * TOSHIBA CORPORATION SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGE OR LIABILITY ARISING FROM YOUR USE OF THE SOURCE CODE OR
 * ANY DOCUMENTATION, INCLUDING BUT NOT LIMITED TO, LOST REVENUES, DATA OR
 * PROFITS, DAMAGES OF ANY SPECIAL, INCIDENTAL OR CONSEQUENTIAL NATURE, PUNITIVE
 * DAMAGES, LOSS OF PROPERTY OR LOSS OF PROFITS ARISING OUT OF OR IN CONNECTION
 * WITH THIS AGREEMENT, OR BEING UNUSABLE, EVEN IF ADVISED OF THE POSSIBILITY OR
 * PROBABILITY OF SUCH DAMAGES AND WHETHER A CLAIM FOR SUCH DAMAGE IS BASED UPON
 * WARRANTY, CONTRACT, TORT, NEGLIGENCE OR OTHERWISE.
 * 
 * (C)Copyright TOSHIBA CORPORATION 2011 All rights reserved
 *****************************************************************************
 */

#include <stdint.h>
#include "TMPM374.h"

/*-------- <<< Start of configuration section >>> ----------------------------*/

/* Watchdog Timer (WD) Configuration */
#define WD_SETUP         1
#define WDMOD_Val        (0x00000000UL)
#define WDCR_Val         (0x000000B1UL)

/* Clock Generator (CG) Configuration */
#define CLOCK_SETUP      1
#define SYSCR_Val        (0x00010000UL)

/* #define INTERNAL_OSCILLATION */

#ifdef  INTERNAL_OSCILLATION
#define OSCCR_Val        (0x00010000UL) /* Use the internal oscillation with PLL OFF */
#else
#define OSCCR_Val        (0x000E0104UL) /* Use the external oscillation with PLL ON */
#endif

#define OSCCR_WUODR_MASK (0x000FFFFFUL)
#define OSCCR_WUODR_EXT  (0xFFF00000UL) /* OSCCR<WUPODR13:0> = MAX */
#define OSCCR_WUODR_PLL  (0x1F400000UL) /* OSCCR<WUPODR13:0> = 2000, warm-up time is 200us @ EXTALH = 10MHz */
#define STBYCR_Val       (0x00000103UL)
#define PLLSEL_Val       (0x0000A13FUL) /* PLLSEL<PLLSEL> = 1 */
#define CKSEL_Val        (0x00000000UL)

/*-------- <<< End of configuration section >>> ------------------------------*/

/*-------- DEFINES -----------------------------------------------------------*/
/* Define clocks */
#define XTALH     (10000000UL)  /* Internal high-speed oscillator freq */
#define EXTALH    (10000000UL)  /* External high-speed oscillator freq */

/* Determine core clock frequency according to settings */
#if (OSCCR_Val & (1U<<17))
#define CORE_TALH (EXTALH)
#else
#define CORE_TALH (XTALH)
#endif
#if ((PLLSEL_Val & (1U<<0)) && (OSCCR_Val & (1U<<2)))   /* If PLL selected and enabled */
#if   ((SYSCR_Val & 7U) == 0U)  /* Gear -> fc                         */
#define __CORE_CLK   (CORE_TALH * 8U )
#elif ((SYSCR_Val & 7U) == 8U)  /* Gear -> fc/2                       */
#define __CORE_CLK   (CORE_TALH * 8U / 2U)
#elif ((SYSCR_Val & 7U) == 5U)  /* Gear -> fc/4                       */
#define __CORE_CLK   (CORE_TALH * 8U / 4U )
#elif ((SYSCR_Val & 7U) == 6U)  /* Gear -> fc/8                       */
#define __CORE_CLK   (CORE_TALH * 8U / 8U)
#elif ((SYSCR_Val & 7U) == 7U)  /* Gear -> fc/16                      */
#define __CORE_CLK   (CORE_TALH * 8U / 16U)
#else                           /* Gear -> reserved                   */
#define __CORE_CLK   (0U)
#endif
#else                           /* If PLL not used                    */
#if   ((SYSCR_Val & 7U) == 0U)  /* Gear -> fc                         */
#define __CORE_CLK   (CORE_TALH)
#elif ((SYSCR_Val & 7U) == 4U)  /* Gear -> fc/2                       */
#define __CORE_CLK   (CORE_TALH / 2U)
#elif ((SYSCR_Val & 7U) == 5U)  /* Gear -> fc/4                       */
#define __CORE_CLK   (CORE_TALH / 4U)
#elif ((SYSCR_Val & 7U) == 6U)  /* Gear -> fc/8                       */
#define __CORE_CLK   (CORE_TALH / 8U)
#else                           /* Gear -> reserved                   */
#define __CORE_CLK   (0U)
#endif
#endif


/* Clock Variable definitions */
uint32_t SystemCoreClock = __CORE_CLK;  /*!< System Clock Frequency (Core Clock) */

/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Update SystemCoreClock according register values.
 */
void SystemCoreClockUpdate(void)
{                               /* Get Core Clock Frequency      */
    uint32_t CoreClock = 0U;
    /* Determine clock frequency according to clock register values        */
    /* System clock is high-speed clock */
    if (TSB_CG_OSCCR_OSCSEL) {  /* If system clock is External high-speed oscillator freq */
        CoreClock = EXTALH;
    } else {                    /* If system clock is Internal high-speed oscillator freq */
        CoreClock = XTALH;
    }
    if (TSB_CG_PLLSEL_PLLSEL && TSB_CG_OSCCR_PLLON) {   /* If PLL enabled      */
        switch (TSB_CG->SYSCR & 7U) {
        case 0U:               /* Gear -> fc          */
            SystemCoreClock = CoreClock * 8U;
            break;
        case 1U:
        case 2U:
        case 3U:               /* Gear -> reserved   */
            SystemCoreClock = 0U;
            break;
        case 4U:               /* Gear -> fc/2      */
            SystemCoreClock = CoreClock * 8U / 2U;
            break;
        case 5U:               /* Gear -> fc/4      */
            SystemCoreClock = CoreClock * 8U / 4U;
            break;
        case 6U:               /* Gear -> fc/8      */
            SystemCoreClock = CoreClock * 8U / 8U;
            break;
        case 7U:               /* Gear -> fc/16      */
            SystemCoreClock = CoreClock * 8U / 16U;
            break;
        default:
            SystemCoreClock = 0U;
        }
    } else {                    /* If PLL not used      */
        switch (TSB_CG->SYSCR & 7U) {
        case 0U:               /* Gear -> fc          */
            SystemCoreClock = CoreClock;
            break;
        case 1U:
        case 2U:
        case 3U:
        case 7U:               /* Gear -> reserved    */
            SystemCoreClock = 0U;
            break;
        case 4U:               /* Gear -> fc/2        */
            SystemCoreClock = CoreClock / 2U;
            break;
        case 5U:               /* Gear -> fc/4        */
            SystemCoreClock = CoreClock / 4U;
            break;
        case 6U:               /* Gear -> fc/8        */
            SystemCoreClock = CoreClock / 8U;
            break;
        default:
            SystemCoreClock = 0U;
        }
    }
}

/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System.
 */
void SystemInit(void)
{
#if (WD_SETUP)                  /* Watchdog Setup */
    TSB_WD->MOD = WDMOD_Val;
    if (!TSB_WD_MOD_WDTE) {     /* If watchdog is to be disabled */
        TSB_WD->CR = WDCR_Val;
    }
#endif

#if (CLOCK_SETUP)               /* Clock Setup */
    TSB_CG->SYSCR = SYSCR_Val;

    if (OSCCR_Val & (1U << 17)) {       /* Use the external oscillation */
        TSB_CG->OSCCR &= OSCCR_WUODR_MASK;
        TSB_CG->OSCCR |= OSCCR_WUODR_EXT;
        TSB_CG_OSCCR_HOSCON = 1U;
        TSB_CG_OSCCR_XEN1 = 1U;
        TSB_CG_OSCCR_WUPSEL2 = 1U;
        TSB_CG_OSCCR_WUPSEL1 = 0U;
        TSB_CG_OSCCR_WUEON = 1U;
        while (TSB_CG_OSCCR_WUEF) {
        }                       /* Warm-up */
        TSB_CG->OSCCR |= OSCCR_Val;
        TSB_CG_OSCCR_XEN2 = 0U;
    } else {                    /* Use the internal oscillation */
        TSB_CG->OSCCR |= OSCCR_Val;
    }

    if (TSB_CG_OSCCR_PLLON) {   /* If PLL enabled */
        TSB_CG->OSCCR &= OSCCR_WUODR_MASK;
        TSB_CG->OSCCR |= OSCCR_WUODR_PLL;
        TSB_CG->STBYCR = STBYCR_Val;
        TSB_CG_OSCCR_WUEON = 1U;
        while (TSB_CG_OSCCR_WUEF) {
        }                       /* Warm-up */
    } else {
        /* Do nothing */
    }

    TSB_CG->PLLSEL = PLLSEL_Val;
    TSB_CG->CKSEL = CKSEL_Val;
#endif
}
