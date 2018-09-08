// cb.h --- chronobox driver K.Olchanski TRIUMF Aug 2018

#ifndef INCLUDE_CB_H
#define INCLUDE_CB_H

#include <stdio.h>
#include <stdint.h>

class Chronobox
{
 public:
   Chronobox(); // ctor
   ~Chronobox(); // dtor

 public:
   int cb_open();

 public:
   uint32_t read32(int addr);
   void write32(int addr, uint32_t data);

 public:
   uint32_t cb_read32(int ireg);
   void cb_write32(int ireg, uint32_t data);
   void cb_write32bis(int ireg, uint32_t data1, uint32_t data2);

 public:
   void cb_reboot();

 public:
   int cb_read_input_num();
   void cb_read_scaler_begin();
   uint32_t cb_read_scaler(int iscaler);

 public:
   char* fBase = NULL;
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

