banana.startMotor()
huskylens.initI2c()
huskylens.initMode(protocolAlgorithm.ALGORITHM_TAG_RECOGNITION)
basic.forever(function on_forever() {
    if (huskylens.isAppear_s(HUSKYLENSResultType_t.HUSKYLENSResultBlock)) {
        banana.huskyLensData(huskylens.readBox_s(Content3.xCenter), huskylens.readBox_s(Content3.yCenter), huskylens.readBox_s(Content3.width), huskylens.readBox_s(Content3.height), true)
    } else {
        banana.huskyLensData(0, 0, 0, 0, false)
    }
    
})
