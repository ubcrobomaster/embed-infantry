#include "test_task.h"
#include "main.h"
#include "pid.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "led.h"
#include "USART_comms.h"

#define pid_kp 4
#define pid_ki 0.01
#define pid_kd 0.5
#define max_out 15000
#define max_iout 0

Gimbal_Motor_t gimbal_pitch_motor;

void testTask(void *pvParameters)
{
    gimbal_pitch_motor.gimbal_motor_raw = get_Pitch_Gimbal_Motor_Measure_Point();
    
    fp32 pitch_signal;
    
    int vision_signal;

    PidTypeDef pid;
    fp32 pid_constants[3] = {pid_kp, pid_ki, pid_kd};
    PID_Init(&pid, PID_POSITION, pid_constants, max_out, max_iout);
    
    while(1) 
    {
        led_green_toggle();

        // Get CAN received data
        gimbal_pitch_motor.gimbal_pos_raw = gimbal_pitch_motor.gimbal_motor_raw->ecd;
        gimbal_pitch_motor.gimbal_speed_raw = gimbal_pitch_motor.gimbal_motor_raw->speed_rpm;
        gimbal_pitch_motor.gimbal_tq_current_raw = gimbal_pitch_motor.gimbal_motor_raw->given_current;

        vision_signal = get_vision_signal();
        
        pitch_signal = PID_Calc(&pid, gimbal_pitch_motor.gimbal_pos_raw, vision_signal);

        // Turn gimbal motor
        CAN_CMD_GIMBAL(0, pitch_signal, 0, 0);

        //Sending data via UART
        send_to_uart(gimbal_pitch_motor, pid, pitch_signal);			
        
    }
}

int truncate_pid_signal(int signal) {
     while (signal > 8191) {
        signal -= 8191;
    }
    while (signal < 0) {
        signal+= 8192;
    }
    return signal;
}

int get_vision_signal() {
    int vision_signal = 3000;  // TODO: Get real values from vision
    int output_signal = truncate_pid_signal(vision_signal);
    return output_signal;
}

void send_to_uart(Gimbal_Motor_t gimbal_yaw_motor, PidTypeDef pid, fp32 pitch_signal) 	
{
    char str[20]; //uart data buffer

    sprintf(str, "position: %d\n\r", gimbal_yaw_motor.gimbal_pos_raw);
    Serial_sendString(str);

    sprintf(str, "speed: %d\n\r", gimbal_yaw_motor.gimbal_speed_raw);
    Serial_sendString(str);        

    sprintf(str, "current: %d\n\r", gimbal_yaw_motor.gimbal_tq_current_raw);
    Serial_sendString(str); 

    sprintf(str, "motor kp: %f\n\r", pid.Kp);
    Serial_sendString(str);

    sprintf(str, "motor kd: %f\n\r", pid.Kd);
    Serial_sendString(str);    

    sprintf(str, "motor ki: %f\n\r", pid.Ki);
    Serial_sendString(str);
    
    sprintf(str, "pitch signal: %f\n\r", pitch_signal);
    Serial_sendString(str);
}
