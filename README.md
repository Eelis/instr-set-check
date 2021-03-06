# instr-set-check

## Motivation

Programs often target specific extended instruction sets.

Unfortunately, when a program is run on a machine that does *not* support the instructions needed by the program,
this problem is not detected until the program attempts to execute an unsupported instruction, at which point the program will crash.

The purpose of this library is to instead detect and report the problem much earlier, during program startup, so that crashes in
the middle of the program run can be avoided.

## Basic usage

All you need to do is:

    #include "instr-set-check.hpp"

This will make the program check during startup to see if the machine supports the required instructions;
and will print an error and exit the program if some required instructions are unsupported.

An example error might look like:

    error: This program requires the following extensions, which are not supported by this machine: SGX, CLFLUSHOPT

## Advanced usage

If you want to run the checks at a different time, or want to report the error to the user in a different way, or want to generate a completely different error message, do:

    #define MANUALLY_INVOKED_INSTRUCTION_SET_CHECK
    #include "instr-set-check.hpp"

    int main()
    {
        instr_set_check::missing_exts missing;
            // constructing the object performs the checks

        diagnose(missing);
            // If there were any missing extensions, `diagnose` will:
            //   1. report them to stderr, and
            //   2. exit the program.
            // Alternatively, you can query the `missing` object directly
            // to enumerate missing extensions.
            // See the implementation of `diagnose` for an example.
    }

## Implementation

For every extension that is enabled in the program (as indicated by compiler-predefined macros such as `__SSE3__`),
the library checks the corresponding flag in [CPUID](https://en.wikipedia.org/wiki/CPUID).
