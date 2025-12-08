enum bananaMotor{ M1=0, M2=1, M3=2, M4=3 }
enum bananaDir{
    //% block="Forward"
    FORWARD = 1,
    //% block="Backward"
    BACKWARD = 2
}

//% color=#c2b711 weight=100 icon="\uf1ec" block="Robot Smooth"
namespace banana {

    //% shim=banana::banana_init
    function _init(): void { return; }

    //% shim=banana::banana_set_motor
    function _set_motor(id: number, speed: number, dir: number): void { return; }

    //% shim=banana::husky_lens_data
    function _husky_lens_data(x: number, y: number, width: number, height: number, isDetected: boolean): void { return; }

    //% blockId=banana_StartMotor block="start motor system"
    export function startMotor(): void{ _init(); }

    //% blockId=banana_Run block="run motor %motor speed %speed direction %dir"
    export function bananaRun(motor: bananaMotor, speed: number, dir: bananaDir): void{
        _set_motor(motor, speed, dir);
    }

    //% blockId=banana_Stop block="stop motor %motor"
    export function bananaStop(motor: bananaMotor): void{
        _set_motor(motor, 0, 0);
    }
    
    //% blockID:banana_HuskyLensData
    //% block="feed vision: x %x y %y width %width height %height detected %isDetected"
    export function feedVisionData(x: number, y: number, width: number, height: number, isDetected: boolean): void{
        _husky_lens_data(x, y, width, height, isDetected);
    }
}