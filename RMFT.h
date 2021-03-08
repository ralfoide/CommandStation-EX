#ifndef RMFT_H
#define RMFT_H
#include "RMFT2.h"

class RMFT {
  public:
   static void inline begin() {RMFT2::begin();}
   static void inline loop() {RMFT2::loop();}
};

#if __has_include ( "myAutomation.h")
  #include "myAutomation.h"
  #define RMFT_ACTIVE
#endif

#endif
