#ifndef PV_KALMAN_H
#define PV_KALMAN_H

#include <math.h>

class PVKalman{
    public:
        // state variables
        float x;
        float v;

        // covarianve variable
        float p_xx;
        float p_vv;
        float p_xv;

        // parameter tuning
        float q_pos;
        float q_vel;
        float r_measure;

        PVKalman(float measurement_noise, float process_noise){
            // initial state
            x = 160;
            v = 0;

            // initial covariance
            p_xx = 100.0;
            p_vv = 100.0;
            p_xv = 0.0;

            // tuning parameters
            r_measure = measurement_noise;
            q_pos = process_noise;
            q_vel = process_noise;

        }

        void predict(float dt){
            // extrapolate state
            x += v * dt;

            // extrapolate uncertainty (covariance)
            p_xx += 2 * p_xv * dt + p_vv * dt * dt + q_pos;
            p_xv += p_vv * dt;
            p_vv += q_vel; // q_vel is the process noise
        }

        void update(float measurement){
            // compute kalman gain
            float k_x = p_xx / (p_xx + r_measure);
            float k_v = p_xv / (p_xx + r_measure);

            // update state estimate
            // new = old + gain * (measured - old)
            float inno = measurement - x;
            x = x + k_x * inno;

            // update velocity based on position error
            v = v + k_v * inno;

            float p_xx_old = p_xx;
            float p_xv_old = p_xv;

            // update covariance
            p_xx = (1.0f - k_x) * p_xx_old;
            p_xv = (1.0f - k_x) * p_xv_old;
            p_vv = p_vv - (k_v * p_xv_old);

        }
};

#endif