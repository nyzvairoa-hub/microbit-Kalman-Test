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
    //% block="run banana test with speed $speed | and direction $dir"
    //% speed.min=0 speed.max=255 speed.defl=100 
    //% dir.min=1 dir.max=2 dir.defl=1 
    //% shim=banana::banana_run
    export function bananaRun(speed: number, dir: number): void{
        return;
    }

    //% blockId=banana_Stop
    //% block="stop banana test"
    //% shim=banana::banana_stop
    export function bananaStop(): void{
        return;
    }
    
    //% blockId=banana_multPrint
    //% block="banana multiply and print $n | and $m"
    //% shim=banana::banana_multPrint
    export function bananaMultPrint(n: number, m: number): void{
        return;
    }
}
