#ifndef qpn_port_h
#define qpn_port_h

#define Q_PARAM_SIZE            4
#define QF_TIMEEVT_CTR_SIZE     4

/* maximum # active objects--must match EXACTLY the QF_active[] definition  */
#define QF_MAX_ACTIVE           1

                               /* interrupt locking policy for GNU compiler */
#define QF_INT_DISABLE()        __asm volatile ("cpsid i")
#define QF_INT_ENABLE()         __asm volatile ("cpsie i")

                      /* interrupt locking policy for ISR level, see NOTE01 */
#define QF_ISR_NEST

#include <stdint.h>  /* standard exact-width integer types supported by GNU */

#include "qepn.h"         /* QEP-nano platform-independent public interface */
#include "qfn.h"           /* QF-nano platform-independent public interface */

/*****************************************************************************
* NOTE01:
* ARM Cortex does not disable interrupts upon the interrupt entry, so
* interrupts can nest by default. You can prevent interrupts from nesting by
* setting the priorities of all interrupts to the same level. In any case,
* you should not call any QP-nano services with interrupts disabled.
*/

#endif                                                        /* qpn_port_h */
