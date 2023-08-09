#include <stdint.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    int32_t a = 2 << 15;
    int32_t b = 3 << 15;

    int64_t c = (a * b) >> 15;
    int64_t result = 0;
    printf("c: %lld\n", c);

    asm(
        "smull   %[result_lo], %[result_hi], %[a], %[b]   \n"           // Multiply a and b, store result in result_lo and result_hi
        "asr     %[result_hi], %[result_hi], #15         \n"            // Shift result_hi right by 15 bits
        "orr     %[result_lo], %[result_lo], %[result_hi], lsl #17  \n" // Combine result_lo and shifted result_hi
        : [result_lo] "=r"(result), [result_hi] "+r"(result)
        : [a] "r"(a), [b] "r"(b)
        : "cc");

    printf("result: %lld\n", result);

    return 0;
}
