#ifndef PV_KALMAN_H
#define PV_KALMAN_H

class PVKalman{
    public:
        // state variables
        float x;
        float v;

        // covarianve variable
        float p_xx;
        float p_vv;

        // parameter tuning
        float q_pos;
        float q_vel;
        float r_measure;

        PVKalman(float measurement_noise, float process_noise){
            // initial state
            x = 0;
            v = 0;

            // initial covariance
            p_xx = 100.0;
            p_vv = 100.0;

            // tuning parameters
            r_measure = measurement_noise;
            q_pos = process_noise;
            q_vel = process_noise;

        }

        void predict(float dt){
            // extrapolate state
            x += v * dt;

            // extrapolate uncertainty (covariance)
            p_xx += dt * dt * p_vv + q_pos;
            p_vv += q_vel;
        }

        void update(float measurement){
            // compute kalman gain
            float k_x = p_xx / (p_xx + r_measure);
            float k_v = p_vv / (p_vv + r_measure);

            // update state estimate
            // new = old + gain * (measured - old)
            float inno = measurement - x;
            x = x + k_x * inno;

            // update velocity based on position error
            v = v + k_v * inno;

            // update uncertainty
            p_xx = (1 - k_x) * p_xx;
            p_vv = (1 - k_v) * p_vv;
        }
};

#endif