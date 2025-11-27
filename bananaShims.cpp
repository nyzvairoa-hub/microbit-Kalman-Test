#include "pxt.h" // Essential Micro:bit/MakeCode header
#include "MicroBit.h"

using namespace pxt;

//define init of PCA9685
#define PCA9685_ADDRESS 0x40
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE
#define LED0_ON_L 0x06

// define motor here
#define M1_IN1_CHANN 4
#define M1_IN2_CHANN 5

//define rotation of motor
#define FORWARD 1
#define BACKWARD 2

// --- NAMESPACE DECLARATIONS ---
namespace banana {
    // Global state variables
    static bool banana_loop_true = false; 
    static int globalSpeed = 0;
    static int globalDir = 0;

    // Helper declarations
    void i2cWrite16x2(uint8_t reg, int value);
    void set_pwm(int channel, int onValue, int offValue);
    void control_motor1(int speed, int dir);
    void i2cWrite(uint8_t reg, uint8_t value);
    void i2cInit();
    
    // Fiber loop declaration
    void banana_loop();

    // Shim declarations
    //%
    void banana_getAccelX();
    //%
    void banana_run(int speed, int dir);
    //%
    void banana_stop();
    //%
    void banana_multPrint(int n, int m);
}

// --- FUNCTION DEFINITIONS (Must use banana:: prefix) ---

void banana::i2cWrite16x2(uint8_t reg, int value){
    uint8_t buff[3];
    buff[0] = reg;
    buff[1] = value & 0xFF; 
    buff[2] = (value >> 8) & 0xFF; 
    uBit.i2c.write(PCA9685_ADDRESS, (const char*)buff, 3);
}

void banana::set_pwm(int channel, int onValue, int offValue){
    uint8_t reg = LED0_ON_L + 4 * channel;
    banana::i2cWrite16x2(reg, onValue);
    banana::i2cWrite16x2(reg + 2, offValue);
}

void banana::control_motor1(int speed, int dir){
    int duty_cycle = (speed * 4095) / 255;
    if(duty_cycle < 0) duty_cycle = 0;
    if(duty_cycle > 4095) duty_cycle = 4095;

    if(dir == FORWARD){
        banana::set_pwm(M1_IN1_CHANN, 0, duty_cycle);
        banana::set_pwm(M1_IN2_CHANN, 0, 0);
        uBit.serial.printf("Direction: %d \r\n", dir);
    } else if(dir == BACKWARD){
        banana::set_pwm(M1_IN1_CHANN, 0, 0);
        banana::set_pwm(M1_IN2_CHANN, 0, duty_cycle);
        uBit.serial.printf("Direction: %d \r\n", dir);
    } else{
        banana::set_pwm(M1_IN1_CHANN, 0, 0);
        banana::set_pwm(M1_IN2_CHANN, 0, 0);
        uBit.serial.printf("Direction: STOP\r\n");
    }
}

void banana::i2cWrite(uint8_t reg, uint8_t value){
    uint8_t buff[2];
    buff[0] = reg;
    buff[1] = value;
    uBit.i2c.write(PCA9685_ADDRESS, (const char*)buff, 2);
}

void banana::i2cInit(){
    banana::i2cWrite(PCA9685_MODE1, 0x00);
    fiber_sleep(10);
    banana::i2cWrite(PCA9685_MODE1, 0x10); 
    banana::i2cWrite(PCA9685_PRESCALE, 0x79); 
    banana::i2cWrite(PCA9685_MODE1, 0x00); 
    fiber_sleep(10);
    banana::i2cWrite(PCA9685_MODE1, 0xa0); 
}

// --- SHIM DEFINITIONS ---

void banana::banana_loop(){
    while(banana_loop_true){
        // Ensure we call the namespaced control function
        banana::control_motor1(globalSpeed, globalDir);
        fiber_sleep(10);
    }
    banana::control_motor1(0, 0);
}

void banana::banana_run(int speed, int dir){
    if(!banana_loop_true){
        banana::i2cInit();
        globalSpeed = speed;
        globalDir = dir;
        banana_loop_true = true;
        
        // Pass the function pointer correctly
        create_fiber(banana::banana_loop);
    }
}

void banana::banana_stop(){
    banana::control_motor1(0, 0);
    globalDir = 0;
    globalSpeed = 0;
    banana_loop_true = false;
}

void banana::banana_getAccelX(){
    int x = uBit.accelerometer.getX(); 
    uBit.serial.printf("Accel X: %d mg\r\n", x);
}

void banana::banana_multPrint(int n, int m){
    int x = n * m;
    uBit.serial.printf("Mult result X: %d mg\r\n", x);
}