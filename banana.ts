
enum bananaMotor{
    M1 = 0,
    M2 = 1,
    M3 = 2,
    M4 = 3
}

// Icon unicode characters can be found at: http://fontawesome.io/icons/
//% color=#c2b711 weight=100 icon="\uf1ec" block="Robot Smooth" advanced=false
namespace banana {

    //% shim=banana::banana_init
    function _init(): void { return; }

    //% shim=banana::banana_set_motor
    function _set_motor(id: number, speed: number): void { return; }

    //% shim=banana::husky_pos
    function _husky_pos(x: number, y: number): void { return; }

    //% shim=banana::husky_size
    function _husky_size(width: number, height: number, isDetected: boolean): void { return; }

    //% shim=banana::pid_value
    function _pid_value(KPTurn: number, KPDist: number): void { return; }

    //% shim=banana::set_auto_mode
    function _set_auto_mode(EnableAuto: boolean): void { return; }

    //% shim=banana::KalmanFilterValues
    function _KalmanFilterValues(r_measure: number, q_measure: number): void { return; }

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
    //% block="set PID values KPTurn: %KPTurn KPDist: %KPDist"
    //% weight=50
    export function pidValue(KPTurn: number, KPDist: number): void{
        _pid_value(KPTurn, KPDist);
    }

    //% blockID:banana_SetAutoMode
    //% block="set tracking mode %enable"
    //% enable.shadow=toggleOnOff
    export function setAutoMode(EnableAuto: boolean): void{
        _set_auto_mode(EnableAuto);
    }

    //% blockID:banana_KalmanFilter
    //% block="set Kalman Filter r_measure: %r_measure q_measure: %q_measure"
    export function KalmanFilterValues(r_measure: number, q_measure: number): void{
        _KalmanFilterValues(r_measure, q_measure);
    }
}