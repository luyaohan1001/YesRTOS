#include "stdint.h"
#include "config.h"
#include "trace.h"

// SysTick memory mapped registers.
#define SYST_CSR             ( *( ( volatile uint32_t * ) 0xe000e010 ) )    // SysTick Control and Status Register
#define SYST_RVR             ( *( ( volatile uint32_t * ) 0xe000e014 ) )    // SysTick Reload Value Register
#define SYST_CVR             ( *( ( volatile uint32_t * ) 0xe000e018 ) )    // SysTick Current Value Register

// SysTick Reload Count
#define SYSTICK_RELOAD_CNT   ((CPU_CLK_FREQUENCY / TIME_SLICE_FREQUENCY) - 1UL)

#define SYST_CLKSOURCE_BIT          (1UL << 2UL)
#define SYST_TICKINT_BIT            (1UL << 1UL)
#define SYST_ENABLE_BIT             (1UL << 0UL)

/**
 * @brief Initialize systick registers.
 */
void systick_clk_init(void)
{
    SYST_CSR = SYST_CVR = 0UL; // clear status and current value
    SYST_RVR = SYSTICK_RELOAD_CNT;  // set reload value
    SYST_CSR = (SYST_CLKSOURCE_BIT | SYST_TICKINT_BIT | SYST_ENABLE_BIT);   // enable and make count to 0 trigger the SysTick exception.
}

/**
 * @brief SysTick exception handler, entry to scheduler on time slicing.
 */
void SysTick_Handler() {
    itm_trace("count_down_to_0\r\n");
}