#include "vmsys.h"
#if defined(__clang__)
  #pragma clang diagnostic ignored "-Wparentheses"

  /* placing code in section(text) does not mark it executable with Clang. */
  #undef  LIBCO_MPROTECT
  #define LIBCO_MPROTECT
#endif

#if defined(__clang__) || defined(__GNUC__)
  #if defined(__i386__)
    #include "archs/x86.c"
  #elif defined(__arm__)
    #include "archs/arm.c"
  #elif defined(_WIN32)
    #include "archs/fiber.c"
  #endif
#elif defined(_MSC_VER)
 /* #if defined(_M_IX86)
    #include "archs/x86.c"
  #else*/
    #include "archs/fiber.c"
 // #endif
#else
  #error "libco: unsupported processor, compiler or operating system"
#endif
