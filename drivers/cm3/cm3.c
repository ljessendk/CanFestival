// Includes for the Canfestival driver
#include "stm32f10x_rcc.h"
#include "stm32f10x_can.h"
#include "stm32f10x_gpio.h"
#include "misc.h"
#include "stm32f10x_tim.h"
#include "canfestival.h"
#include "timer.h"
#include "cm3.h"
#include "can.h"

/* Gestion d'un buffer tournant de messages CAN en Tx pour que le transfert
 * en mode block fonctionne avec SDO_BLOCK_SIZE > 3 
 */
#define MESS_BUFFER_SIZE (SDO_BLOCK_SIZE + 3)
typedef struct  
{
    Message Buf[MESS_BUFFER_SIZE];
    Message *readp;
    Message *writep;
} MessBuf_t;
static MessBuf_t messBuf;
void MessBuf_Init(MessBuf_t *pFifo);
int MessBuf_Write(MessBuf_t *pFifo, Message *pgd);
int MessBuf_Read(MessBuf_t *pFifo, Message *pgd);

static uint8_t tx_fifo_in_use = 0;

TIMEVAL last_counter_val = 0;
TIMEVAL elapsed_time = 0;

static CO_Data *co_data = NULL;

void clearTimer(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM3 disable counter */
	TIM_Cmd(TIM3, DISABLE);

	/* TIM Interrupts disable */
	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, DISABLE);

}

void start_callback(CO_Data* d, UNS32 id)
{
}

// Initializes the timer, turn on the interrupt and put the interrupt time to zero
void initTimer(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	/* Enable the TIM3 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Compute the prescaler value */
	/* SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f10x.c file */
	uint16_t PrescalerValue = (uint16_t) (SystemCoreClock  / 100000) - 1;

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	TIM_ClearITPendingBit(TIM3, TIM_SR_UIF);

	/* TIM3 disable counter */
	TIM_Cmd(TIM3, DISABLE);

	/* Preset counter for a safe start */
	TIM_SetCounter(TIM3, 0);

	/* TIM Interrupts enable */
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	// this is needed for correct canfestival virtual timer management start
	SetAlarm(NULL, 0, start_callback, 0, 0);
}

//Set the timer for the next alarm.
void setTimer(TIMEVAL value)
{
  	uint32_t timer = TIM_GetCounter(TIM3);        // Copy the value of the running timer
	elapsed_time += timer - last_counter_val;
	last_counter_val = 65535-value;
	TIM_SetCounter(TIM3, 65535-value);
	TIM_Cmd(TIM3, ENABLE);
}

//Return the elapsed time to tell the Stack how much time is spent since last call.
TIMEVAL getElapsedTime(void)
{
  	uint32_t timer = TIM_GetCounter(TIM3);        // Copy the value of the running timer
	if(timer < last_counter_val)
		timer += 65535;
	TIMEVAL elapsed = timer - last_counter_val + elapsed_time;
	//printf("elapsed %lu - %lu %lu %lu\r\n", elapsed, timer, last_counter_val, elapsed_time);
	return elapsed;
}

// This function handles Timer 3 interrupt request.
void TIM3_IRQHandler(void)
{
	//printf("--\r\n");
	if(TIM_GetFlagStatus(TIM3, TIM_SR_UIF) == RESET)
		return;
	last_counter_val = 0;
	elapsed_time = 0;
	TIM_ClearITPendingBit(TIM3, TIM_SR_UIF);
	TimeDispatch();
}

/* prescaler values for 87.5%  sampling point (88.9% at 1Mbps)
   if unknown bitrate default to 50k
*/
uint16_t brp_from_birate(uint32_t bitrate)
{
	if(bitrate == 10000)
		return 225;
	if(bitrate == 50000)
		return 45;
	if(bitrate == 125000)
		return 18;
	if(bitrate == 250000)
		return 9;
	if(bitrate == 500000)
		return 9;
	if(bitrate == 1000000)
		return 4;
	return 45;
}

//Initialize the CAN hardware 
unsigned char canInit(CO_Data * d, uint32_t bitrate)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;
  CAN_InitTypeDef        CAN_InitStructure;
  CAN_FilterInitTypeDef  CAN_FilterInitStructure;
    
  /* save the canfestival handle */  
  co_data = d;

  MessBuf_Init(&messBuf);

  /* CAN GPIOs configuration **************************************************/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(CAN_GPIO_CLK, ENABLE);
  
  /* Configure CAN pin: RX */
  GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(CAN_GPIO_PORT, &GPIO_InitStructure);
  
  /* Configure CAN pin: TX */
  GPIO_InitStructure.GPIO_Pin = CAN_TX_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(CAN_GPIO_PORT, &GPIO_InitStructure);

  GPIO_PinRemapConfig(GPIO_Remap1_CAN1 , ENABLE);

  /* NVIC configuration *******************************************************/
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* CAN configuration ********************************************************/  
  /* Enable CAN clock */
  RCC_APB1PeriphClockCmd(CAN_CLK, ENABLE);
  
  /* CAN register init */
  CAN_DeInit(CANx);
  CAN_StructInit(&CAN_InitStructure);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM = DISABLE;
  CAN_InitStructure.CAN_ABOM = ENABLE;
  CAN_InitStructure.CAN_AWUM = DISABLE;
  CAN_InitStructure.CAN_NART = DISABLE;
  CAN_InitStructure.CAN_RFLM = DISABLE;
  CAN_InitStructure.CAN_TXFP = ENABLE;
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
    
  /* CAN Baudrate (CAN clocked at 36 MHz)  36e6 / ( prescaler * (1+BS1+BS2))  */
  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
  if(bitrate == 1000000){
  	CAN_InitStructure.CAN_BS1 = CAN_BS1_7tq;
  	CAN_InitStructure.CAN_BS2 = CAN_BS2_1tq;
  }
  else if(bitrate == 500000){
  	CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;
  	CAN_InitStructure.CAN_BS2 = CAN_BS2_1tq;
  }
  else{
 	CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
  	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
  }
  CAN_InitStructure.CAN_Prescaler = brp_from_birate(bitrate);
  
  CAN_Init(CANx, &CAN_InitStructure);

  /* CAN filter init */
  CAN_FilterInitStructure.CAN_FilterNumber = 0;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
  
  /* Enable FIFO 0 message pending Interrupt */
  CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);

  return 1;
}

void canClose(void)
{
  NVIC_InitTypeDef  NVIC_InitStructure;
  /* NVIC configuration : remove irq */
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  /* CAN déinit */
  CAN_DeInit(CANx);
  /* disable CAN clock */
  RCC_APB1PeriphClockCmd(CAN_CLK, DISABLE);
}

// The driver send a CAN message passed from the CANopen stack
unsigned char lowlevel_canSend(Message *m)
{
	int i, res;
	CanTxMsg TxMessage = {0};
	TxMessage.StdId = m->cob_id;
	TxMessage.IDE = CAN_ID_STD;
	if(m->rtr)
  		TxMessage.RTR = CAN_RTR_REMOTE;
	else
  		TxMessage.RTR = CAN_RTR_DATA;
	TxMessage.DLC = m->len;
	for(i=0 ; i<m->len ; i++)
		TxMessage.Data[i] = m->data[i]; 
    res = CAN_Transmit(CANx, &TxMessage);
	if(res == CAN_TxStatus_NoMailBox)
		return 0; 	// error
    return 1;		// succesful
}

void USB_HP_CAN1_TX_IRQHandler(void)
{
	Message msg;
	CAN_ClearITPendingBit(CAN1, CAN_IT_TME);
	if(MessBuf_Read(&messBuf, &msg)){
  		CAN_ITConfig(CANx, CAN_IT_TME, DISABLE);	// fifo vide, pas de trames à envoyer
		tx_fifo_in_use = 0;
	}
	else
		lowlevel_canSend(&msg);
}

// The driver send a CAN message passed from the CANopen stack
unsigned char canSend(CAN_PORT notused, Message *m)
{
	unsigned char rv;
  	CAN_ITConfig(CANx, CAN_IT_TME, DISABLE);
	if (tx_fifo_in_use)
		rv = MessBuf_Write(&messBuf, m) ? 0 : 1;
	else{
		rv = lowlevel_canSend(m);
		if(!rv){	// si pas de mailbox libre on écrit dans la fifo
			rv = MessBuf_Write(&messBuf, m) ? 0 : 1;
			tx_fifo_in_use = 1;		
		}
	}
	if (tx_fifo_in_use)
  		CAN_ITConfig(CANx, CAN_IT_TME, ENABLE);
	return rv;
}

//The driver pass a received CAN message to the stack
/*
unsigned char canReceive(Message *m)
{
}
*/
unsigned char canChangeBaudRate_driver( CAN_HANDLE fd, char* baud)
{
	return 0;
}

/**
  * @brief  This function handles CAN1 RX0 interrupt request.
  * @param  None
  * @retval None
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	int i;
	CanRxMsg RxMessage = {0};
	Message rxm = {0};
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
	// Drop extended frames
	if(RxMessage.IDE == CAN_ID_EXT)
		return;
	rxm.cob_id = RxMessage.StdId;
  	if(RxMessage.RTR == CAN_RTR_REMOTE)
		rxm.rtr = 1;
	rxm.len = RxMessage.DLC;
	for(i=0 ; i<rxm.len ; i++)
  		 rxm.data[i] = RxMessage.Data[i];
	canDispatch(co_data, &rxm);
}

void disable_it(void)
{
	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
  	CAN_ITConfig(CANx, CAN_IT_FMP0, DISABLE);
}

void enable_it(void)
{
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);
}

/*--------- Jeu de fonctions pour utiliser le buffer de messages CAN ----------*/
/*=============================================================================*/

void MessBuf_Init(MessBuf_t *pFifo)
{
	pFifo->readp=pFifo->writep=pFifo->Buf;
}

/* Ecriture d'un nouveau message dans le buffer
 * retour :  1 buffer plein
 *           0 ok
 */
int MessBuf_Write(MessBuf_t *pFifo, Message *pgd)
{
	int Space;
	/* calcul espace restant dans la fifo */
	Space = (pFifo->writep < pFifo->readp) ? 
		(pFifo->readp - pFifo->writep -1) : (pFifo->readp - pFifo->writep + (MESS_BUFFER_SIZE-1));
	if(Space == 0)
		return 1;
	*(pFifo->writep)=*pgd;
	pFifo->writep++;
	if (pFifo->writep >= (pFifo->Buf + MESS_BUFFER_SIZE ))
		pFifo->writep = pFifo->Buf;
	return 0;
}

/* Lecture d'un  message dans le buffer
 * retour :  1 buffer vide
 *           0 ok
 */
int MessBuf_Read(MessBuf_t *pFifo, Message *pgd)
{
	if (pFifo->readp == pFifo->writep)
		return 1;
	*pgd=*(pFifo->readp);
	pFifo->readp++;
	if(pFifo->readp >= (pFifo->Buf + MESS_BUFFER_SIZE))
		pFifo->readp = pFifo->Buf;
	return 0;
}

