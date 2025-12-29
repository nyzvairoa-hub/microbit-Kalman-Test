banana.start_motor()
huskylens.init_i2c()
huskylens.init_mode(protocolAlgorithm.ALGORITHM_TAG_RECOGNITION)

def on_forever():
    if huskylens.isAppear_s(HUSKYLENSResultType_t.HUSKYLENS_RESULT_BLOCK):
        banana.husky_lens_data(huskylens.readBox_s(Content3.X_CENTER),
            huskylens.readBox_s(Content3.Y_CENTER),
            huskylens.readBox_s(Content3.WIDTH),
            huskylens.readBox_s(Content3.HEIGHT),
            True)
    else:
        banana.husky_lens_data(0, 0, 0, 0, False)
basic.forever(on_forever)
