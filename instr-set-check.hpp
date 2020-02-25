#ifndef INSTR_SET_CHECK_HPP
#define INSTR_SET_CHECK_HPP

#include <cpuid.h>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <iterator>

namespace instr_set_check
{
    namespace detail
    {
        enum register_ { eax, ebx, ecx, edx };

        struct feature
        {
            char const * name;
            uint32_t leaf;
            register_ reg;
            int bit;
        };

        constexpr feature needed[] = {
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

        static_assert(std::size(needed) <= 64);

        inline uint64_t get_missing()
        {
            uint64_t missing = 0; // one bit per feature listed in 'needed'

            uint32_t a[4];
            __get_cpuid(0, &a[eax], &a[ebx], &a[ecx], &a[edx]);

            uint32_t const max_leaf = a[eax];
            uint32_t current_leaf = 0;

            for (int i = 0; i != std::size(needed); ++i)
            {
                feature const & feat = needed[i];

                if (feat.leaf != current_leaf)
                {
                    if (feat.leaf > max_leaf)
                    {
                        missing |= (1 << i);
                        continue;
                    }

                    __get_cpuid_count(feat.leaf, 0, &a[eax], &a[ebx], &a[ecx], &a[edx]);
                    current_leaf = feat.leaf;
                }

                if (!(a[feat.reg] & (1 << feat.bit)))
                    missing |= (1 << i);
            }

            return missing;
        }
    }

    class missing_exts
    {
        uint64_t missing = detail::get_missing();

    public:

        bool any() const { return missing != 0; }

        template<typename F>
        void for_each(F f) const // F must be callable with a char const*
        {
            if (!any()) return;

            for (int i = 0; i != std::size(detail::needed); ++i)
                if (missing & (1 << i))
                    f(detail::needed[i].name);
        }
    };

    inline void diagnose(missing_exts const missing)
    {
        if (!missing.any()) return;

        std::fprintf(stderr,
            "error: This program requires the following extensions, "
            "which are not supported by this machine: ");

        char const * fmt = "%s";
        missing.for_each(
            [&](char const * const name)
            {
                std::fprintf(stderr, fmt, name);
                fmt = ", %s";
            });

        std::fprintf(stderr, "\n");

        std::exit(1);
    }

    #ifndef MANUALLY_INVOKED_INSTRUCTION_SET_CHECK
    namespace detail
    {
        inline int dummy = (diagnose(missing_exts()), 0);
    }
    #endif
}

#endif
