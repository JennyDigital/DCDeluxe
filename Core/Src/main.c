/* USER CODE BEGIN Header */

/*  DCDeluxe, A door chime based on the STM32.
.
    Copyright (C) 2024 Jennifer Gunn (JennyDigital).

	jennifer.a.gunn@outlook.com

    This software/hardware combination is free and open;
    you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA
*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dac.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include "configuration.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define WAVETABLE_SZ  4096U
#define RES_8B        255U
#define RES_12B       4096U
#define pi            3.1415926

#define CHOSEN_RES    RES_12B

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

            uint16_t        wv,
                            pitch_delay           = NOTE_BASE_CYCLE;

            uint16_t        ph1                   = 2048,
                            ph2                   = 2048,
                            ph3                   = 2048,
                            po                    = 3;

            uint16_t        note_dec_step         = DECAY_COUNTS;

            int16_t         amp1                  = 0,
                            amp2                  = 0,
                            amp3                  = 0;

            uint8_t         mvol                  = 0,
                            playing               = 1,
                            seq_notes;

            uint8_t         option;

volatile    uint32_t        systick_counter       = 0;

            int16_t         wave[ WAVETABLE_SZ ]  = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void    HAL_TIM_PeriodElapsedCallback ( TIM_HandleTypeDef *htim );
void    StartTimer                    ( void );
void    genSine                       ( void );
uint8_t readOption                    ( void );
void    playNotes                     ( void );

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  //HAL_FLASH_Unlock();
  //HAL_FLASH_OB_Unlock()
  //FLASH_OBProgramInitTypeDef pOBInitStruct = {0};

  //pOBInitStruct.OptionType = OPTIONBYTE_RDP;

  //pOBInitStruct.RDPLevel    = OB_RDP_LEVEL1;

  //if( HAL_GPIO_ReadPin( UNLOCK_GPIO_Port,  UNLOCK_Pin )
  //{

  //}
  //pOBInitStruct.RDPLevel    = OB_RDP_LEVEL1;

  //HAL_FLASHEx_OBProgram( &pOBInitStruct );
  //HAL_FLASH_OB_Launch();

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  HAL_Delay( 100 );
  genSine();

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DAC1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  HAL_DAC_MspInit( &hdac1 );
  HAL_DAC_Start( &hdac1, DAC_CHANNEL_1 );
  HAL_Delay( 200 );
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

      // Get option selection.
    //
    option = readOption();
    //option = 2;

    // Set up note
    //
    if( option != 3 )         // One note
    {
      amp1 = NOTE1_VOL;
    }
    else                      // Chord
    {
      amp1 = amp2 = amp3 = NOTE1_VOL;
    }

    playNotes();

#ifndef  TEST_CYCLING_SET
    // Wait for button to be pressed and released
    //
    while( !HAL_GPIO_ReadPin( TRIGGER_GPIO_Port, TRIGGER_Pin ) );
    HAL_Delay( 50 );
    while( HAL_GPIO_ReadPin( TRIGGER_GPIO_Port, TRIGGER_Pin ) );
#endif

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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 75;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_IncTick(void)
{
  uwTick += uwTickFreq;

  if( systick_counter )
  {
    systick_counter--;
  }

}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);

  if( htim !=&htim3 ) return;
   if( playing )
    {
      ph1 += PH1_STEP;        // Increment phase accumulator for note 1
      ph1 %= WAVETABLE_SZ;

      ph2 += PH2_STEP;        // Increment phase accumulator for note 2
      ph2 %= WAVETABLE_SZ;

      ph3 += PH3_STEP;        // Increment phase accumulator for note 3
      ph3 %= WAVETABLE_SZ;

       wv =                   // Create composite waveform.
            ( 
              ( wave[ ph1 ] * amp1 / 2048 ) 
                +
              ( wave[ ph2 ] * amp2 / 2048 )
                +
              ( wave[ ph3 ] * amp3 / 2048 )
            ) / 1.57 + 2048;

      HAL_DAC_SetValue( &hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, wv );
    }
}


void StartTimer( void )
{
  HAL_TIM_Base_Start_IT( &htim3 );
  HAL_TIM_Base_Start( &htim3 );
}


void genSine( void )
{
  for( int i=0; i<WAVETABLE_SZ; i++)
  {
    wave[ i ] = ( ( sin( i * 2 * pi / WAVETABLE_SZ ) + 1 ) ) * CHOSEN_RES / 4 - 1024;
  }
}


uint8_t readOption( void )
{
  uint8_t opt_read = 0;

  opt_read  = HAL_GPIO_ReadPin( OPT1_GPIO_Port, OPT1_Pin );
  opt_read |= HAL_GPIO_ReadPin( OPT2_GPIO_Port, OPT2_Pin ) << 1;
  opt_read |= HAL_GPIO_ReadPin( OPT3_GPIO_Port, OPT3_Pin ) << 2;
  opt_read |= HAL_GPIO_ReadPin( OPT4_GPIO_Port, OPT4_Pin ) << 3;

  return opt_read;
}

void playNotes( void )
{
  // RampToCenter();
  StartTimer();

  uint8_t note2_not_triggered = 1,
          note3_not_triggered = 1;
  
  amp1 = NOTE1_VOL;
  playing = 1;

  while( playing )
  {
   if( !systick_counter )
    {
      // Handle volume decay
      //
      if( amp1 ) amp1 -= DROP_RATE;
      if( amp2 ) amp2 -= DROP_RATE;
      if( amp3 ) amp3 -= DROP_RATE;
      if( amp1 <= CUTOFF_POINT )  amp1 = 0;
      if( amp2 <= CUTOFF_POINT )  amp2 = 0;
      if( amp3 <= CUTOFF_POINT )  amp3 = 0;

      // Manage triggering of extra notes.
      //
      switch( option )
      {
        case 0:       // One note only
          break;

        case 1:       // Two notes.
          if( amp1 <= SECOND_NOTE_THRESHOLD && note2_not_triggered ) 
          {
            amp2 = NOTE2_VOL;
            ph2 = 0;
            note2_not_triggered = 0;
          }
          break;

        case 2:       // Three notes.
          if( amp1 <= SECOND_NOTE_THRESHOLD && note2_not_triggered ) 
          {
            amp2 = NOTE2_VOL;
            ph2 = 0;
            note2_not_triggered = 0;
          }

          if( amp2 <= THIRD_NOTE_THRESHOLD && note3_not_triggered && !note2_not_triggered ) 
          {
            amp3 = NOTE3_VOL;
            ph3 = 0;
            note3_not_triggered = 0;
          }
          break;

        case 3:       // Chord.
          break;
      }

      // Stop when all notes done.
      //
      if( !( amp1 | amp2 | amp3 ) ) playing = 0;

      // Reset counter
      //
      systick_counter = note_dec_step;

    }     // End of if( !systick_counter )
  }       // End of while( playing )
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
