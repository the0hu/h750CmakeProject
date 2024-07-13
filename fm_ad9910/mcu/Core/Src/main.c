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
#include "ad9910.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#include "test.h"
#include "lcd_spi_169.h"
#include <math.h>
#include <stdint.h>
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

// usart1-----烧录
// usart2-----用来与上位机调试
#define  NUM_OF_DATA 2
uint64_t rx_data[NUM_OF_DATA] = {}; //串口接收数组
uint32_t tx_data[1] = {0};
uint32_t count = 0;
uint8_t buffer_index = 0; //串口接收下标
uint8_t rxBuffer[16];  // 假设串口每次接收16字节数据
amfmStruct data ={0};
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
  AD9910_init();
  SPI_LCD_Init();
  HAL_UART_Receive_IT(&huart2, (uint8_t *)rxBuffer, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (count==1) {
      get_parameter(rxBuffer, rx_data, &data);

      count=0;
    }
    
    // LCD_DisplayNumber(10, 10, rx_data[0], 15);
    // LCD_DisplayNumber(10, 40, data.type, 5);
    // LCD_DisplayNumber(10, 60, data.phase_dev, 10);
    // LCD_DisplayNumber(10, 90, data.Rb, 3);
    switch (data.type) {
      case 1:
            LCD_DisplayString(10, 10, "AM");
            LCD_DisplayString(10, 40, "Freq:");LCD_DisplayNumber(80, 40, data.am.freq, 5);
            LCD_DisplayString(10, 70, "ma:");
            LCD_DisplayNumber(80,70,(data.am.ma * 100)/100,1);LCD_DisplayString(100, 70,".");LCD_DisplayNumber(115,70,(int)(data.am.ma * 100)%100,3);
        break;
      case 2:
            LCD_DisplayString(10, 10, "FM");
            LCD_DisplayString(10, 40, "Freq:");LCD_DisplayNumber(80, 40, data.fm.freq, 5);
            LCD_DisplayString(10, 70, "mf:");
            LCD_DisplayNumber(80, 70, (data.fm.mf*100)/100, 1);LCD_DisplayString(100, 70,".");LCD_DisplayNumber(115,70,(int)(data.fm.mf * 100)%100,3);
            LCD_DisplayString(10, 100, "diff:");LCD_DisplayNumber(80, 100, data.fm.diff_fmax, 5);
        break;
      case 5:
            LCD_DisplayString(10, 10, "ASK");
            LCD_DisplayString(10, 40, "Rc:");LCD_DisplayNumber(80, 40, data.ask.Rc, 5);
        break;
      case 6:
            LCD_DisplayString(10, 10, "FSK");
            LCD_DisplayString(10, 40, "Rc:");LCD_DisplayNumber(80, 40, data.fsk.Rc, 5);
            LCD_DisplayString(10, 70, "h:");
            LCD_DisplayNumber(80, 70, (data.fsk.h*100)/100, 2);LCD_DisplayString(110, 70,".");LCD_DisplayNumber(130,70,(int)(data.fsk.h * 100)%100,2);
        break;
      case 7:
            LCD_DisplayString(10, 10, "PSK");
            LCD_DisplayString(10, 40, "Rc:");LCD_DisplayNumber(60, 40, data.psk.Rc, 5);
        break;
    }


   
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    
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
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
    rxBuffer[buffer_index++] = huart->Instance->RDR;
    if (buffer_index == 16)
        { 
         buffer_index = 0;
         HAL_UART_Receive_IT(&huart2, rxBuffer, 1);   
         count =1;  
        }
    else {
      HAL_UART_Receive_IT(&huart2, rxBuffer+buffer_index, 1); 
    }
       
    }
    
}


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
