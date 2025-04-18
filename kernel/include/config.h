/**
 * @file config.h
 * @author Luyao Han (luyaohan1001@gmail.com)
 * @brief Configuration file to YesRTOS, including platform and application dependent settings.
 * @version 0.1
 * @date 2024-07-15
 *
 * @copyright Copyright (c) 2024
 *
 */

#define CPU_CLK_FREQ_HZ         (16000000UL)    // Core frequency.
#define TIMESLICE_FREQ_HZ       (3600UL)           // Timeslice frequency.

#define TASK_QUEUE_DEPTH        (32U)           // RoundRobinScheduler

#define MAX_PRIO_LEVEL          (8U)            // PreemptFIFOScheduler

// #define HOST_PLATFORM // Host platform of cross-compilation; used to control native traces to iostream.