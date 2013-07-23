#pragma once

#ifdef _WIN32
    #define MINICRAFT_WINDOWS
#else
    #define MINICRAFT_LINUX
#endif

#ifdef MINICRAFT_WINDOWS
    #define ERROR_MESSAGE(X,Y) MessageBox(0, (X), (Y), MB_OK | MB_ICONERROR)
    #ifdef __MINGW32__
        #define BREAKPOINT() asm("int $3")
    #else
        //#define BREAKPOINT() _CrtDbgBreak()
        #define BREAKPOINT() __asm { int 5 }
    #endif
#endif

#ifdef MINICRAFT_LINUX
    #define ERROR_MESSAGE(X,Y) std::cerr<<(Y)<<(X)<<'\n';
    // this has to stay here until I find something normal -.-
    #include <signal.h>
    #define BREAKPOINT() raise(SIGTRAP)
#endif


