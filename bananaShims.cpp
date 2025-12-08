#include "pxt.h" 
#include "MicroBit.h"
#include "nrf_gpio.h"
#include <math.h>

using namespace pxt;

// --- CONSTANTS ---
#define PCA9685_ADDRESS (0x40 << 1)
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE
#define LED0_ON_L 0x06

#define STOP 0
#define FORWARD 1
#define BACKWARD 2

// --- KALMAN CLASS (Directly Embedded) ---
class PVKalman {
    public:
        float x, v;       // State
        float p_xx, p_vv; // Covariance
        float q_pos, q_vel, r_measure; // Tuning

        PVKalman(float measurement_noise, float process_noise){
            x = 0; v = 0;
            p_xx = 100.0; p_vv = 100.0;
            r_measure = measurement_noise;
            q_pos = process_noise; q_vel = process_noise;
        }

        void predict(float dt){
            x += v * dt;
            p_xx += dt * dt * p_vv + q_pos;
            p_vv += q_vel;
        }

        void update(float measurement){
            float k_x = p_xx / (p_xx + r_measure);
            float k_v = p_vv / (p_vv + r_measure);
            
            float inno = measurement - x;
            x = x + k_x * inno;
            v = v + k_v * inno;

            p_xx = (1.0 - k_x) * p_xx;
            p_vv = (1.0 - k_v) * p_vv;
        }
};

// --- NAMESPACE START ---
namespace banana {

    // --- GLOBAL VARIABLES --- 
    static bool banana_loop_bool = false; 
    static int globalSpeed[4] = {0,0,0,0};
    static int globalDir[4] = {0,0,0,0};
    static int globalChannel[4] = {4,6,10,8};

    // --- Kalman Filter Instances ---
    static PVKalman filterX(10.0, 0.1);
    static PVKalman filterWth(10.0, 0.2); 

    // --- Sensor Variables --- 
    static int sensorX = 0;
    static int sensorY = 0;
    static int sensorW = 0; // Renamed to avoid confusion
    static int sensorH = 0;
    static bool objectDetected = false; 

    // --- HELPER FUNCTIONS --- 
    void i2cWrite(uint8_t reg, uint8_t value){
        uint8_t buff[2];
        buff[0] = reg; buff[1] = value;
        
        #if MICROBIT_CODAL
            uBit.i2c.write(PCA9685_ADDRESS, buff, 2, false);
        #else
            uBit.i2c.write(PCA9685_ADDRESS, (char*)buff, 2, false);
        #endif
    }

    void i2cWrite16x2(uint8_t reg, int value){
        uint8_t buff[3];
        buff[0] = reg; buff[1] = value & 0xFF; buff[2] = (value >> 8) & 0xFF; 
        
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
        } else if(dir == BACKWARD){
            set_pwm(channel, 0, 0);
            set_pwm(channel + 1, 0, duty_cycle);
        } else{
            set_pwm(channel, 0, 0);
            set_pwm(channel + 1, 0, 0);
        }
    }

    void i2cInit(){
        // Pin configuration for I2C
        nrf_gpio_cfg(32, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT,
                    NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_NOSENSE);
        uBit.sleep(50); 
        i2cWrite(PCA9685_MODE1, 0x00); uBit.sleep(10);
        i2cWrite(PCA9685_MODE1, 0x10); uBit.sleep(5);
        i2cWrite(PCA9685_PRESCALE, 0x79); 
        i2cWrite(PCA9685_MODE1, 0x00); uBit.sleep(5);
        i2cWrite(PCA9685_MODE1, 0xa0); uBit.sleep(5);
    }

    // --- MAIN LOOP ---
    void banana_loop(){
        while(banana_loop_bool){
            float dt = 0.01; 

            // Predict
            filterX.predict(dt);
            filterWth.predict(dt);

            // Correct
            if(objectDetected){
                filterX.update((float)sensorX);
                filterWth.update((float)sensorW);
            }

            // Clean Data
            int smoothX = (int)filterX.x;
            int smoothW = (int)filterWth.x;

            // Optional Debug
            // uBit.serial.printf("X: %d, W: %d\r\n", smoothX, smoothW);

            for(int i = 0; i < 4; i++){
                controlMotor(i);
            }
            
            // Critical Yield
            uBit.sleep(10);
        }
    }

    // --- EXPORTS ---

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
    void husky_lens_data(int x, int y, int _w, int _h, bool isDetected){
        sensorX = x;
        sensorY = y;
        sensorW = _w;
        sensorH = _h;
        objectDetected = isDetected;
    }
}