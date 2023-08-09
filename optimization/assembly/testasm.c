#include <stdint.h>
#include <stdio.h>
#include "fixedpoint.h"

int main(int argc, char const *argv[])
{
    fixedpoint_t a = 2 << 15;
    fixedpoint_t b = 3 << 15;

    int32_t result_l;
    int32_t result_h;
    printf("c: %s\n", fixedpoint_str(fixedpoint_mul(a, b)));

    // asm(
    //     "smull   %[result_lo], %[result_hi], %[a], %[b]   \n\t"           // Multiply a and b, store result in result_lo and result_hi
    //     "asr     %[result_hi], %[result_hi], #15         \n\t"            // Shift result_hi right by 15 bits
    //     "orr     %[result_lo], %[result_lo], %[result_hi], lsl #17  \n\t" // Combine result_lo and shifted result_hi
    //     : [result_lo] "=r"(result_l), [result_hi] "+r"(result_h)
    //     : [a] "r"(a), [b] "r"(b)
    //     : "cc");
    // int64_t result = ((int64_t)result_h << 32) | (result_l & 0xffffffff);
    // printf("result: %s\n", result);

    return 0;
}
