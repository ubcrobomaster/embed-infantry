/**
* Gimbal task 
* Converts user input to Gimbal motor setpoints
*
* Gimbal position is defined by angles phi and theta
* Theta increases counterclockwise if viewed from the top and is 0 when facing fwd.
* Phi increases counterclockwise if viewed from the right side and is 0 when horizontal
* (when Theta is 0)
*
* Gimbal control strategy and either be gyro based or encoder based 
* RM motors give encoders wrap around at a half rotation negative and positive
* Encoders results are between 0 and 8191
*/


#include "gimbal_task.h"
#include "stm32f4xx.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "start_task.h"


/******************** User Includes ********************/
#include "CAN_Receive.h"
#include "user_lib.h"
#include "remote_control.h"
#include "USART_comms.h"
#include <stdio.h>

float rc_channel_1;
float rc_channel_2;
const RC_ctrl_t* rc_ptr;
Gimbal_t gimbal;

//UART mailbox
char str[32] = {0};


/**
 * @brief Initialises PID and fetches Gimbal motor data to ensure 
 *  gimbal points in direction specified. 
 * @param FreeRTOS parameters
 * @retval None
 */
void gimbal_task(void* parameters){

    vTaskDelay(GIMBAL_TASK_INIT_TIME);
	
    gimbal.pitch_motor->gimbal_motor_raw = get_Pitch_Gimbal_Motor_Measure_Point();
    gimbal.yaw_motor->gimbal_motor_raw = get_Yaw_Gimbal_Motor_Measure_Point();

    fp32 pid_constants[3] = {pid_kp, pid_ki, pid_kd};
    PidTypeDef pid_pitch;
    PID_Init(&pid_pitch, PID_POSITION, pid_constants, max_out, max_iout);

    PidTypeDef pid_yaw;
    PID_Init(&pid_yaw, PID_POSITION, pid_constants, max_out, max_iout);
    

    rc_ptr = get_remote_control_point();
    
    while(1){	
        /* For now using strictly encoder feedback for position */
     
        
        // Get CAN received data
        gimbal.pitch_pos_raw = gimbal.pitch_motor->gimbal_motor_raw->ecd;
        gimbal.pitch_speed_raw = gimbal.pitch_motor->gimbal_motor_raw->speed_rpm;

        gimbal.yaw_pos_raw = gimbal.yaw_motor->gimbal_motor_raw->ecd;
        gimbal.yaw_speed_raw = gimbal.yaw_motor->gimbal_motor_raw->speed_rpm;

        // Calculate setpoints based on RC signal.
        if(switch_is_mid(rc_ptr->rc.s[0]) || switch_is_up(rc_ptr->rc.s[0])){
            gimbal.yaw_pos_set = linear_map_int_to_int(rc_ptr->rc.ch[2], RC_MIN, RC_MAX, YAW_MAX, YAW_MIN);
            gimbal.pitch_pos_set = linear_map_int_to_int(rc_ptr->rc.ch[3], RC_MIN, RC_MAX, PITCH_MAX, PITCH_MIN);
        }else{
            gimbal.yaw_pos_set = linear_map_int_to_int(0, RC_MIN, RC_MAX, YAW_MAX, YAW_MIN);
            gimbal.pitch_pos_set = linear_map_int_to_int(0, RC_MIN, RC_MAX, PITCH_MAX, PITCH_MIN);
        }
        
        gimbal.pitch_speed_set = PID_Calc(&pid_pitch, gimbal.pitch_motor->pos_raw, gimbal.pitch_pos_set);
        gimbal.yaw_speed_set = PID_Calc(&pid_yaw, gimbal.yaw_motor->pos_raw, gimbal.yaw_pos_set);

        // Turn gimbal motor
        CAN_CMD_GIMBAL(gimbal.yaw_speed_set, gimbal.pitch_speed_set, 0, 0);
        
        vTaskDelay(GIMBAL_TASK_DELAY);
        
        //send_to_uart(gimbal_pitch_motor, pid_pitch, pitch_signal);  //Sending data via UART
	}
}




/** 
 * @brief  Reads vision instruction from UART and cap to certin values
 * @param  None
 * @retval Vision signal in range of 0 and 8191
 */
int get_vision_signal(void) {
    int vision_signal = 5700;  // TODO: Get real values from vision
        
    while (vision_signal > 8191) {
        vision_signal -= 8191;
    }
    while (vision_signal < 0) {
        vision_signal+= 8192;
    }
    return vision_signal;
}


/** 
 * @brief Updates Uart with position information on the yaw motor and the PID settings
 * This will block for several milliseconds
 * @param gimbal_yaw_motor struct containing information about the gimbal yaw motor
 * @param pid struct containing pid coefficients
 * @param pitch_signal signal to pitch motor
 * @retval None
 */
void send_to_uart(Gimbal_Motor_t gimbal_yaw_motor, PidTypeDef pid_pitch, fp32 pitch_signal) 	
{
    char str[20]; //uart data buffer

    sprintf(str, "position: %d\n\r", gimbal.yaw_motor->pos_raw);
    serial_send_string(str);

    sprintf(str, "speed: %d\n\r", gimbal.yaw_motor->speed_raw);
    serial_send_string(str);       

    sprintf(str, "current: %d\n\r", gimbal.yaw_motor->current_raw);
    serial_send_string(str);

    sprintf(str, "motor kp: %f\n\r", pid_pitch.Kp);
    serial_send_string(str);

    sprintf(str, "motor kd: %f\n\r", pid_pitch.Kd);
    serial_send_string(str);    

    sprintf(str, "motor ki: %f\n\r", pid_pitch.Ki);
    serial_send_string(str);
    
    sprintf(str, "pitch signal: %f\n\r", pitch_signal);
    serial_send_string(str);
}
