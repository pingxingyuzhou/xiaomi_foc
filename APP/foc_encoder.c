

#include "foc_encoder.h"
#include "usr_config.h"

tEncoder Encoder;

static inline uint16_t Encoder_read(void)
{
    uint16_t angle;
    SPI2_NSS_L;
    angle = SPI_RW_byte(2, AS5047_address) & 0x3fff;
    SPI2_NSS_H;
    return angle;
}
void ENCODER_sample(float dt)
{
    if (UsrConfig.calib_valid)
    {
        // dir swap
        if (UsrConfig.encoder_dir_rev)
        {
            Encoder.raw = ENCODER_CPR - Encoder_read();
        }
        else
        {
            Encoder.raw = Encoder_read();
        }

        // offset compensation
        int off_1 = UsrConfig.offset_lut[Encoder.raw >> 7];
        int off_2 = UsrConfig.offset_lut[((Encoder.raw >> 7) + 1) % 128];
        int off_interp = off_1 + ((off_2 - off_1) * (Encoder.raw - ((Encoder.raw >> 7) << 7)) >> 7); // Interpolate between lookup table entries
        int cnt = Encoder.raw - off_interp;                                                          // Correct for nonlinearity with lookup table from calibration
        if (cnt > ENCODER_CPR)
        {
            cnt -= ENCODER_CPR;
        }
        else if (cnt < 0)
        {
            cnt += ENCODER_CPR;
        }
        Encoder.cnt = cnt;
    }
    else
    {
        // dir swap
        if (UsrConfig.encoder_dir_rev)
        {
            Encoder.raw = ENCODER_CPR - Encoder_read();
        }
        else
        {
            Encoder.raw = Encoder_read();
        }
        Encoder.cnt = Encoder.raw;
    }

    // Turns
    if (Encoder.cnt - Encoder.cnt_last > ENCODER_CPR / 2)
    {
        Encoder.turns -= 1;
    }
    else if (Encoder.cnt - Encoder.cnt_last < -ENCODER_CPR / 2)
    {
        Encoder.turns += 1;
    }
    Encoder.cnt_last = Encoder.cnt;

    // Position
    Encoder.position = Encoder.turns + (float)Encoder.cnt / (float)ENCODER_CPR;

    // Elec angle
    Encoder.elec_angle = (UsrConfig.motor_pole_pairs * (float)(Encoder.cnt - UsrConfig.encoder_offset)) / ((float)ENCODER_CPR);
    Encoder.elec_angle = 2.0f * PI * (Encoder.elec_angle - (int)Encoder.elec_angle);
    Encoder.elec_angle = Encoder.elec_angle < 0 ? Encoder.elec_angle + 2.0f * PI : Encoder.elec_angle;

    float vel = (Encoder.position - Encoder.position_last) / dt;
    Encoder.position_last = Encoder.position;

    float sum = vel;
    for (int i = 1; i < VEL_VEC_SIZE; i++)
    {
        Encoder.vel_vec[VEL_VEC_SIZE - i] = Encoder.vel_vec[VEL_VEC_SIZE - i - 1];
        sum += Encoder.vel_vec[VEL_VEC_SIZE - i];
    }
    Encoder.vel_vec[0] = vel;

    Encoder.velocity = sum / ((float)VEL_VEC_SIZE);
    Encoder.velocity_elec = 2.0f * PI * Encoder.velocity * UsrConfig.motor_pole_pairs;
}



