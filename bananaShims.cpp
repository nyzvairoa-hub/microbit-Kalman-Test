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
    static bool isAutoMode = true;
    static int motorSpeed[4] = {0,0,0,0};
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

    const int TARGET_X = 160;
    const int TARGET_WTH = 100;
    static float KP_TURN = 0.4;
    static float KP_DIST = 1.5;

    const int DEAD_TURN = 10;
    const int DEAD_DIST = 5;

    const int MAX_TURN_SPEED = 150;
    const int MIN_DRIVE_SPEED = 50;

    // Fuzzy Logic Parameters
    static float minE = 20.0;
    static float maxE = 120.0;

    // Your "Car Intuition" Limits
    static float minWidth = 30.0;  // Far away (Limit you found)
    static float maxWidth = 120.0; // Close up

    uint64_t lastTime = uBit.systemTime();

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

    void controlMotorSign(int motorID,int speedVal){
        int dir = STOP;
        int duty = 0;
        int channel = globalChannel[motorID];

        if(speedVal > 0){
            dir = FORWARD;
            duty = speedVal;
        } else if(speedVal < 0){
            dir = BACKWARD;
            duty = -speedVal;
        } else{
            dir = STOP;
            duty = 0;
        }

        if (duty > 255) duty = 255;

        int pwmVal = (duty * 4095) / 255;

        if(dir == FORWARD){
            set_pwm(channel, 0, pwmVal);
            set_pwm(channel + 1, 0, 0);
        } else if(dir == BACKWARD){
            set_pwm(channel, 0, 0);
            set_pwm(channel + 1, 0, pwmVal);
        } else{
            set_pwm(channel, 0, 0);
            set_pwm(channel + 1, 0, 0);
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

    float map_float(float x, float in_min, float in_max, float out_min, float out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    // --- 2. FIBER LOOP ---
// --- 2. FIBER LOOP ---
    void banana_loop(){
        int lostCount = 0;
        const int MAX_LOST_LOOP = 20;

        while(banana_loop_bool){
            // Timekeeping
            uint64_t now = uBit.systemTime();
            int dt_ms = (int)(now - lastTime);
            lastTime = now;
            
            // Kalman Predict
            float dt = 0.01; 
            filterX.predict(dt); filterWth.predict(dt);

            // Kalman Update
            if(objectDectected){
                filterX.update((float)sensorX); filterWth.update((float)width);
                lostCount = 0;
            } else {
                lostCount++;
                if(lostCount > MAX_LOST_LOOP){
                    filterX.v = 0; filterWth.v = 0;
                }
            }

            if(isAutoMode){
                if(lostCount > MAX_LOST_LOOP){
                    for(int i = 0; i < 4; i++){ motorSpeed[i] = 0; }
                } else {
                    int smoothX = (int)filterX.x;
                    int smoothW = (int)filterWth.x;

                    int errorTurn = smoothX - TARGET_X;
                    int errorDist = TARGET_WTH - smoothW;

                    // Deadbands
                    if (abs(errorTurn) < DEAD_TURN) errorTurn = 0;
                    if (abs(errorDist) < DEAD_DIST) errorDist = 0;

                    // --- LOGIC FIX 1: Prevent Negative Drive Gain ---
                    float absError = (float)abs(errorTurn);
                    float dynamic_kp_turn = KP_TURN;
                    float dynamic_kp_dist = KP_DIST;

                    if(absError < minE){
                        // Zone 1: Driving Straight
                        dynamic_kp_turn = KP_TURN;
                        dynamic_kp_dist = KP_DIST;
                    } else if(absError > maxE){
                        // Zone 3: Hard Turn (Panic)
                        dynamic_kp_turn = KP_TURN + 0.2; 
                        // FIX: Multiply instead of Subtract. 
                        // This keeps 20% of drive speed instead of stopping/reversing.
                        dynamic_kp_dist = KP_DIST * 0.2; 
                    } else {
                        // Zone 2: Sliding
                        dynamic_kp_turn = map_float(absError, minE, maxE, KP_TURN, KP_TURN + 0.2);
                        // FIX: Map to 20% scaler
                        dynamic_kp_dist = map_float(absError, minE, maxE, KP_DIST, KP_DIST * 0.2);
                    }

                    // --- LOGIC FIX 2: Use the Distance Scaler ---
                    float dist_scaler = 1.0;
                    float currentWidth = (float)smoothW; // Use filtered width

                    if (currentWidth <= minWidth) {
                        // FAR AWAY: Steer Gentle
                        dist_scaler = 0.7;
                    } else if (currentWidth >= maxWidth) {
                        // CLOSE UP: Steer Sharp
                        dist_scaler = 1.0;
                    } else {
                        // MIDDLE
                        dist_scaler = map_float(currentWidth, minWidth, maxWidth, 0.35, 1.0);
                    }

                    // FIX: Apply the scaler to the turn!
                    int turnOutput = (int)(dynamic_kp_turn * dist_scaler * errorTurn);
                    int driveOutput = (int)(dynamic_kp_dist * errorDist);

                    // Anti-Stall
                    if(abs(driveOutput) > 0 && abs(driveOutput) < MIN_DRIVE_SPEED){
                        if(driveOutput > 0) driveOutput = MIN_DRIVE_SPEED;
                        else driveOutput = -MIN_DRIVE_SPEED;
                    }

                    // Clamping
                    if(turnOutput > MAX_TURN_SPEED) turnOutput = MAX_TURN_SPEED;
                    if(turnOutput < -MAX_TURN_SPEED) turnOutput = -MAX_TURN_SPEED;

                    // Mixing
                    int leftSpeed = driveOutput + turnOutput;
                    int rightSpeed = driveOutput - turnOutput;

                    // FIX 3: Enable All Motors (Assuming 4WD)
                    motorSpeed[0] = leftSpeed;
                    //motorSpeed[1] = leftSpeed; // Uncommented
                    motorSpeed[2] = rightSpeed;
                    //motorSpeed[3] = rightSpeed; // Uncommented
                }
            }

            for(int i = 0; i < 4; i++){
                controlMotorSign(i, motorSpeed[i]);
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
    void banana_set_motor(int motorID, int speed){
        if(motorID >= 0 && motorID < 4){
            isAutoMode = false;
            motorSpeed[motorID] = speed;
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

    //%
    void pid_value(float _kp_turn, float _kp_dist){
        KP_TURN = _kp_turn;
        KP_DIST = _kp_dist;
    }

    //%
    void set_auto_mode(bool enabled){
        isAutoMode = enabled;
    }
} 
