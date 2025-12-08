banana.startMotor()
huskylens.initI2c()
huskylens.initMode(protocolAlgorithm.ALGORITHM_TAG_RECOGNITION)
basic.forever(function () {
    huskylens.request()
    if (huskylens.isAppear_s(HUSKYLENSResultType_t.HUSKYLENSResultBlock)) {
        banana.huskyLensData(
        huskylens.readeBox(1, Content1.xCenter),
        huskylens.readeBox(1, Content1.yCenter),
        huskylens.readeBox(1, Content1.width),
        huskylens.readeBox(1, Content1.height),
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
})
