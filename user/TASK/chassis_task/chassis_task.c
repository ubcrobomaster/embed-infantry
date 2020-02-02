#include "chassis_task.h"
#include "main.h"
#include "stm32f4xx.h"
#include <stdio.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "led.h"
#include "CAN_Receive.h"
#include "delay.h"
#include "USART_comms.h"
#include "INS_task.h"
#include "remote_control.h"
#include "pid.h"


static uint8_t chassis_init(Chassis_t *chassis_init);
static void chassis_update_data(Chassis_t *chassis_update);
static void chassis_set_mode(void);
static void chassis_remote_calc(chassis_user_mode_e mode);
//static void chassis_set_mode(void);
static void chassis_motor_calc(void);
static void chassis_PID(void);


static Chassis_t chassis;
    
fp32 def_pid_constants[3] = {0.02, 0.0, 0.1};

Chassis_t* get_chassis_point() {
    return &chassis;
}

void chassis_task(void *pvParameters){
    //Initializes chassis
    while(!chassis_init(&chassis)){
    }
    
	while(1) {
		    //update info
				chassis_update_data(&chassis);
        //set mode
        //chassis_set_mode();
        //process RC data into xyz speed
        chassis_remote_calc(CHASSIS_VECTOR_RAW);
        //process xyz speed into vector decomposition 
        chassis_motor_calc();
        //PID calculations, process 
        chassis_PID();
        //output
        CAN_CMD_CHASSIS(chassis.motor[FRONT_RIGHT].speed_out, chassis.motor[FRONT_LEFT].speed_out, 
            chassis.motor[BACK_LEFT].speed_out, chassis.motor[BACK_RIGHT].speed_out);
        
        vTaskDelay(1);
    }
}

static void chassis_update_data(Chassis_t *chassis_update){
		for (int i = 0; i < 4; i++) {
        chassis_update->motor[i].speed_raw = chassis_update->motor[i].motor_raw->speed_rpm;
        chassis_update->motor[i].pos_raw = chassis_update->motor[i].motor_raw->ecd;
        chassis_update->motor[i].current_raw = chassis_update->motor[i].motor_raw->given_current;
		}
}

/**
 * @brief Starts chassis motors and peripherals, initializes a front vector
 * @param
 * @retval TRUE if init is completed
 */
static uint8_t chassis_init(Chassis_t *chassis_init){
    //Forces motors to reset ID
    CAN_CMD_CHASSIS_RESET_ID();
    
    //Link pointers with CAN motors
    for (int i = 0; i < 4; i++) {
        chassis_init->motor[i].motor_raw = get_Chassis_Motor_Measure_Point(i);
        chassis_init->motor[i].speed_raw = chassis_init->motor[i].motor_raw->speed_rpm;
        chassis_init->motor[i].pos_raw = chassis_init->motor[i].motor_raw->ecd;
        chassis_init->motor[i].current_raw = chassis_init->motor[i].motor_raw->given_current;
			
				PID_Init(&chassis_init->motor[i].pid_control, PID_DELTA, def_pid_constants, 1000.0, 50.0);
    }
    
    //Init yaw and front vector
    chassis_init->vec_raw = get_INS_angle_point();
    chassis_init->yaw_pos_raw = chassis_init->vec_raw + INS_YAW_ADDRESS_OFFSET;
		
    //Pointer remote
    chassis_init->rc_raw = get_remote_control_point();
    
    return TRUE;
}


/**
 * @brief Based on the mode of operation, remote control data is processed
 * @param 
 * @retval 
 */

/*static void chassis_set_mode(void){
	//Don't do anything
}*/


/**
 * @brief Takes the remote control data and converts as specified by user
 * @param  A value from the enum chassis_user_mode_e
 * @retval 
 */

static void chassis_remote_calc(chassis_user_mode_e mode){
    //Get remote control data and put into x_speed_raw etc
    //process based on mode (which is currently none) and put into x_speed_set
    //Debug print out current 
    
    // get rc data and put into chassis struct
    chassis.x_speed_raw = chassis.rc_raw->rc.ch[RC_X];
    chassis.y_speed_raw = chassis.rc_raw->rc.ch[RC_Y];
    chassis.z_speed_raw = chassis.rc_raw->rc.ch[RC_Z];
    
    // process raw sppeds based on modes (for now they are just the same)
    chassis.x_speed_set = chassis.x_speed_raw;
    chassis.y_speed_set = chassis.y_speed_raw;
    chassis.z_speed_set = chassis.z_speed_raw;
    
}


/**
 * @brief Handles the data from remote controller
 * @param 
 * @retval 
 */
static void chassis_motor_calc(void){
	//Take x_speed_set etc and handle mechanum wheels
    //Put results into Chassis_Motor_t speed_set (and/or pos_set)
    chassis.motor[FRONT_LEFT].speed_set = MULTIPLIER * (chassis.y_speed_set + chassis.z_speed_set + chassis.x_speed_set);
    chassis.motor[BACK_LEFT].speed_set = MULTIPLIER * (chassis.y_speed_set + chassis.z_speed_set - chassis.x_speed_set);
    chassis.motor[FRONT_RIGHT].speed_set = MULTIPLIER * (-chassis.y_speed_set + chassis.z_speed_set + chassis.x_speed_set);
    chassis.motor[BACK_RIGHT].speed_set = MULTIPLIER * (-chassis.y_speed_set + chassis.z_speed_set - chassis.x_speed_set);
    
    /*volatile char str[32];
    sprintf((char*) str, "x: %d\n\r", chassis.x_speed_set);
    serial_send_string(str);
    sprintf((char*) str, "y: %d\n\r", chassis.y_speed_set);
    serial_send_string(str);
    sprintf((char*) str, "z: %d\n\r", chassis.z_speed_set);
    serial_send_string(str);
    sprintf((char*) str, "sum: %d\n\r", (chassis.x_speed_set + chassis.y_speed_set + chassis.z_speed_set));
    serial_send_string(str);*/
}

/**
 * @brief PID calculations for motors, both in translation and rotation 
 * @param 
 * @retval 
 */
char pid_out[64];
fp32 front_right;
fp32 back_right;
fp32 front_left;
fp32 back_left;

static void chassis_PID(void){
	//Don't worry about this for now
	//translation
  //rotation
	//get velocity
	//pid calc
	//set speed_out
	//FRONT RIGHT
	front_right = PID_Calc(&chassis.motor[FRONT_RIGHT].pid_control, 
	chassis.motor[FRONT_RIGHT].speed_raw,
	chassis.motor[FRONT_RIGHT].speed_set);
	chassis.motor[FRONT_RIGHT].speed_out += front_right;
	
	back_right = PID_Calc(&chassis.motor[BACK_RIGHT].pid_control, 
	chassis.motor[BACK_RIGHT].speed_raw, 
	chassis.motor[BACK_RIGHT].speed_set);
	chassis.motor[BACK_RIGHT].speed_out += back_right;
	
	front_left = PID_Calc(&chassis.motor[FRONT_LEFT].pid_control, 
	chassis.motor[FRONT_LEFT].speed_raw, 
	chassis.motor[FRONT_LEFT].speed_set);
	chassis.motor[FRONT_LEFT].speed_out += front_left;
	
	back_left = PID_Calc(&chassis.motor[BACK_LEFT].pid_control, 
	chassis.motor[BACK_LEFT].speed_raw, 
	chassis.motor[BACK_LEFT].speed_set);
	chassis.motor[BACK_LEFT].speed_out += back_left;
	
	sprintf(pid_out, "Front Right - target: %d, sensor: %d, output: %d \n\r", 
	chassis.motor[FRONT_RIGHT].speed_set, 
	chassis.motor[FRONT_RIGHT].speed_raw, 
	chassis.motor[FRONT_RIGHT].speed_out);
	serial_send_string(pid_out);
	
	sprintf(pid_out, "Back Right - target: %d, sensor: %d, output: %d \n\r", 
	chassis.motor[BACK_RIGHT].speed_set, 
	chassis.motor[BACK_RIGHT].speed_raw, 
	chassis.motor[BACK_RIGHT].speed_out);
	serial_send_string(pid_out);
	
	sprintf(pid_out, "Front Left - target: %d, sensor: %d, output: %d \n\r", 
	chassis.motor[FRONT_LEFT].speed_set, 
	chassis.motor[FRONT_LEFT].speed_raw, 
	chassis.motor[FRONT_LEFT].speed_out);
	serial_send_string(pid_out);
	
	sprintf(pid_out, "Back Left - target: %d, sensor: %d, output: %d \n\r", 
	chassis.motor[BACK_LEFT].speed_set, 
	chassis.motor[BACK_LEFT].speed_raw, 
	chassis.motor[BACK_LEFT].speed_out);
	serial_send_string(pid_out);
	
}
