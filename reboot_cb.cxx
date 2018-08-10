//
// reboot the chronobox fpga
//
// K.Olchanski TRIUMF August 2018
//

#include <stdio.h>

#include "cb.h"

int main(int argc, char* argv[])
{
   Chronobox* cb = new Chronobox();
   cb->cb_open();
   cb->cb_reboot();
   delete cb;
   return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
