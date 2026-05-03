
enum bananaMotor{
    M1 = 0,
    M2 = 1,
    M3 = 2,
    M4 = 3
}

enum bananaServo {
    S1 = 0,
    S2 = 1,
    S3 = 2,
    S4 = 3,
    S5 = 15,
    S6 = 14,
    S7 = 13,
    S8 = 12,

}

// Icon unicode characters can be found at: http://fontawesome.io/icons/
//% color=#c2b711 weight=100 icon="\uf1ec" block="Robot Smooth" advanced=false
namespace banana {

    //% shim=banana::banana_init
    function _init(): void { return; }

    //% shim=banana::banana_set_motor
    function _set_motor(id: number, speed: number): void { return; }
    //% shim=banana::banana_set_servo
    function _banana_set_servo(channel: number, degrees: number): void { return; }

    //% shim=banana::husky_pos
    function _husky_pos(x: number, y: number): void { return; }

    //% shim=banana::husky_size
    function _husky_size(width: number, height: number, isDetected: boolean): void { return; }

    //% shim=banana::kp_value
    function _kp_value(KPTurn: number, KPDist: number): void { return; }

    //% shim=banana::kd_value
    function _kd_value(KDTurn: number, KDDist: number): void { return; }

    //% shim=banana::set_auto_mode
    function _set_auto_mode(EnableAuto: boolean): void { return; }

    //% shim=banana::KalmanFilterValuesAngle
    function _KalmanFilterValuesAngle(r_measure_angle: number, q_measures_angle_pos: number, q_measures_angle_vel: number): void { return; }

    //% shim=banana::KalmanFilterValuesDistance
    function _KalmanFilterValuesDistance(r_measure_dis: number, q_measures_dis_pos: number, q_measures_dis_vel: number): void { return; }
    
    //% blockId=banana_StartMotor
    //% block="start motor"
    export function startMotor(): void{
        _init();
    }

    //% blockId=banana_Run
    //% block="run motor %motor with speed %speed"
    //% speed.min=-255 speed.max=255 speed.defl=0
    export function bananaRun(motor: bananaMotor, speed: number): void{
        _set_motor(motor, speed);
    }

    //% block="banana set servo channel %channel to %degrees degrees"
    //% degrees.min=0 degrees.max=180
    export function banana_set_servo(channel: bananaServo, degrees: number): void {
        _banana_set_servo(channel, degrees); 
    }

    //% blockId=banana_Stop
    //% block="stop motor %motor"
    export function bananaStop(motor: bananaMotor): void{
        _set_motor(motor, 0);
    }
    
    //% blockID:banana_HuskyLensData
    //% block="HuskyLens data x: %x y: %y width: %width height: %height isDetected: %isDetected"
    //% weight=50
    export function huskyLensData(x: number, y: number, width: number, height: number, isDetected: boolean): void{
        _husky_pos(x, y);
        _husky_size(width, height, isDetected);
    }

    //% blockID:banana_PIDValue
    //% block="set PID values KPTurn: %KPTurn KPDist: %KPDist KDTurn: %KDTurn KDDist: %KDDist"
    //% weight=50
    export function pidValue(KPTurn: number, KPDist: number, KDTurn: number, KDDist: number): void{
        _kp_value(KPTurn, KPDist);
        _kd_value(KDTurn, KDDist);
    }

    //% blockID:banana_SetAutoMode
    //% block="set tracking mode %enable"
    //% enable.shadow=toggleOnOff
    export function setAutoMode(EnableAuto: boolean): void{
        _set_auto_mode(EnableAuto);
    }

    //% blockID:banana_KalmanFilter
    //% block="set Kalman Filter r_measure: %r_measure_angle and %r_measure_dis q_measure_angle: %q_measures_angle_pos and %q_measures_angle_vel q_measure_distance: %q_measures_dis_pos and %q_measures_dis_vel"
    export function KalmanFilterValues(r_measure_angle: number,r_measure_dis: number, q_measures_angle_pos: number, q_measures_angle_vel: number, q_measures_dis_pos: number, q_measures_dis_vel: number): void{
        _KalmanFilterValuesAngle(r_measure_angle, q_measures_angle_pos, q_measures_angle_vel);
        _KalmanFilterValuesDistance(r_measure_dis, q_measures_dis_pos, q_measures_dis_vel);
    }


    //% block="get C++ latency (ms)"
    //% weight=10
    //% shim=banana::dt_time
    export function getLoopLatency(): number {
        return 0; // This is a dummy return; the C++ shim overrides it.
    }

}