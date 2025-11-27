#include "pxt.h" // Essential Micro:bit/MakeCode header
#include "MicroBit.h"

using namespace pxt;

namespace banana{
    #define v1_mult 5
    #define v2_mult 10
    static bool banana_loop_true = false;
    static float alpha = 0.1f; // Smoothing factor for low-pass filter

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

            uBit.serial.printf("Raw X: %d mg, Smooth X: %f mg\r\n", x, smoothX);

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
            uBit.serial.printf("Using alpha: %f\r\n", alpha);
            banana_loop_true = true;
            create_fiber(banana_loop);
        }
    }

    //%
    void banana_stop(){
        banana_loop_true = false;
    }
    
    //%
    void banana_multPrint(int n, int m){
        int x = n * m;
        uBit.serial.printf("Mult result X: %d mg\r\n", x);
    }

}

