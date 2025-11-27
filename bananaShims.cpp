#include "pxt.h" // Essential Micro:bit/MakeCode header
#include "MicroBit.h"

using namespace pxt;

//define init of PCA9685
#define PCA9685_ADDRESS 0x40 //#define is use for constant value.
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE
#define LED0_ON_L 0x06

// define motor here
#define M1_IN1_CHANN 4
#define M1_IN2_CHANN 5

//define rotation of motor
#define FORWARD 1
#define BACKWARD 2

namespace banana{
    static bool banana_loop_true = false; // static const is for read only memory, similar to #define but have more flexsibilty
    static float alpha = 0.1f; // Smoothing factor for low-pass filter

    /**
    * @brief writes a 16-bit value (2 bytes) to specific register in I2C device
    * used for setting PWM cycle start/end points
    * @param reg register address to write to
    * @param value 12-bit (0-4095) value to write to register.
    */
    void i2cWrite16x2(uint8_t reg, int value){
        uint8_t buff[3];
        buff[0] = reg;
        buff[1] = value & 0xFF; // low byte
        buff[2] = (value >> 8) & 0xFF; // high byte
        uBit.i2c.write(PCA9685_ADDRESS, (const char*)buff, 3);
    }


    /**
    * @brief sets pwm to specific channel (0-15)
    * @param channel channel number (4/5 for M1)
    * @param onValue PWM cycle start tick (at 0)
    * @param offValue PWM cycle end tick (0-4095)
    */
    void set_pwm(int channel, int onValue, int offValue){
        uint8_t reg = LED0_ON_L + 4 * channel;

        i2cWrite16x2(reg, onValue);
        i2cWrite16x2(reg + 2, offValue);
    }

    /**
    * @brief controls motor 1 speed and direction
    * @param power power level (0-255)
    * @param dir direction (FORWARD, BACKWARD, STOP)
    */
    void control_motor1(int power, int dir){
        //scale the power from 0-255 to 0-4095
        int duty_cycle = (power * 4095) / 255;
        if(duty_cycle < 0) duty_cycle = 0;
        if(duty_cycle > 4095) duty_cycle = 4095;

        if(dir == FORWARD){
            //forward
            set_pwm(M1_IN1_CHANN, 0, duty_cycle);
            set_pwm(M1_IN2_CHANN, 0, 0);
            uBit.serial.printf("Direction: %d \r\n", dir);
        } else if(dir == BACKWARD){
            //backward
            set_pwm(M1_IN1_CHANN, 0, 0);
            set_pwm(M1_IN2_CHANN, 0, duty_cycle);
            uBit.serial.printf("Direction: %d \r\n", dir);
        } else{
            //stop
            set_pwm(M1_IN1_CHANN, 0, 0);
            set_pwm(M1_IN2_CHANN, 0, 0);
            uBit.serial.printf("Direction: STOP\r\n");
        }
    }

    /**
    * @brief sends a single byte command to the PCA9685
    */
    void i2cWrite(uint8_t reg, uint8_t value){
        uint8_t buff[2];
        buff[0] = reg;
        buff[1] = value;
        uBit.i2c.write(PCA9685_ADDRESS, (const char*)buff, 2);

    }

    void i2cInit(){
        i2cWrite(PCA9685_MODE1, 0x00);
        fiber_sleep(10);

        i2cWrite(PCA9685_MODE1, 0x01);
        i2cWrite(PCA9685_PRESCALE, 0x79);
        i2cWrite(PCA9685_MODE1, 0x00);
        fiber_sleep(10);
        i2cWrite(PCA9685_MODE1, 0xa0);
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

            uBit.serial.printf("Raw X: %d mg, Smooth X: %f mg\r\n", x, smoothX);

            fiber_sleep(10);
        }
    }

    //%
    void banana_run(float alphaVal){
        if(!banana_loop_true){
            i2cInit();
            
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

