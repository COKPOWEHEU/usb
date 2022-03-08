/*
 *  Linux
 */
#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)
  #ifndef linux
    #define linux 1 //чтобы была всего одна константа, а не десяток
  #endif
  //Linux
  #include <dlfcn.h>
  #define DynLoad(s) dlopen(s, RTLD_LAZY)
  #define DynFunc(lib, name)  dlsym(lib,name)
  #define DynClose(lib) dlclose(lib)
/*
 *  Win 32
 */
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
  #ifndef WIN32
    #define WIN32 1
  #endif
  //Win32
  #define _CRT_SECURE_NO_WARNINGS //что-то нужное для совместимости с MS Visual Studio
  #define _USE_MATH_DEFINES
  #include <windows.h>
  
  #define DynLoad(s) LoadLibrary(s)
  #define DynFunc(lib, name)   GetProcAddress((HINSTANCE)lib, name)
  #define DynClose(lib) FreeLibrary(lib)
 /*
  *  Other systems (unsupported)
  */
#else
  #error "Unsupported platform"
#endif
