#ifndef CHASSIS_TASK_H
#define CHASSIS_TASK_H

#include "CAN_receive.h"
#include "main.h"
#include "remote_control.h"


//Chassis motor CAN ID offset
//usage: (Chassis_t)chassis->motor[FRONT_LEFT] 
#define FRONT_RIGHT 1
#define FRONT_LEFT 2
#define BACK_LEFT 3
#define BACK_RIGHT 4

// RC channels
#define RC_X 2
#define RC_Y 1
#define RC_Z 0


typedef struct 
{
    const motor_measure_t *motor_raw;
    
    //Current speed read from motors
    uint16_t pos_raw;
    uint16_t speed_raw;
    uint16_t current_raw;
    
    //Target speed set by user/remote control
    uint16_t pos_set;
    uint16_t speed_set;
    
    //Final output speed
    uint16_t speed_out;
} Chassis_Motor_t;


typedef struct 
{
    Chassis_Motor_t motor[4];
    
    //Raw remote control data
    const RC_ctrl_t *rc_raw;
    
    //Current front vector
    const fp32 *vec_raw;
    const fp32 *yaw_pos_raw;
    
    //Current speed, vector combination of the speed read from motors
    uint16_t x_speed_raw;
    uint16_t y_speed_raw;
    uint16_t z_speed_raw;
    
    //Speed set by user/remote control
    uint16_t x_speed_set;
    uint16_t y_speed_set;
    uint16_t z_speed_set;
} Chassis_t;


typedef enum{
    CHASSIS_VECTOR_RAW,
    CHASSIS_FOLLOW_GIMBAL_YAW,
    CHASSIS_INDIVIDUAL_CONTROL,
} chassis_user_mode_e;



extern void chassis_task(void *pvParameters);
extern uint8_t chassis_init(Chassis_t *chassis_init);
extern Chassis_t* get_chassis_point(void);

#endif
	
