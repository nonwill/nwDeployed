
#if BUILDING_LIBICONV
#if defined WIN32 && BUILDING_DLL
#define LIBICONV_DLL_EXPORTED __declspec(dllexport)
#else
#define LIBICONV_DLL_EXPORTED
#endif
