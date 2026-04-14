#ifndef PV_KALMAN_H
#define PV_KALMAN_H

class PVKalman{
public:
    // x = [pos, vel]^-1
    float pos;
    float vel;

    // P = [P00 P01
    //     P10 P11]
    float P00; 
    float P01;
    float P10;
    float P11;

    float Q_pos;
    float Q_vel;
    float R_measure;

    PVKalman(float measurment_noise, float process_noise_pos, float process_noise_vel){
        // initial State
        pos = 0;
        vel = 0;

        P00 = 0.0f;
        P01 = 0.0f;
        P10 = 0.0f;
        P11 = 0.0f;

        Q_pos = process_noise_pos;
        Q_vel = process_noise_vel;
        R_measure = measurment_noise;
    }

    void predict(float dt){
        pos = pos + (vel * dt);
        vel = vel; // constant velocity model

        // predict state covariance
        // P = A * P * A^T + Q
        P00 = P00 + (dt * (P01 + P10)) + (dt* dt * P11) + Q_pos;
        P01 = P01 + (dt * P11);
        P10 = P10 + (dt * P11);
        P11 = P11 + Q_vel;
    }

    void update(float measurement){
        // H = [1 0]
        // S = H * P * H^T + R
        float S = P00 + R_measure;

        // K = P * H^T * S^-1
        // K = [K_pos K_vel]^T
        float K_pos = P00 / S;
        float K_vel = P01 / S;

        // Innovation: y = z - H * x
        float y = measurement - pos;

        // Update state: x = x + K * y
        pos = pos + (K_pos * y);
        vel = vel + (K_vel * y);

        float P00_old = P00;
        float P01_old = P01;

        P00 = P00 - (K_pos * P00_old);
        P01 = P01 - (K_pos * P01_old);
        P10 = P01;
        P11 = P11 - (K_vel * P01_old);
    }
};

#endif