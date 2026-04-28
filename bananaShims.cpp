#include "pxt.h" 
#include "MicroBit.h"
#include "NewKalmanFilterHeader.h"

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

    static float r_measure = 0;
    static float q_process_angle_pos = 0;
    static float q_process_angle_vel = 0;
    static float q_process_distance_pos = 0;
    static float q_process_distance_vel = 0;

    // --- Kalman Filter Initiation ---
    //static PVKalman filterX(r_measure, q_measure);
    //static PVKalman filterWth(r_measure, q_measure);
    static PVKalman filterAngle(r_measure, q_process_angle_pos, q_process_angle_vel);
    static PVKalman filterDistance(r_measure, q_process_distance_pos, q_process_distance_vel);

    // --- Variable for sensor --- 
    static int sensorX = 0;
    static int sensorY = 0;
    static int width = 0;
    static int height = 0;
    static bool objectDectected = false;

    const int TARGET_X = 160;
    const int TARGET_WTH = 80;
    static float KP_TURN = 0.4;
    static float KP_DIST = 1.5;
    static float KD_TURN = 0.5; 
    static float KD_DIST = 0.5;

    const int DEAD_TURN = 10;
    const int DEAD_DIST = 5;

    const int MAX_TURN_SPEED = 150;
    const int MIN_DRIVE_SPEED = 40;

    // Fuzzy Logic Parameters
    static float minE = 20.0;
    static float maxE = 120.0;

    // Your "Car Intuition" Limits
    static float minWidth = 30.0;  // Far away (Limit you found)
    static float maxWidth = 120.0; // Close up

    uint64_t lastTime = uBit.systemTime();
    int global_dt_ms = 0; 
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
        const int MAX_LOST_LOOP = 70;

        while(banana_loop_bool){
            // Timekeeping
            uint64_t now = uBit.systemTime();
            int dt_ms = (int)(now - lastTime);
            lastTime = now;

            global_dt_ms = dt_ms;

            //uBit.serial.printf("%d\r\n", dt_ms);
            // --- BUG FIX: Convert ms to seconds for the Physics Math ---
            float dt = (float)dt_ms / 1000.0f;
            if (dt <= 0.0f) dt = 0.01f;
            if (dt > 0.1f) dt = 0.1f; // Cap dt to prevent massive jumps if processor hangs
            
            // Kalman Predict
            //float dt = 0.01; 
            filterAngle.predict(dt); filterDistance.predict(dt);

            if(isAutoMode){
                
                if(objectDectected){
                    filterAngle.update((float)sensorX); filterDistance.update((float)width);
                    lostCount = 0;
                } 
                else {
                    lostCount++;
                }
            
                int smoothX = (int)filterAngle.pos;
                int smoothW = (int)filterDistance.pos;

                int errorTurn = smoothX - TARGET_X;
                int errorDist = TARGET_WTH - smoothW;

                // Deadbands
                if (abs(errorTurn) < DEAD_TURN) errorTurn = 0;
                if (abs(errorDist) < DEAD_DIST) errorDist = 0;

                // --- LOGIC FIX 1: Prevent Negative Drive Gain ---
                float currentWidth = (float)smoothW; 
                float dist_scaler = 1.0;
                float drive_scaler = 1.0;

                if (currentWidth <= minWidth) {
                    // SCENARIO B: FAR AWAY
                    dist_scaler = 0.2;   // Gentle, lazy turning arc
                    drive_scaler = 1.0;  // 100% forward drive speed
                } else if (currentWidth >= maxWidth) {
                    // SCENARIO A: DANGER CLOSE
                    dist_scaler = 0.5;   // 100% sharp pivot steering
                    drive_scaler = 0.2;  // Drop forward speed to 20% to avoid crashing
                } else {
                    // MIDDLE DISTANCE: Smooth transition
                    dist_scaler = map_float(currentWidth, minWidth, maxWidth, 0.2, 0.5);
                    drive_scaler = map_float(currentWidth, minWidth, maxWidth, 1.0, 0.2);
                }

                float velocityTurn = filterAngle.vel; 
                float velocityDist = filterDistance.vel;

                float turn_P = KP_TURN * errorTurn;
                float turn_D = KD_TURN * velocityTurn;

                float dist_P = KP_DIST * errorDist;
                float dist_D = KD_DIST * velocityDist;

                // FIX: Apply the scaler to the turn!
                int turnOutput = (int)((turn_P + turn_D) * dist_scaler);
                int driveOutput = (int)((dist_P - dist_D) * drive_scaler);

                // Anti-Stall

                const int TURN_DEADBAND = 30; 
                if(abs(driveOutput) < TURN_DEADBAND) {
                if (turnOutput > 0) turnOutput += TURN_DEADBAND;
                else if (turnOutput < 0) turnOutput -= TURN_DEADBAND;
                }                

                if(abs(driveOutput) > 0 && abs(driveOutput) < MIN_DRIVE_SPEED){
                    if(driveOutput > 0) driveOutput = MIN_DRIVE_SPEED;
                    else driveOutput = -MIN_DRIVE_SPEED;
                }

                // Clamping
                if(turnOutput > MAX_TURN_SPEED) turnOutput = MAX_TURN_SPEED;
                if(turnOutput < -MAX_TURN_SPEED) turnOutput = -MAX_TURN_SPEED;

                //driveOutput = 100; // Constant speed for testing

                if(lostCount > MAX_LOST_LOOP){
                    turnOutput = 0;
                    driveOutput = (int)(driveOutput * 0.20); // Coast to a stop
                    if(abs(driveOutput) < 20) driveOutput = 0; // Hard stop at low speeds
                }

                // Mixing
                int leftSpeed = driveOutput + turnOutput;
                int rightSpeed = driveOutput - turnOutput;

                //uBit.serial.printf("right: %d, left: %d, drive: %d, turn: %d\r\n", rightSpeed, leftSpeed, driveOutput, turnOutput);

                // FIX 3: Enable All Motors (Assuming 4WD)
                motorSpeed[0] = leftSpeed;
                //motorSpeed[1] = leftSpeed; // Uncommented
                motorSpeed[2] = rightSpeed;
                //motorSpeed[3] = rightSpeed; // Uncommented

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
    void kp_value(float _kp_turn, float _kp_dist){
        KP_TURN = _kp_turn;
        KP_DIST = _kp_dist;
    }

    //%
    void kd_value(float _kd_turn, float _kd_dist){
        KD_TURN = _kd_turn;
        KD_DIST = _kd_dist;
    }

    //%
    void set_auto_mode(bool enabled){
        isAutoMode = enabled;
    }

    //%
    int dt_time(){
        return global_dt_ms;
    }

    //%
    void KalmanFilterValuesAngle(float r_meas, float q_meas_pos, float q_meas_vel){
        r_measure = r_meas;
        q_process_angle_pos = q_meas_pos;
        q_process_angle_vel = q_meas_vel;

        // ADD THESE LINES TO UPDATE THE OBJECTS
        filterAngle.R_measure = r_meas;
        filterAngle.Q_pos = q_meas_pos;
        filterAngle.Q_vel = q_meas_vel;

    }

    //%
    void KalmanFilterValuesDistance(float r_meas, float q_meas_pos, float q_meas_vel){
        r_measure = r_meas;
        q_process_distance_pos = q_meas_pos;
        q_process_distance_vel = q_meas_vel;

        // ADD THESE LINES TO UPDATE THE OBJECTS
        filterDistance.R_measure = r_measure;
        filterDistance.Q_pos = q_meas_pos;
        filterDistance.Q_vel = q_meas_vel;
    }
} 
