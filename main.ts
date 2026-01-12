input.onButtonPressed(Button.A, function () {
    basic.pause(2000)
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
            datalogger.log(
            datalogger.createCV("Target", 100),
            datalogger.createCV("Actual", huskylens.readBox_s(Content3.width))
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
banana.startMotor()
banana.setAutoMode(true)
banana.pidValue(0.4, 2.5)
huskylens.initI2c()
huskylens.initMode(protocolAlgorithm.ALGORITHM_TAG_RECOGNITION)
