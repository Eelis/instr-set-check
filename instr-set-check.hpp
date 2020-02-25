#ifndef INSTR_SET_CHECK_HPP
#define INSTR_SET_CHECK_HPP

#include <cpuid.h>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <iterator>

namespace instr_set_check
{
    enum Register { eax, ebx, ecx, edx };

    struct Feature
    {
        char const * name;
        uint32_t leaf;
        Register reg;
        int bit;
    };

    constexpr Feature needed[] = {
        #ifdef __SSE3__
            { "SSE 3", 1, ecx, 0 },
        #endif
        #ifdef __SSE4_1__
            { "SSE 4.1", 1, ecx, 19 },
        #endif
        #ifdef __SSE4_2__
            { "SSE 4.2", 1, ecx, 20 },
        #endif
        #ifdef __POPCNT__
            { "POPCNT", 1, ecx, 23 },
        #endif
        #ifdef __XSAVE__
            { "XSAVE", 1, ecx, 26 },
        #endif
        #ifdef __AVX__
            { "AVX", 1, ecx, 28 },
        #endif
        #ifdef __F16C__
            { "F16C", 1, ecx, 29 },
        #endif
        #ifdef __RDRND__
            { "RDRND", 1, ecx, 30 },
        #endif
        #ifdef __PCLMUL__
            { "PCLMUL", 1, ecx, 1 },
        #endif
        #ifdef __FSGSBASE__
            { "FSGSBASE", 7, ebx, 0 },
        #endif
        #ifdef __SGX__
            { "SGX", 7, ebx, 2 },
        #endif
        #ifdef __BMI__
            { "BMI", 7, ebx, 3 },
        #endif
        #ifdef __BMI2__
            { "BMI2", 7, ebx, 8 },
        #endif
        #ifdef __RTM__
            { "RTM", 7, ebx, 11 },
        #endif
        #ifdef __CLFLUSHOPT__
            { "CLFLUSHOPT", 7, ebx, 23 },
        #endif
        #ifdef __RDSEED__
            { "RDSEED", 7, ebx, 18 },
        #endif
        #ifdef __ADX__
            { "ADX", 7, ebx, 19 },
        #endif
    };

    struct Checker
    {
        uint64_t missing = 0; // one bit per feature listed in 'needed'
        static_assert(std::size(needed) <= 64);

        Checker()
        {
            uint32_t a[4];
            __get_cpuid(0, &a[eax], &a[ebx], &a[ecx], &a[edx]);

            uint32_t const max_leaf = a[eax];
            uint32_t current_leaf = 0;

            auto mark_missing = [&](int idx){ missing |= (1 << idx); };

            for (int idx = 0; idx != std::size(needed); ++idx)
            {
                Feature const & feature = needed[idx];

                if (feature.leaf != current_leaf)
                {
                    if (feature.leaf > max_leaf)
                    {
                        mark_missing(idx);
                        continue;
                    }

                    __get_cpuid_count(feature.leaf, 0, &a[eax], &a[ebx], &a[ecx], &a[edx]);
                    current_leaf = feature.leaf;
                }

                if (!(a[feature.reg] & (1 << feature.bit)))
                    mark_missing(idx);
            }
        }

        void report_missing()
        {
            if (!missing) return;

            std::fprintf(stderr,
                "error: This program requires the following extensions, "
                "which are not supported by this machine: ");
            char const * fmt = "%s";
            for (int idx = 0; idx != std::size(needed); ++idx)
                if (missing & (1 << idx))
                {
                    std::fprintf(stderr, fmt, needed[idx].name);
                    fmt = ", %s";
                }
            std::fprintf(stderr, "\n");

            std::exit(1);
        }
    };

    #ifndef MANUALLY_INVOKED_INSTRUCTION_SET_CHECK
    namespace detail
    {
        inline int dummy = (Checker().report_missing(), 0);
    }
    #endif
}

#endif
