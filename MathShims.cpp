#include "pxt.h" // Essential Micro:bit/MakeCode header

// --- 1. LOW LATENCY ADDITION (Mirroring the 'lowLatencyAdd' block) ---
// The function name (LowLatencyAdd) MUST match the shim target annotation: shim=MathShims::LowLatencyAdd
// The extern "C" ensures compatibility with the C-style function call from the linker.
extern "C" int LowLatencyAdd(int num1, int num2) {
    // This is where your highly efficient math happens.
    // In your final project, this will contain the full KF and PID floating-point matrix operations.
    // Here, we just perform a direct integer addition.
    int result = num1 + num2;

    // The function must return the result expected by the TypeScript block (a number).
    return result;
}


// --- 2. LOW LATENCY SUBTRACTION (Mirroring the 'lowLatencySubtract' block) ---
// The function name (LowLatencySubtract) MUST match the shim target annotation: shim=MathShims::LowLatencySubtract
extern "C" int LowLatencySubtract(int num1, int num2) {
    int result = num1 - num2;
    return result;
}
