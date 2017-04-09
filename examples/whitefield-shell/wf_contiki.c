#include "contiki.h"

#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
PROCESS(wf_contiki, "Whitefield Process");
AUTOSTART_PROCESSES(&wf_contiki);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(wf_contiki, ev, data)
{
  PROCESS_BEGIN();

  printf("Hello, whitefield\n");
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
