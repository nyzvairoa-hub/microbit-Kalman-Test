#include "pxt.h" 
#include "MicroBit.h"
#include "KalmanFilterHeader.h"

using namespace pxt;

// --- CONSTANTS ---
#define PCA9685_ADDRESS (0x40 << 1)
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE
#define LED0_ON_L 0x06

#define STOP 0
#define FORWARD 1
#define BACKWARD 2

// --- NAMESPACE START ---
namespace banana {

    // --- GLOBAL VARIABLES --- 
    static bool banana_loop_bool = false; 
    static int globalSpeed[4] = {0,0,0,0} ;
    static int globalDir[4] = {0,0,0,0} ;
    static int globalChannel[4] = {4,6,10,8};

    // --- Kalman Filter Initiation ---
    static PVKalman filterX(10.0, 0.1);
    static PVKalman filterWth(10.0, 0.1);

    // --- Variable for sensor --- 
    static int sensorX = 0;
    static int sensorY = 0;
    static int width = 0;
    static int height = 0;
    static bool objectDectected = false;

    // --- 1. HELPER FUNCTIONS --- 

    void i2cWrite(uint8_t reg, uint8_t value){
        uint8_t buff[2];
        buff[0] = reg;
        buff[1] = value;
        
        #if MICROBIT_CODAL
            int res = uBit.i2c.write(PCA9685_ADDRESS, buff, 2, false);
        #else
            int res = uBit.i2c.write(PCA9685_ADDRESS, (char*)buff, 2, false);
        #endif
        
        if (res != 0) {
            //uBit.serial.printf("I2C Write Error: %d at reg %d\r\n", res, reg);
        }
    }

    void i2cWrite16x2(uint8_t reg, int value){
        uint8_t buff[3];
        buff[0] = reg;
        buff[1] = value & 0xFF; 
        buff[2] = (value >> 8) & 0xFF; 
        
        #if MICROBIT_CODAL
            uBit.i2c.write(PCA9685_ADDRESS, buff, 3, false);
        #else
            uBit.i2c.write(PCA9685_ADDRESS, (char*)buff, 3, false);
        #endif
    }

    void set_pwm(int channel, int onValue, int offValue){
        uint8_t reg = LED0_ON_L + 4 * channel;
        i2cWrite16x2(reg, onValue);
        i2cWrite16x2(reg + 2, offValue);
    }

    void controlMotor(int motorID){

        int speed = globalSpeed[motorID];
        int dir = globalDir[motorID];
        int channel = globalChannel[motorID];

        int duty_cycle = (speed * 4095) / 255;
        if(duty_cycle < 0) duty_cycle = 0;
        if(duty_cycle > 4095) duty_cycle = 4095;

        if(dir == FORWARD){
            set_pwm(channel, 0, duty_cycle);
            set_pwm(channel + 1, 0, 0);
            //uBit.serial.printf("M1: FWD %d\r\n", speed);
        } else if(dir == BACKWARD){
            set_pwm(channel, 0, 0);
            set_pwm(channel + 1, 0, duty_cycle);
            //uBit.serial.printf("M1: REV %d\r\n", speed);
        } else{
            set_pwm(channel, 0, 0);
            set_pwm(channel + 1, 0, 0);
            //uBit.serial.printf("M1: STOP\r\n");
        }
    }

    // --- REFINED SCANNER ---
    void scanI2C() {
        uBit.serial.printf("\r\n--- STARTING I2C SCAN ---\r\n");
        int devices_found = 0;
        
        // Scan standard 7-bit addresses
        for (int i = 8; i < 120; i++) {
            uint8_t dummy = 0;
            #if MICROBIT_CODAL
                int res = uBit.i2c.write(i << 1, &dummy, 0, false); 
            #else
                int res = uBit.i2c.write(i << 1, (char*)&dummy, 0, false);
            #endif
            
            if (res == 0) {
                uBit.serial.printf("DEVICE FOUND AT: 0x%x (%d)\r\n", i, i);
                devices_found++;
                uBit.sleep(10); 
            }
        }
        uBit.serial.printf("Scan Complete. Found %d devices.\r\n", devices_found);
        uBit.serial.printf("-------------------------\r\n");
    }

    void i2cInit(){
        scanI2C(); // Run scan first

        // Give the bus a moment to settle after scanning
        uBit.sleep(50); 

        // Try initialization sequence
        i2cWrite(PCA9685_MODE1, 0x00);
        uBit.sleep(10);
        i2cWrite(PCA9685_MODE1, 0x10); 
        uBit.sleep(5);
        i2cWrite(PCA9685_PRESCALE, 0x79); 
        i2cWrite(PCA9685_MODE1, 0x00); 
        uBit.sleep(5);
        i2cWrite(PCA9685_MODE1, 0xa0); 
        uBit.sleep(5);
    }

    // --- 2. FIBER LOOP ---
    void banana_loop(){
        while(banana_loop_bool){
            float dt = 0.01; // Assuming loop runs every 10ms

            // perdict
            filterX.predict(dt);
            filterWth.predict(dt);

            // correction phase
            if(objectDectected){
                filterX.update((float)sensorX);
                filterWth.update((float)width);
            }

            // clean data
            int smoothX = (int)filterX.x;
            int smoothWth = (int)filterWth.x;

            //uBit.serial.printf("X: %d, Wth: %d\r\n", smoothX, smoothWth);

            for(int i = 0; i < 4; i++){
                controlMotor(i);
            }
            uBit.sleep(10);
        }
    }

    //%
    void banana_init(){
     if(!banana_loop_bool){
        i2cInit();
        banana_loop_bool = true;
        create_fiber(banana_loop);
        }
    }

    //%
    void banana_set_motor(int motorID, int speed, int dir){
        if(motorID >= 0 && motorID < 4){
            globalSpeed[motorID] = speed;
            globalDir[motorID] = dir;
        }
    }

    //%
    void husky_pos(int x, int y){
        sensorX = x;
        sensorY = y;
    }

    //%
    void husky_size(int _w, int _h, bool _b){
        width = _w;
        height = _h;
        objectDectected = _b;
    }
} 
