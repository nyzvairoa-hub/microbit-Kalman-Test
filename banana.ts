// Icon unicode characters can be found at: http://fontawesome.io/icons/
//% color=#c2b711 weight=100 icon="\uf1ec" block="Banana Meeeeee" advanced=false
namespace banana {

    //% blockId=banana_getAccelX
    //% block="get banana accel X"
    //% shim=banana::banana_getAccelX
    //% parts="accelerometer"
    export function getAccelX(): void{
        return;
    }

    //% blockId=banana_Run
    //% block="run banana test %alpha"
    //% alpha.min=0.0 alpha.max=1.0 alpha.defl=0.1
    //% shim=banana::banana_run
    //% parts="accelerometer"
    export function bananaRun(alpha: number): void{
        return;
    }

    //% blockId=banana_Stop
    //% block="stop banana test"
    //% shim=banana::banana_stop
    //% parts="accelerometer"
    export function bananaStop(): void{
        return;
    }
    
    //% blockId=banana_multPrint
    //% block="banana multiply and print %n | and %m"
    //% shim=banana::banana_multPrint
    export function bananaMultPrint(n: number, m: number): void{
        return;
    }
}
