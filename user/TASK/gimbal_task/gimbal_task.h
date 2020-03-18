/*
* Header file for gimbal_task.c
* Holds tuning constants and unit conversions
*/

#ifndef GIMBAL
#define GIMBAL
#include "main.h"
#include "CAN_receive.h"
#include "pid.h"
#include "shoot_task.h"
#include "remote_control.h"
#include "USART_comms.h"

/******************************* Task Delays *********************************/
#define GIMBAL_TASK_DELAY 1
#define GIMBAL_INIT_DELAY 300


/*************** Converts between motor position and degrees *****************/
#define Motor_Ecd_to_Rad 0.000766990394f /* 2PI / 8191 */
#define FALSE 0
#define TRUE 1


/****************************** PID Constants ********************************/
#define pid_kp_yaw 250.0f
#define pid_ki_yaw 0.0f
#define pid_kd_yaw 250.0f
#define max_out_yaw 15000.0f
#define max_i_term_out_yaw 1000.0f

#define pid_kp_pitch 40.0f
#define pid_ki_pitch 0.0f
#define pid_kd_pitch 0.0f
#define max_out_pitch 5000.0f
#define max_i_term_out_pitch 1000.0f

/***************************** Gimbal Constants *****************************/
#define GIMBAL_TASK_INIT_TIME 300
#define CONTROL_TIME 1
#define RC_MIN -660
#define RC_MAX 660
#define ENCODER_MIN 0
#define ENCODER_MAX 8191
#define YAW_MIN 2359
#define YAW_MAX 6576
#define PITCH_MIN 2020
#define PITCH_MAX 3000
#define ERROR_MULTIPLIER 2048

#define BUFFER_SIZE 100

/************************** Gimbal Data Structures ***************************/
typedef struct 
{
	const motor_measure_t *motor_feedback;
	uint16_t pos_read;
	int16_t speed_read;
	int16_t current_read;

    int16_t pos_set;
    int16_t current_out; // TODO: update to voltage out

    PidTypeDef pid_controller;
} Gimbal_Motor_t;

typedef struct 
{
    const RC_ctrl_t *rc_update;
	Gimbal_Motor_t yaw_motor;
	Gimbal_Motor_t pitch_motor;
    const fp32 *angle_update;
	const fp32 *gyro_update;
	const fp32 *accel_update;
    const uint32_t current_angle;
    // TODO: Add gimbal angles when we care about orientation of robot in 3-d space
    
    fp32 yaw_setpoint[2]; // {real, imaj}
    fp32 yaw_position[2]; // {real, imaj}
    fp32 yaw_error;
    
    Shoot_t *launcher;
} Gimbal_t;

/////////////////////////
typedef struct
{
    uint8_t new_angle_flag;
    uint16_t yaw_angle;
    uint16_t pitch_angle;
    uint8_t count_rx;
    char buffer_rx[BUFFER_SIZE];
} Gimbal_Angles;


/******************************* Function Declarations ***********************/
int get_vision_signal(void);
extern void gimbal_task(void *pvParameters);
extern void send_to_uart(Gimbal_t *gimbal); 
void send_vision_angle();

// get a pointer to the gimbal struct
extern const Gimbal_Angles* get_gimbal_angle_struct(void);

/******************************* Variable Declarations ***********************/
// These will later be produced from RC
/* angle in radians*/


#endif
