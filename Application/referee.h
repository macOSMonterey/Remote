#ifndef REFEREE_H
#define REFEREE_H

#include "main.h"

#include "protocol.h"

typedef struct __attribute__((__packed__)) //0x0001
{
    float vx;
    float vy;
    float yaw;   // wz 物理含义是yaw角速度，名字保留wz也可以
} chassis_vel_t;


extern void init_referee_struct_data(void);
extern void referee_data_solve(uint8_t *frame);

void get_buffer_energy_and_shooter_barrel_heat(float *buffer_energy, float *shooter_barrel_heat);

extern uint8_t get_robot_id(void);

#endif
