#include "pxt.h" 
#include "MicroBit.h"

using namespace pxt;

// --- CONSTANTS ---
#define PCA9685_ADDRESS 0x40
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE
#define LED0_ON_L 0x06
#define M1_IN1_CHANN 4
#define M1_IN2_CHANN 5
#define FORWARD 1
#define BACKWARD 2

// --- NAMESPACE START ---
namespace banana {

    // --- GLOBAL VARIABLES ---
    // Must be static to be fiber-safe
    static bool banana_loop_true = false; 
    static int globalSpeed = 0;
    static int globalDir = 0;

    // --- 1. HELPER FUNCTIONS (Logic INSIDE namespace) ---

    void i2cWrite(uint8_t reg, uint8_t value){
        uint8_t buff[2];
        buff[0] = reg;
        buff[1] = value;

        #if MICROBIT_CODAL
            int res = uBit.i2c.write(PCA9685_ADDRESS, buff, 2);
        #else
            int res = uBit.i2c.write(PCA9685_ADDRESS, (char*)buff, 2);
        #endif
        
        if (res != 0) {
            uBit.serial.printf("I2C Write Error: %d at reg %d\r\n", res, reg);
        }
    }

    void i2cWrite16x2(uint8_t reg, int value){
        uint8_t buff[3];
        buff[0] = reg;
        buff[1] = value & 0xFF; 
        buff[2] = (value >> 8) & 0xFF;

        #if MICROBIT_CODAL
            uBit.i2c.write(PCA9685_ADDRESS, buff, 3);
        #else
            uBit.i2c.write(PCA9685_ADDRESS, (char*)buff, 3);
        #endif
    }

    void set_pwm(int channel, int onValue, int offValue){
        uint8_t reg = LED0_ON_L + 4 * channel;
        i2cWrite16x2(reg, onValue);
        i2cWrite16x2(reg + 2, offValue);
    }

    void control_motor1(int speed, int dir){
        int duty_cycle = (speed * 4095) / 255;
        if(duty_cycle < 0) duty_cycle = 0;
        if(duty_cycle > 4095) duty_cycle = 4095;

        if(dir == FORWARD){
            set_pwm(M1_IN1_CHANN, 0, duty_cycle);
            set_pwm(M1_IN2_CHANN, 0, 0);
            //uBit.serial.printf("M1: FWD %d\r\n", speed);
        } else if(dir == BACKWARD){
            set_pwm(M1_IN1_CHANN, 0, 0);
            set_pwm(M1_IN2_CHANN, 0, duty_cycle);
            //uBit.serial.printf("M1: REV %d\r\n", speed);
        } else{
            set_pwm(M1_IN1_CHANN, 0, 0);
            set_pwm(M1_IN2_CHANN, 0, 0);
            //uBit.serial.printf("M1: STOP\r\n");
        }
    }

    void scanI2C() {
        uBit.serial.printf("\r\n--- STARTING I2C SCAN ---\r\n");
        int devices_found = 0;
        
        // Scan standard 7-bit addresses (0x08 to 0x77) to avoid reserved blocks
        for (int i = 8; i < 120; i++) {
            uint8_t dummy = 0;
            #if MICROBIT_CODAL
                // V2: address is shifted left by 1 (8-bit address)
                int res = uBit.i2c.write(i << 1, &dummy, 0); 
            #else
                int res = uBit.i2c.write(i << 1, (char*)&dummy, 0);
            #endif
            
            if (res == 0) {
                uBit.serial.printf("DEVICE FOUND AT: 0x%x (%d)\r\n", i, i);
                devices_found++;
                uBit.sleep(10); // Print delay
            }
        }
        
        if (devices_found == 0) {
            uBit.serial.printf("NO DEVICES FOUND! Check Power/Connections.\r\n");
        } else {
            uBit.serial.printf("Scan Complete. Found %d devices.\r\n", devices_found);
        }
        uBit.serial.printf("-------------------------\r\n");
    }

    void i2cInit(){

        scanI2C();
        fiber_sleep(50);
        i2cWrite(PCA9685_MODE1, 0x00);
        fiber_sleep(10);
        i2cWrite(PCA9685_MODE1, 0x10); 
        i2cWrite(PCA9685_PRESCALE, 0x79); 
        i2cWrite(PCA9685_MODE1, 0x00); 
        fiber_sleep(10);
        i2cWrite(PCA9685_MODE1, 0xa0); 
    }

    // --- 2. FIBER LOOP (Logic INSIDE namespace) ---
    void banana_loop(){
        while(banana_loop_true){
            control_motor1(globalSpeed, globalDir);
            fiber_sleep(10);
        }
        control_motor1(0, 0);
    }

    // --- 3. EXPORTED SHIMS (Logic INSIDE namespace) ---
    // The parser needs the code block { ... } to be right here!

    //%
    void banana_run(int speed, int dir){
        if(!banana_loop_true){
            i2cInit();
            globalSpeed = speed;
            globalDir = dir;
            banana_loop_true = true;
            create_fiber(banana_loop);
        } else {
            // Update speed even if loop is already running
            globalSpeed = speed;
            globalDir = dir;
        }
    }

    //%
    void banana_stop(){
        control_motor1(0, 0);
        globalDir = 0;
        globalSpeed = 0;
        banana_loop_true = false;
    }

} // --- END OF NAMESPACE ---