#ifndef __FOC_ENCODER_H
#define __FOC_ENCODER_H

#include "main.h"

#define AS5047_address 0xffff
#define ENCODER_CPR 16383
#define ENCODER_CPR_DIV_2 (ENCODER_CPR >> 1)
#define VEL_VEC_SIZE 40

typedef struct sEncoder
{
    uint16_t raw;
    int cnt;
    int cnt_last;
    int turns;
    float position;
    float position_last;
    float elec_angle;
    float velocity;
    float velocity_elec;

    float vel_vec[VEL_VEC_SIZE];
} tEncoder;

extern tEncoder Encoder;

void ENCODER_sample(float dt);

float InitEnc_with_Hall(float Zeroangel, float Realangle, uint16_t *hall_buf);

#endif
