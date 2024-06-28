/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <arm_math.h>
#include <arm_const_structs.h>
#include <lcd_spi_169.h>

#include <my_rda5820.h>
#include <ad9910.h>
#include <stdint.h>

#include <stdio.h>
#include <test.h>
#include <apfft.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define __ATTR_RAM_D1	__attribute__ ((section(".RAM_D1"))) __attribute__ ((aligned (4)))

#define  ADC_LENGTH 1024
#define ADC2_LEN  150
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

__ATTR_RAM_D1  uint16_t adc_value[ADC_LENGTH]={0}; 
// uint16_t fliter_value[ADC_LENGTH] = {0};
float32_t input_test[ADC_LENGTH * 2  ]={0};
float32_t output_test[ADC_LENGTH]={0};


uint16_t rx_data[10] = {0};
uint8_t tx_data[2] ={0};
uint32_t adc2_data[ADC2_LEN] ={0};
float32_t var_data[ADC2_LEN] ={0};

uint16_t vol_value = 0; //采样的直流电压�??
float32_t real_vol =0; //转换为真实电压�??
float32_t after_vol =0;
float32_t diff_vol =0; //前后两次的电压差

uint32_t fqre_a = 0;
uint32_t fqre_b = 0;
uint32_t sendTofpga_a[1]={0};//发�?�给FPGA
uint8_t  micro_a =0 ,micro_b =0;//对两个波的频率进行微调的�?

uint16_t middleValueFilter(int N);



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_SPI4_Init();
  MX_TIM6_Init();
  MX_ADC2_Init();
  MX_SPI3_Init();
  /* USER CODE BEGIN 2 */
  SPI_LCD_Init();
  AD9910_init();
  
  LCD_SetDirection(0);
  uint16_t i;
  HAL_ADCEx_Calibration_Start(&hadc1,ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
  HAL_ADC_Start_DMA(&hadc1,(uint32_t *)adc_value,ADC_LENGTH);
  HAL_TIM_Base_Start(&htim6);
  
  HAL_Delay(500);
  HAL_TIM_Base_Stop(&htim6);
  HAL_ADC_Stop_DMA(&hadc1);


  
  for(i=0; i<ADC_LENGTH; i++){
          // input_test[i*2] = 1+ cos(2 *PI * 10000 * i / 1024) + 2*cos(2 *PI *50000 *i/1024);
          input_test[i * 2] = (float32_t)adc_value[i]*3.3f/65535;
          input_test[i * 2 +1] = 0;
          
  }
 
  arm_cfft_f32(&arm_cfft_sR_f32_len1024, input_test , 0, 1);
  arm_cmplx_mag_f32(input_test, output_test, ADC_LENGTH);

    // output_test[0] /= 1024;
    // for(i=1;i<1024;i++){
    //   output_test[i] /=512;
    // }
  // 对fft后的数组进行�????????????些处理和分析
  FftMagStruct data={0};
  // data.mag[0] = 0;
  for(i=1;i<(ADC_LENGTH /2);i++){
    data.mag[i] = output_test[i];
  }

  // for (int i =0; i<sizeof(tx_data); i++) {
  //     HAL_SPI_Transmit(&hspi1, (uint8_t*)&test_data[i], 1, 10);
  // }
  
  
  float32_t var_buff[1];//方差
  uint16_t flag =0;
  uint8_t add_flag =0;//方差接近0的次数
  uint8_t stop_flag =0;
  //对数据进行处�?
  Test(&data);
  fqre_a = (data.index_a - data.index_a%5)*1000;
  fqre_b = (data.index_b - data.index_b%5)*1000;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    
    // sendTofpga_a[0] = (fqre_a<<8) | (micro_a<<1) | data.type_a;

    //spi1向FPGA发�?�两个波形的信息
    // for (int i =0; i<sizeof(tx_data); i++) {
    //   HAL_SPI_Transmit(&hspi1, (uint8_t*)&tx_data[i], 1, 10);
    // }
    
    // LCD_DisplayNumber(10, 140, real_vol*100, 5);
    // LCD_DisplayNumber(10, 160, diff_vol*100, 5);

    SineWave(500,0,(data.index_b - data.index_b%5)*1000);
    
    
    /****对adc2采样ADC2_LEN个点****/
    vol_value = middleValueFilter(7);
    real_vol = (float32_t)vol_value*3.3f/65536;
    if (flag<ADC2_LEN) {
      adc2_data[flag] = real_vol*100;
      var_data[flag]  = real_vol*100;
      flag++;
    }
    /****对adc2采样ADC2_LEN后，进行求方�?****/
    if (flag==ADC2_LEN) {
      sendDataViaUSART(adc2_data, ADC2_LEN);
      arm_var_f32((float32_t *)var_data, ADC2_LEN, &var_buff[0]);
      flag =0;
      if (stop_flag ==0) {
        //根据方差大小来调整频率
        if (var_buff[0]>=3000)                     { micro_a += 20;}
        if (var_buff[0]>=1500&& var_buff[0]<3000)  { micro_a += 15;}
        if (var_buff[0]>=600 && var_buff[0]<1500)  { micro_a += 10;}
        if (var_buff[0]>=300 && var_buff[0]<600)   { micro_a += 5;}
        if (var_buff[0]>=150 && var_buff[0]<300)   { micro_a += 3;}
        if (var_buff[0]>=10  && var_buff[0]<150)   {micro_a += 1;}
        if (var_buff[0]<=5)                        { add_flag +=1;}
        //当方差<10的次数到三次就停止调整频率
        if (add_flag ==3) {
          stop_flag = 1;
          add_flag = 0;
        }
        if (micro_a >= 100) {
          micro_a =0;
          fqre_a +=1;
        }
      }

      // sendTofpga_a[0] = ((10000000 + 50)<<1 | 0)<<7;
      sendTofpga_a[0] = ((fqre_a*100 + micro_a)<<1 | 0)<<7;

      HAL_SPI_Transmit(&hspi1, (uint8_t*)&sendTofpga_a[0], 1, 10);
    }

    LCD_DisplayNumber(10, 10, fqre_a, 6);
    // LCD_DisplayNumber(60, 10, data.wave_a, 6);
    LCD_DisplayNumber(140, 10, data.type_a, 1);
    LCD_DisplayNumber(10, 50, fqre_b, 6);
    // LCD_DisplayNumber(60, 50, data.wave_b, 6);
    LCD_DisplayNumber(140, 50, data.type_b, 1);
    // LCD_DisplayNumber(10, 80, tx_data[0], 3);
    // LCD_DisplayNumber(10, 110, tx_data[1], 3);
    LCD_DisplayNumber(100, 140, var_buff[0], 5);
    LCD_DisplayNumber(100, 160, micro_a, 5);


   
    
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 48;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV4;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 12;
  PeriphClkInitStruct.PLL2.PLL2P = 4;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//中位值滤波算法，N为奇�????????????
  uint16_t middleValueFilter(int N){
    int value_buf[N];
    int i,j,k,temp;
    for(i=0;i<N;i++){
      HAL_ADC_Start(&hadc2);
      HAL_ADC_PollForConversion(&hadc2,10);
      HAL_ADC_Stop(&hadc2);
      value_buf[i]=HAL_ADC_GetValue(&hadc2);
    }
    //冒泡法排�????????????,从小到大
    for(j=0;j<N-1;++j){
      for(k=0;k<N-j-1;++k){
        if(value_buf[k]>value_buf[k+1]){
          temp=value_buf[k];
          value_buf[k]=value_buf[k+1];
          value_buf[k+1]=temp;
        }
      }
    }
    return value_buf[(N-1)/2];
  }

//   static uint16_t i=1;
// void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* spiHandle)
// {
  
// 	if(spiHandle->Instance == SPI1)
// 	{
//     HAL_SPI_Receive_IT(&hspi1,(uint8_t *)&rx_data[i%10],1);
// 	}
//   i++;
// }

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
