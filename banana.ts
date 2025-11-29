
enum bananaMotor{
    M1 = 0,
    M2 = 1,
    M3 = 2,
    M4 = 3
}

enum bananDir{
    //% block="Forward"
    FORWARD = 1,
    //% block="Backward"
    BACKWARD = 2
}

// Icon unicode characters can be found at: http://fontawesome.io/icons/
//% color=#c2b711 weight=100 icon="\uf1ec" block="Banana Meeeeee" advanced=false
namespace banana {

    //% shim=banana::banana_init
    function _init(): void { return; }

    //% shim=banana::banana_set_motor
    function _set_motor(id: number, speed: number, dir: number): void { return; }

    //% blockId=banana_StartMotor
    //% block="start motor"
    export function startMotor(): void{
        _init();
    }

    //% blockId=banana_Run
    //% block="run motor %motor with speed %speed and direction %dir"
    //% speed.min=0 speed.max=255 speed.defl=100
    export function bananaRun(motor: bananaMotor, speed: number, dir: bananDir): void{
        _set_motor
    }

    //% blockId=banana_Stop
    //% block="stop motor %motor"
    export function bananaStop(motor: bananaMotor): void{
        _set_motor(motor, 0, 0)
    }
    
}