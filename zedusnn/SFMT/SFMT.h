#ifndef SFMT_H
#define SFMT_H


#include <stdio.h>

#if defined(_MSC_VER)
#define SFMT_DllExport   __declspec( dllexport )

#ifdef __cplusplus
extern "C" {
#endif

#elif defined(__GNUC__)
#define SFMT_DllExport
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
  #include <inttypes.h>
#elif defined(_MSC_VER) || defined(__BORLANDC__)
  typedef unsigned int uint32_t;
  typedef unsigned __int64 uint64_t;
  #define inline __inline
#else
  #include <inttypes.h>
  #if defined(__GNUC__)
    #define inline __inline__
  #endif
#endif

#ifndef PRIu64
  #if defined(_MSC_VER) || defined(__BORLANDC__)
    #define PRIu64 "I64u"
    #define PRIx64 "I64x"
  #else
    #define PRIu64 "llu"
    #define PRIx64 "llx"
  #endif
#endif

#if defined(__GNUC__)
#define ALWAYSINLINE __attribute__((always_inline))
#else
#define ALWAYSINLINE
#endif

#if defined(_MSC_VER)
  #if _MSC_VER >= 1200
    #define PRE_ALWAYS __forceinline
  #else
    #define PRE_ALWAYS inline
  #endif
#else
  #define PRE_ALWAYS inline
#endif

SFMT_DllExport uint32_t gen_rand32(void);
SFMT_DllExport uint64_t gen_rand64(void);
SFMT_DllExport int fill_array32(uint32_t *array, int size);
SFMT_DllExport int fill_array64(uint64_t *array, int size);
SFMT_DllExport void init_gen_rand(uint32_t seed);

void init_by_array(uint32_t *init_key, int key_length);
const char *get_idstring(void);
int get_min_array_size32(void);
int get_min_array_size64(void);
int get_array32_extended_size(int size);
int get_array64_extended_size(int size);

#endif

#if defined(_MSC_VER)
#ifdef __cplusplus
}
#endif
#endif
