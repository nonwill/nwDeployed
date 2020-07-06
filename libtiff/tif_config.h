#ifndef _TIF_CONFIG_H_
#define _TIF_CONFIG_H_

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Support CCITT Group 3 & 4 algorithms */
#define CCITT_SUPPORT 1

/* Pick up YCbCr subsampling info from the JPEG data stream to support files
   lacking the tag (default enabled). */
#define CHECK_JPEG_YCBCR_SUBSAMPLING 1

/* enable partial strip reading for large strips (experimental) */
/* #undef CHUNKY_STRIP_READ_SUPPORT */

/* Support C++ stream API (requires C++ compiler) */
/* #undef CXX_SUPPORT */

/* Treat extra sample as alpha (default enabled). The RGBA interface will
   treat a fourth sample with no EXTRASAMPLE_ value as being ASSOCALPHA. Many
   packages produce RGBA files but don't mark the alpha properly. */
#define DEFAULT_EXTRASAMPLE_AS_ALPHA 1

/* enable deferred strip/tile offset/size loading */
/* #undef DEFER_STRILE_LOAD */

/* Define to 1 if you have the <assert.h> header file. */
#define HAVE_ASSERT_H 1

/* Define to 1 if you have the declaration of `optarg', and to 0 if you don't.
   */
#define HAVE_DECL_OPTARG 0

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
/* #undef HAVE_FSEEKO */

/* Define to 1 if you have the `getopt' function. */
/* #undef HAVE_GETOPT */

/* Define to 1 if you have the <GLUT/glut.h> header file. */
/* #undef HAVE_GLUT_GLUT_H */

/* Define to 1 if you have the <GL/glut.h> header file. */
/* #undef HAVE_GL_GLUT_H */

/* Define to 1 if you have the <GL/glu.h> header file. */
/* #undef HAVE_GL_GLU_H */

/* Define to 1 if you have the <GL/gl.h> header file. */
/* #undef HAVE_GL_GL_H */

/* Define as 0 or 1 according to the floating point format suported by the
   machine */
#define HAVE_IEEEFP 1

/* Define to 1 if you have the <inttypes.h> header file. */
/* #undef HAVE_INTTYPES_H */

/* Define to 1 if you have the <io.h> header file. */
/* #undef HAVE_IO_H */

/* Define to 1 if you have the `jbg_newlen' function. */
#define HAVE_JBG_NEWLEN 1

/* Define to 1 if you have the `lfind' function. */
/* #undef HAVE_LFIND */

/* Define to 1 if you have the <memory.h> header file. */
/* #undef HAVE_MEMORY_H */

/* Define to 1 if you have the `mmap' function. */
/* #undef HAVE_MMAP */

/* Define to 1 if you have the <OpenGL/glu.h> header file. */
/* #undef HAVE_OPENGL_GLU_H */

/* Define to 1 if you have the <OpenGL/gl.h> header file. */
/* #undef HAVE_OPENGL_GL_H */

/* Define if you have POSIX threads libraries and header files. */
/* #undef HAVE_PTHREAD */

/* Define to 1 if you have the <search.h> header file. */
#define HAVE_SEARCH_H 1

/* Define to 1 if you have the `setmode' function. */
/* #undef HAVE_SETMODE */

/* Define to 1 if you have the `snprintf' function. */
#if defined(snprintf) || (!defined(_MSC_VER) || _MSC_VER >= 1900)
#define HAVE_SNPRINTF 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
/* #undef HAVE_STDINT_H */

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
/* #undef HAVE_STRCASECMP */

/* Define to 1 if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strtol' function. */
#define HAVE_STRTOL 1

/* Define to 1 if you have the `strtoll' function. */
/* #undef HAVE_STRTOLL */

/* Define to 1 if you have the `strtoul' function. */
#define HAVE_STRTOUL 1

/* Define to 1 if you have the `strtoull' function. */
/* #undef HAVE_STRTOULL */

/* Define to 1 if you have the <sys/stat.h> header file. */
/* #undef HAVE_SYS_STAT_H */

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#if !defined(_WIN32)
#define HAVE_UNISTD_H 1
#endif

/* Use nonstandard varargs form for the GLU tesselator callback */
/* #undef HAVE_VARARGS_GLU_TESSCB */

/* Define to 1 if you have the <windows.h> header file. */
#if defined(_WIN32)
#define HAVE_WINDOWS_H 1
#endif

/* Native cpu byte order: 1 if big-endian (Motorola) or 0 if little-endian
   (Intel) */
//#if defined(Q_BYTE_ORDER) && (Q_BYTE_ORDER == Q_BIG_ENDIAN)
//#define HOST_BIGENDIAN 1
//#else
//#define HOST_BIGENDIAN 0
//#endif

/* Set the native cpu bit order (FILLORDER_LSB2MSB or FILLORDER_MSB2LSB) */
#define HOST_FILLORDER FILLORDER_LSB2MSB

/* Support ISO JBIG compression (requires JBIG-KIT library) */
#if defined(HAS_JBIGKIT)
#define JBIG_SUPPORT 1
#endif

/* 8/12 bit libjpeg dual mode enabled */
/* #undef JPEG_DUAL_MODE_8_12 */

/* Support JPEG compression (requires IJG JPEG library) */
#if defined(HAS_LIBJPEG)
#define JPEG_SUPPORT 1
#endif

/* 12bit libjpeg primary include file with path */
/* #undef LIBJPEG_12_PATH */

/* Support LogLuv high dynamic range encoding */
#define LOGLUV_SUPPORT 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
/* #undef LT_OBJDIR */

/* Support LZMA2 compression */
#if defined(HAS_XZUTILS)
#define LZMA_SUPPORT 1
#endif

/* Support LZW algorithm */
#define LZW_SUPPORT 1

/* Support Microsoft Document Imaging format */
#define MDI_SUPPORT 1

/* Support NeXT 2-bit RLE algorithm */
#define NEXT_SUPPORT 1

/* Support Old JPEG compresson (read-only) */
#if defined(HAS_LIBJPEG)
#define OJPEG_SUPPORT 1
#endif

/* Name of package */
#define PACKAGE "tiff"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "tiff@lists.maptools.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "LibTIFF Software"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "LibTIFF Software 4.1.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "tiff"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "4.1.0"

/* Support Macintosh PackBits algorithm */
#define PACKBITS_SUPPORT 1

/* Support Pixar log-format algorithm (requires Zlib) */
#if defined(HAS_ZLIB)
#define PIXARLOG_SUPPORT 1
#endif

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* The size of `signed int', as computed by sizeof. */
/* #undef SIZEOF_SIGNED_INT */

/* The size of `signed long', as computed by sizeof. */
/* #undef SIZEOF_SIGNED_LONG */

/* The size of `signed long long', as computed by sizeof. */
/* #undef SIZEOF_SIGNED_LONG_LONG */

/* The size of `size_t', as computed by sizeof. */
/* #undef SIZEOF_SIZE_T */

/* The size of `unsigned char *', as computed by sizeof. */
/* #undef SIZEOF_UNSIGNED_CHAR_P */

/* The size of `unsigned int', as computed by sizeof. */
/* #undef SIZEOF_UNSIGNED_INT */

/* The size of `unsigned long', as computed by sizeof. */
/* #undef SIZEOF_UNSIGNED_LONG */

/* The size of `unsigned long long', as computed by sizeof. */
/* #undef SIZEOF_UNSIGNED_LONG_LONG */

/* Define to 1 if you have the ANSI C header files. */
/* #undef STDC_HEADERS */

/* Support strip chopping (whether or not to convert single-strip uncompressed
   images to mutiple strips of specified size to reduce memory usage) */
#define STRIPCHOP_DEFAULT TIFF_STRIPCHOP

/* Default size of the strip in bytes (when strip chopping enabled) */
#define STRIP_SIZE_DEFAULT 8192

/* Enable SubIFD tag (330) support */
#define SUBIFD_SUPPORT 1

/* Support ThunderScan 4-bit RLE algorithm */
#define THUNDER_SUPPORT 1
/* Signed 64-bit type formatter */
#define TIFF_INT64_FORMAT "%I64d"

/* Signed 64-bit type */
#define TIFF_INT64_T signed __int64

/* Unsigned 64-bit type formatter */
#define TIFF_UINT64_FORMAT "%I64u"

/* Unsigned 64-bit type */
#define TIFF_UINT64_T unsigned __int64

#if _WIN64
/*
  Windows 64-bit build
*/

/* Pointer difference type */
#  define TIFF_PTRDIFF_T TIFF_INT64_T

/* The size of `size_t', as computed by sizeof. */
#  define SIZEOF_SIZE_T 8

/* Size type formatter */
#  define TIFF_SIZE_FORMAT TIFF_INT64_FORMAT

/* Unsigned size type */
#  define TIFF_SIZE_T TIFF_UINT64_T

/* Signed size type formatter */
#  define TIFF_SSIZE_FORMAT TIFF_INT64_FORMAT

/* Signed size type */
#  define TIFF_SSIZE_T TIFF_INT64_T

#else
/*
  Windows 32-bit build
*/

/* Pointer difference type */
#  define TIFF_PTRDIFF_T signed int

/* The size of `size_t', as computed by sizeof. */
#  define SIZEOF_SIZE_T 4

/* Size type formatter */
#  define TIFF_SIZE_FORMAT "%u"

/* Size type formatter */
#  define TIFF_SIZE_FORMAT "%u"

/* Unsigned size type */
#  define TIFF_SIZE_T unsigned int

/* Signed size type formatter */
#  define TIFF_SSIZE_FORMAT "%d"

/* Signed size type */
#  define TIFF_SSIZE_T signed int

#endif

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
/* #undef TIME_WITH_SYS_TIME */

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* define to use win32 IO system */
#if defined(_WIN32)
#define USE_WIN32_FILEIO 1
#endif

/* Version number of package */
#define VERSION "4.1.0"

/* Support webp compression */
#if defined(HAS_LIBWEBP)
#define WEBP_SUPPORT 1
#endif

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to 1 if the X Window System is missing or not being used. */
#define X_DISPLAY_MISSING 1

#define lfind _lfind

/* Support Deflate compression */
#if defined(HAS_ZLIB)
#define ZIP_SUPPORT 1
#endif

/* Support zstd compression */
#if defined(HAS_ZSTD)
#define ZSTD_SUPPORT 1
#endif

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif


/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

#endif /* _TIF_CONFIG_H_ */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * fill-column: 78
 * End:
 */
