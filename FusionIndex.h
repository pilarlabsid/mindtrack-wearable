#ifndef FUSION_INDEX_H
#define FUSION_INDEX_H

class FusionIndex {

public:

    FusionIndex();

    float compute(
        float hr,
        float hrv,
        float gsr,
        float temperature,
        float ax,
        float ay,
        float az
    );

private:

    // filtering
    float movingAverage(float value, float *buffer);

    // normalization
    float normalize(float value, float min, float max);

    // net motion magnitude (gravitasi sudah dikurangi)
    float motionNet(float ax, float ay, float az);

    // adaptive weight
    void adaptiveWeights(
        float motion,
        float &w_hr,
        float &w_hrv,
        float &w_gsr,
        float &w_motion,
        float &w_temp
    );

    // fusion
    float fuse(
        float hr,
        float hrv,
        float gsr,
        float motion,
        float temp,
        float w_hr,
        float w_hrv,
        float w_gsr,
        float w_motion,
        float w_temp
    );

    // kalman
    float kalman(float measurement);

    // smoothing
    float smooth(float current);

    // buffers
    float hr_buffer[5];
    float hrv_buffer[5];
    float gsr_buffer[5];
    float temp_buffer[5];
    float motion_buffer[5];

    int index;

    float kalman_estimate;
    float kalman_error;

    float smooth_prev;
};

#endif