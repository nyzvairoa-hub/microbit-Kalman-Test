#include "pxt.h" // Essential Micro:bit/MakeCode header
#include "MicroBit.h"

using namespace pxt;

namespace banana{
    #define v1_mult 5
    #define v2_mult 10
    static bool banana_loop_true = false;
    static float alpha = 0.1f; // Smoothing factor for low-pass filter
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
    void banana_getAccelX(){

        int x = uBit.accelerometer.getX(); 
        uBit.serial.printf("Accel X: %d mg\r\n", x);
    }

    void banana_loop(){
        float smoothX = 0.0f;
        while(banana_loop_true){
            int x = uBit.accelerometer.getX();
            //int y = uBit.accelerometer.getY();
            //int z = uBit.accelerometer.getZ();

            float rawX = (float)x;
            
            smoothX = alpha * rawX + (1.0f - alpha) * smoothX;

            uBit.serial.printf("Raw X: %d mg, Smooth X: %.2f mg\r\n", x, smoothX);

            fiber_sleep(10);
        }
    }

    //%
    void banana_run(float alphaVal){
        if(!banana_loop_true){
            if(alphaVal >= 0.0f && alphaVal <= 1.0f){
                alpha = alphaVal;
            } else {
                alpha = 0.1f; // Default value if out of range
            }

            banana_loop_true = true;
            create_fiber(banana_loop);
        }
    }

    //%
    void banana_stop(){
        banana_loop_true = false;
    }

}

