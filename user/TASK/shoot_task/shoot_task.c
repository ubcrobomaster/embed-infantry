/**
  ******************************************************************************
    * @file    TASK/shoot_task
    * @date    24-January/2020
    * @brief   Shooting controls
    * @attention Shoot motors are plugged into pins A8 and E14, order unknown (update this if you find out)
  ******************************************************************************
**/

#include "shoot_task.h"
#include "main.h"
#include "stm32f4xx.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"


/******************** User Includes ********************/

#include "remote_control.h"
#include "CAN_Receive.h"
#include "user_lib.h"
#include "fric.h"

shoot_t shoot;

static uint16_t shoot_init(void);
static void shoot_control_loop(void);

/******************** Task/Functions Called from Outside ********************/

void shoot_task(void *pvParameters){
    while(!shoot_init()) {

    }

    while(1) {
        shoot_control_loop();
    }
    
}

static uint16_t shoot_init(void) {
    //Get RC pointers
    shoot.rc = get_remote_control_point();

    //Initialize ramp
    ramp_init(&shoot.ramp1, RAMP_PRD, Fric_DOWN, Fric_OFF);
    ramp_init(&shoot.ramp2, RAMP_PRD, Fric_DOWN, Fric_OFF);

    //Set pwm motor to zero
    shoot.fric1_pwm = Fric_OFF;
    shoot.fric2_pwm = Fric_OFF;

    return TRUE;
}


static void shoot_control_loop(void) {
    //Add remote control stuff later
    //Currently running SHOOT_ON fully
    shoot.fric1_pwm = Fric_OFF;
    shoot.fric2_pwm = Fric_OFF;

    //Turning the motors on one by one
    ramp_calc(&shoot.ramp1, SHOOT_FRIC_PWM_ADD_VALUE);
    if(shoot.ramp1.out == SHOOT_FRIC_PWM_ADD_VALUE) {
        ramp_calc(&shoot.ramp2, SHOOT_FRIC_PWM_ADD_VALUE);
    }

    //Sets the ramp maximum, ramp will update to reach this value
    shoot.ramp1.max_value = Fric_DOWN;
    shoot.ramp2.max_value = Fric_DOWN;

    //Sets pwm output to be the ramp value
    shoot.fric1_pwm = (uint16_t)(shoot.ramp1.out);
    shoot.fric2_pwm = (uint16_t)(shoot.ramp2.out);

    //Turn flywheel on
    fric1_on(shoot.fric1_pwm);
    fric2_on(shoot.fric2_pwm);
}
