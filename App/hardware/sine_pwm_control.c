/* 
 *
 *2020 01 01 & hxdyxd
 *
 */

#include <math.h>
#include "tim.h"
#include "sine_pwm_control.h"

static uint32_t PWM_Sin_Table[SAMP_PER_PERIOD/2];
static uint16_t PWM_Sin_Wave_Table[TABLE_SIZE];
static uint16_t SPWM_MAX = 0;

uint16_t sine_wave_get_dutycycle_maxval(void)
{
    return SPWM_MAX;
}


/**
  * @brief  init SPWM table
  * @param  void
  * @retval void
  */
void sine_wave_table_init(void)
{
    for(int i=0; i<SAMP_PER_PERIOD/2; i++) {
        PWM_Sin_Table[i] = sin(PIx2 * i * SAMP_RATE) * 4096;
    }
}


/**
  * @brief  设定SPWM表,偶数1,奇数2
  * @param  uint16_t max
  * @retval void
  */
void sine_wave_set_table(uint16_t max)
{
    uint16_t i;
    
    if(max > PWM_MAX_V || max == SPWM_MAX) {
        return;
    }
    
    SPWM_MAX = max;
    
    for(i=0; i < SAMP_PER_PERIOD/2; i++) {
        //CH1
        PWM_Sin_Wave_Table[i*2] = max * PWM_Sin_Table[i] / 4096;
        PWM_Sin_Wave_Table[TABLE_SIZE/2 + i*2] = 0;
        //CH2
        PWM_Sin_Wave_Table[i*2+1] = 0;
        PWM_Sin_Wave_Table[TABLE_SIZE/2+1 + i*2] = max * PWM_Sin_Table[i] / 4096;
    }
}

/* Complement */
#define TIM_CHANNEL_1N                      (0x0002U)
#define TIM_CHANNEL_2N                      (0x0006U)
HAL_StatusTypeDef HAL_TIM_DMABurst_WriteStart_WithBufferSize(
    TIM_HandleTypeDef *htim, uint32_t BurstBaseAddress, uint32_t BurstRequestSrc,
    uint32_t *BurstBuffer, uint32_t  BurstLength, uint16_t BufferSize);


/**
  * @brief  SPWM signal status set
  * @param  uint8_t enable
  * @retval void
  */
void sine_wave_set_status(uint8_t enable)
{
    if(enable) {
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1N);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2N);
        HAL_TIM_DMABurst_WriteStart_WithBufferSize(&htim1, TIM_DMABASE_CCR1, TIM_DMA_UPDATE,
            (uint32_t *)PWM_Sin_Wave_Table, TIM_DMABURSTLENGTH_2TRANSFERS, TABLE_SIZE);
    } else {
        HAL_TIM_DMABurst_WriteStop(&htim1, TIM_DMA_UPDATE);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1N);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2N);
    }
}

void TIM_DMAPeriodElapsedCallback(DMA_HandleTypeDef *hdma)
{
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

    htim->State = HAL_TIM_STATE_READY;
}




/* Complement */
#define TIM_DMAPeriodElapsedCplt        TIM_DMAPeriodElapsedCallback
#define TIM_DMAPeriodElapsedHalfCplt    TIM_DMACplt
#define TIM_DMATriggerCplt              TIM_DMACplt
#define TIM_DMADelayPulseHalfCplt       TIM_DMACplt
#define TIM_DMATriggerHalfCplt          TIM_DMACplt


void TIM_DMACplt(DMA_HandleTypeDef *hdma)
{
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

    htim->State = HAL_TIM_STATE_READY;
    
}



/**
  * @brief  Configure the DMA Burst to transfer Data from the memory to the TIM peripheral
  * @param  TIM_HandleTypeDef *htim
            uint32_t BurstBaseAddress, TIM Base address from where the DMA.
            uint32_t BurstRequestSrc, TIM DMA Request sources.
            uint32_t *BurstBuffer, The Buffer address.
            uint32_t BurstLength, DMA Burst length.
            uint16_t BufferSize, The Buffer size.
    @note   This function can be used any DMA data transfer length.
  * @retval void
  */
HAL_StatusTypeDef HAL_TIM_DMABurst_WriteStart_WithBufferSize(TIM_HandleTypeDef *htim, uint32_t BurstBaseAddress, uint32_t BurstRequestSrc,
                                              uint32_t *BurstBuffer, uint32_t  BurstLength, uint16_t BufferSize)
{
  /* Check the parameters */
  assert_param(IS_TIM_DMABURST_INSTANCE(htim->Instance));
  assert_param(IS_TIM_DMA_BASE(BurstBaseAddress));
  assert_param(IS_TIM_DMA_SOURCE(BurstRequestSrc));
  assert_param(IS_TIM_DMA_LENGTH(BurstLength));

  if (htim->State == HAL_TIM_STATE_BUSY)
  {
    return HAL_BUSY;
  }
  else if (htim->State == HAL_TIM_STATE_READY)
  {
    if ((BurstBuffer == NULL) && (BurstLength > 0U))
    {
      return HAL_ERROR;
    }
    else
    {
      htim->State = HAL_TIM_STATE_BUSY;
    }
  }
  else
  {
    /* nothing to do */
  }
  switch (BurstRequestSrc)
  {
    case TIM_DMA_UPDATE:
    {
      /* Set the DMA Period elapsed callbacks */
      htim->hdma[TIM_DMA_ID_UPDATE]->XferCpltCallback = TIM_DMAPeriodElapsedCplt;
      htim->hdma[TIM_DMA_ID_UPDATE]->XferHalfCpltCallback = TIM_DMAPeriodElapsedHalfCplt;

      /* Set the DMA error callback */
      htim->hdma[TIM_DMA_ID_UPDATE]->XferErrorCallback = TIM_DMAError ;

      /* Enable the DMA channel */
      if (HAL_DMA_Start_IT(htim->hdma[TIM_DMA_ID_UPDATE], (uint32_t)BurstBuffer, (uint32_t)&htim->Instance->DMAR, BufferSize) != HAL_OK)
      {
        return HAL_ERROR;
      }
      break;
    }
    case TIM_DMA_CC1:
    {
      /* Set the DMA compare callbacks */
      htim->hdma[TIM_DMA_ID_CC1]->XferCpltCallback = TIM_DMADelayPulseCplt;
      htim->hdma[TIM_DMA_ID_CC1]->XferHalfCpltCallback = TIM_DMADelayPulseHalfCplt;

      /* Set the DMA error callback */
      htim->hdma[TIM_DMA_ID_CC1]->XferErrorCallback = TIM_DMAError ;

      /* Enable the DMA channel */
      if (HAL_DMA_Start_IT(htim->hdma[TIM_DMA_ID_CC1], (uint32_t)BurstBuffer,
                           (uint32_t)&htim->Instance->DMAR, BufferSize) != HAL_OK)
      {
        return HAL_ERROR;
      }
      break;
    }
    case TIM_DMA_CC2:
    {
      /* Set the DMA compare callbacks */
      htim->hdma[TIM_DMA_ID_CC2]->XferCpltCallback = TIM_DMADelayPulseCplt;
      htim->hdma[TIM_DMA_ID_CC2]->XferHalfCpltCallback = TIM_DMADelayPulseHalfCplt;

      /* Set the DMA error callback */
      htim->hdma[TIM_DMA_ID_CC2]->XferErrorCallback = TIM_DMAError ;

      /* Enable the DMA channel */
      if (HAL_DMA_Start_IT(htim->hdma[TIM_DMA_ID_CC2], (uint32_t)BurstBuffer,
                           (uint32_t)&htim->Instance->DMAR, BufferSize) != HAL_OK)
      {
        return HAL_ERROR;
      }
      break;
    }
    case TIM_DMA_CC3:
    {
      /* Set the DMA compare callbacks */
      htim->hdma[TIM_DMA_ID_CC3]->XferCpltCallback = TIM_DMADelayPulseCplt;
      htim->hdma[TIM_DMA_ID_CC3]->XferHalfCpltCallback = TIM_DMADelayPulseHalfCplt;

      /* Set the DMA error callback */
      htim->hdma[TIM_DMA_ID_CC3]->XferErrorCallback = TIM_DMAError ;

      /* Enable the DMA channel */
      if (HAL_DMA_Start_IT(htim->hdma[TIM_DMA_ID_CC3], (uint32_t)BurstBuffer,
                           (uint32_t)&htim->Instance->DMAR, BufferSize) != HAL_OK)
      {
        return HAL_ERROR;
      }
      break;
    }
    case TIM_DMA_CC4:
    {
      /* Set the DMA compare callbacks */
      htim->hdma[TIM_DMA_ID_CC4]->XferCpltCallback = TIM_DMADelayPulseCplt;
      htim->hdma[TIM_DMA_ID_CC4]->XferHalfCpltCallback = TIM_DMADelayPulseHalfCplt;

      /* Set the DMA error callback */
      htim->hdma[TIM_DMA_ID_CC4]->XferErrorCallback = TIM_DMAError ;

      /* Enable the DMA channel */
      if (HAL_DMA_Start_IT(htim->hdma[TIM_DMA_ID_CC4], (uint32_t)BurstBuffer,
                           (uint32_t)&htim->Instance->DMAR, BufferSize) != HAL_OK)
      {
        return HAL_ERROR;
      }
      break;
    }
    case TIM_DMA_COM:
    {
      /* Set the DMA commutation callbacks */
      htim->hdma[TIM_DMA_ID_COMMUTATION]->XferCpltCallback =  TIMEx_DMACommutationCplt;
      htim->hdma[TIM_DMA_ID_COMMUTATION]->XferHalfCpltCallback =  TIMEx_DMACommutationHalfCplt;

      /* Set the DMA error callback */
      htim->hdma[TIM_DMA_ID_COMMUTATION]->XferErrorCallback = TIM_DMAError ;

      /* Enable the DMA channel */
      if (HAL_DMA_Start_IT(htim->hdma[TIM_DMA_ID_COMMUTATION], (uint32_t)BurstBuffer,
                           (uint32_t)&htim->Instance->DMAR, BufferSize) != HAL_OK)
      {
        return HAL_ERROR;
      }
      break;
    }
    case TIM_DMA_TRIGGER:
    {
      /* Set the DMA trigger callbacks */
      htim->hdma[TIM_DMA_ID_TRIGGER]->XferCpltCallback = TIM_DMATriggerCplt;
      htim->hdma[TIM_DMA_ID_TRIGGER]->XferHalfCpltCallback = TIM_DMATriggerHalfCplt;

      /* Set the DMA error callback */
      htim->hdma[TIM_DMA_ID_TRIGGER]->XferErrorCallback = TIM_DMAError ;

      /* Enable the DMA channel */
      if (HAL_DMA_Start_IT(htim->hdma[TIM_DMA_ID_TRIGGER], (uint32_t)BurstBuffer,
                           (uint32_t)&htim->Instance->DMAR, BufferSize) != HAL_OK)
      {
        return HAL_ERROR;
      }
      break;
    }
    default:
      break;
  }
  /* configure the DMA Burst Mode */
  htim->Instance->DCR = (BurstBaseAddress | BurstLength);

  /* Enable the TIM DMA Request */
  __HAL_TIM_ENABLE_DMA(htim, BurstRequestSrc);

  htim->State = HAL_TIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/******************* (C) COPYRIGHT 2020 hxdyxd *****END OF FILE****/
