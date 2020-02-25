# instr-set-check

## Motivation

Programs often target specific extended instruction sets.

Unfortunately, when a program is run on a machine that does /not/ support the instructions needed by the program,
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
        ...
        instr_set_check::Checker checker; // constructing the object performs the checks

        // you can now either call checker.report_missing() to have the error (if any) printed to stderr,
        // or examine the bits in checker.missing, which contains 1 bit per Feature element in the instr_set_check::needed array.
    }
