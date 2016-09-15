
#include <stdio.h>
#include "SFMT.h"

//Exporting rSFMT.cpp functions:
SFMT_DllExport double to_real1(uint32_t v);
SFMT_DllExport double genrand_real1(void);
SFMT_DllExport double to_real2(uint32_t v);
SFMT_DllExport double genrand_real2(void);
SFMT_DllExport double to_real3(uint32_t v);
SFMT_DllExport double genrand_real3(void);
SFMT_DllExport double to_res53(uint64_t v);
SFMT_DllExport double to_res53_mix(uint32_t x, uint32_t y);
SFMT_DllExport double genrand_res53(void) ;
SFMT_DllExport double genrand_res53_mix(void);

/* These real versions are due to Isaku Wada */
/** generates a random number on [0,1]-real-interval */
double to_real1(uint32_t v)
{
    return v * (1.0/4294967295.0); 
    /* divided by 2^32-1 */ 
}

/** generates a random number on [0,1]-real-interval */
double genrand_real1(void)
{
    return to_real1(gen_rand32());
}

/** generates a random number on [0,1)-real-interval */
double to_real2(uint32_t v)
{
    return v * (1.0/4294967296.0); 
    /* divided by 2^32 */
}

/** generates a random number on [0,1)-real-interval */
double genrand_real2(void)
{
    return to_real2(gen_rand32());
}

/** generates a random number on (0,1)-real-interval */
double to_real3(uint32_t v)
{
    return (((double)v) + 0.5)*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

/** generates a random number on (0,1)-real-interval */
double genrand_real3(void)
{
    return to_real3(gen_rand32());
}

/** These real versions are due to Isaku Wada */

/** generates a random number on [0,1) with 53-bit resolution*/
double to_res53(uint64_t v) 
{ 
    return v * (1.0/18446744073709551616.0L);
}

/** generates a random number on [0,1) with 53-bit resolution from two
 * 32 bit integers */
double to_res53_mix(uint32_t x, uint32_t y) 
{ 
    return to_res53(x | ((uint64_t)y << 32));
}

/** generates a random number on [0,1) with 53-bit resolution
 */
double genrand_res53(void) 
{ 
    return to_res53(gen_rand64());
} 

/** generates a random number on [0,1) with 53-bit resolution
    using 32bit integer.
 */
double genrand_res53_mix(void) 
{ 
    uint32_t x, y;

    x = gen_rand32();
    y = gen_rand32();
    return to_res53_mix(x, y);
} 