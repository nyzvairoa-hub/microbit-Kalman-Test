input.onButtonPressed(Button.A, function () {
    basic.pause(2000)
    banana.setAutoMode(true)
    banana.startMotor()
    while (true) {
        now = control.millis()
        dt_actual = now - lastTime
        lastTime = now
        // Approx time step for blocks
        huskylens.request()
        if (huskylens.isAppear_s(HUSKYLENSResultType_t.HUSKYLENSResultBlock)) {
            banana.huskyLensData(
            huskylens.readBox_s(Content3.xCenter),
            huskylens.readBox_s(Content3.yCenter),
            huskylens.readBox_s(Content3.width),
            huskylens.readBox_s(Content3.height),
            true
            )
        } else {
            banana.huskyLensData(
            0,
            0,
            0,
            0,
            false
            )
        }
    }
})
let dt_actual = 0
let now = 0
let lastTime = 0
lastTime = control.millis()
banana.pidValue(
0.5,
2,
0.5,
0.5
)
banana.KalmanFilterValues(
1.368948849,
0.04624,
1,
3,
0.1,
0.1
)
huskylens.initI2c()
huskylens.initMode(protocolAlgorithm.ALGORITHM_TAG_RECOGNITION)
