#include "can1.h"

#include "led.h"

extern QueueHandle_t CAN1_Queue;					//CAN1消息队列句柄

/**
  * @brief  Initialize gimbal and chassis
  * @param  void
  * @retval void
  * @attention 201~204对应底盘,205,206对应云台
  */
void CAN1_Init(void)
{	
	GPIO_InitTypeDef gpio_str;
	CAN_InitTypeDef can_str;
	CAN_FilterInitTypeDef can_fil_str;
	NVIC_InitTypeDef  NVIC_InitStructure;//接收中断

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN1, ENABLE);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN1, DISABLE);
	
	/**********************************************************/
	gpio_str.GPIO_Mode=GPIO_Mode_AF;
	gpio_str.GPIO_OType=GPIO_OType_PP;
	gpio_str.GPIO_Pin=GPIO_Pin_0| GPIO_Pin_1;
	gpio_str.GPIO_PuPd=GPIO_PuPd_UP;
	gpio_str.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOD,&gpio_str);
	
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_CAN1);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_CAN1);
	/**********************************************************/
	
	can_str.CAN_ABOM=ENABLE;
	can_str.CAN_AWUM=DISABLE;
	can_str.CAN_BS1=CAN_BS1_9tq;//Tbs1范围CAN_BS1_1tq ~CAN_BS1_16tq
	can_str.CAN_BS2=CAN_BS2_4tq;//Tbs2范围CAN_BS2_1tq ~	CAN_BS2_8tq
	can_str.CAN_Mode=CAN_Mode_Normal;
	can_str.CAN_NART=ENABLE;//允许报文自动传送 
	can_str.CAN_Prescaler=3;//分频系数3
	can_str.CAN_RFLM=DISABLE;//报文不锁定,新的覆盖旧的 
	can_str.CAN_SJW=CAN_SJW_1tq;//重新同步跳跃宽度
	can_str.CAN_TTCM=DISABLE;	//非时间触发通信模式 
	can_str.CAN_TXFP=DISABLE;
	CAN_Init(CAN1,&can_str);
	
	can_fil_str.CAN_FilterActivation=ENABLE;//激活过滤器0
	can_fil_str.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;//过滤器0关联到FIFO0
	can_fil_str.CAN_FilterIdHigh=0x0000;//ID最大化
	can_fil_str.CAN_FilterIdLow=0x0000;
	can_fil_str.CAN_FilterMaskIdHigh    =   0x0000;
	can_fil_str.CAN_FilterMaskIdLow     =   0x0000;
	can_fil_str.CAN_FilterMode          =   CAN_FilterMode_IdMask;
	can_fil_str.CAN_FilterNumber        =   0;//过滤器0
	can_fil_str.CAN_FilterScale         =   CAN_FilterScale_32bit;//32位 
	CAN_FilterInit(&can_fil_str);
	/******************************************************************/

	CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);//FIFO0消息挂号中断允许,每接收一次进一次中断	    
  
    NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;//
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // 主优先级为1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/**
  * @brief  CAN1接收中断
  * @param  void
  * @retval void
  */
void CAN1_RX0_IRQHandler(void)
{
	CanRxMsg RxMessage;
	int16_t speed_measure,rota_measure,current_measure;
	Blue_Off;
  CAN_Receive(CAN1, 0, &RxMessage);
}


/**
  * @brief  Revolver M2008 P36
  * @param  
  * @retval void
  */
void CAN1_Revolver_Send(float input)
{
	CanTxMsg can_msg;

	can_msg.StdId=0x1FF; 
  can_msg.IDE=CAN_ID_STD;		  
  can_msg.RTR=CAN_RTR_DATA;		
  can_msg.DLC=0x08;						
	
	can_msg.Data[0]=0;
	can_msg.Data[1]=0;
	can_msg.Data[2]=(u8)((int16_t)input>>8);
	can_msg.Data[3]=(u8)((int16_t)input);
	can_msg.Data[4]=0;
	can_msg.Data[5]=0;
	can_msg.Data[6]=0;
	can_msg.Data[7]=0;  
	
	CAN_Transmit(CAN1, &can_msg);
	Green_Off;
}




/**
  * @brief  云台
  * @param  
  * @retval void
  */
void CAN1_Cloud_Send(float *PID_6623)
{
	CanTxMsg can_msg;
	
	can_msg.StdId=0x1FF;	 
    can_msg.IDE=CAN_ID_STD;		  
    can_msg.RTR=CAN_RTR_DATA;		 
    can_msg.DLC=8;							 
	
	can_msg.Data[0]=(u8)((int16_t)PID_6623[0]>>8);//yaw
	can_msg.Data[1]=(u8)((int16_t)PID_6623[0]);
	can_msg.Data[2]=(u8)((int16_t)PID_6623[1]>>8);//pitch
	can_msg.Data[3]=(u8)((int16_t)PID_6623[1]);
	can_msg.Data[4]=0;//roll
	can_msg.Data[5]=0;
	can_msg.Data[6]=0;
	can_msg.Data[7]=0;        
	
	CAN_Transmit(CAN1, &can_msg);
}

