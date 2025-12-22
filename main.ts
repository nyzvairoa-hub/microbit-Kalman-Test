let y = 0
let x = 0
banana.startMotor()
huskylens.initI2c()
huskylens.initMode(protocolAlgorithm.ALGORITHM_TAG_RECOGNITION)
basic.forever(function () {
    huskylens.request()
    if (huskylens.isAppear_s(HUSKYLENSResultType_t.HUSKYLENSResultBlock)) {
        x = huskylens.readeBox(1, Content1.width)
        y = huskylens.readeBox(1, Content1.yCenter)
        banana.huskyLensData(x, y, true)
    } else {
        banana.huskyLensData(-1, -1, false)
    }
})
