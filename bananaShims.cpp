#include "pxt.h" // Essential Micro:bit/MakeCode header
#include "MicroBit.h"

using namespace pxt;

namespace banana{
    #define v1_mult 5
    #define v2_mult 10

    //%
    int banana_add(int left, int right){
        return left + right;
    }

    //%
    int banana_minus(int left, int right){
        return left - right;
    }

    //%
    int banana_mult(int bananas){
        #if MICROBIT_CODAL
            return bananas * v2_mult;
        #else
            return bananas * v1_mult;
        #endif
    }

    //%
    int bananas_v2_only(int bananas){
        #if MICROBIT_CODAL
            return bananas;
        #else
            target_panic(PANIC_VARIANT_NOT_SUPPORTED);
        #endif
    }

    //%
    int banana_multV2(int n, int m){
        return n*m;
    }

    //%
    int banana_getAccelX(){

        int x = uBit.accelerometer.getX(); 
        uBit.serial.printf("Accel X: %d mg\r\n", x);
        return x;
    }

}

