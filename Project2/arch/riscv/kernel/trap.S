#include <asm.h>
#include <csr.h>

ENTRY(setup_exception)

  /* TODO: [p2-task3] save exception_handler_entry into STVEC */
  la   t0, exception_handler_entry
  csrw CSR_STVEC, t0

    /* TODO: [p2-task4] enable interrupts globally */
  /* enable interrupts globally will be done by calling
   * enable_interrupt() and enable_preempt() in main()
   * so there's nothing to do here
   */
  jr   ra
ENDPROC(setup_exception)
