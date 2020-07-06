/* create opj_config.h for CMake */
#if !defined (_WIN32) || !defined (_MSC_VER) || (_MSC_VER >= 1700)
#define OPJ_HAVE_STDINT_H 		1
#endif

/*--------------------------------------------------------------------------*/
/* OpenJPEG Versioning                                                      */

/* Version number. */
#define OPJ_VERSION_MAJOR 2
#define OPJ_VERSION_MINOR 3
#define OPJ_VERSION_BUILD 1
