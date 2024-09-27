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
#include "configuration.h"
#include "notes.h"
#include "scores.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define WAVETABLE_SZ  16384U
#define SAMPLE_FREQ   ( 150000000.0 / 15.0 / 120.0 )

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// The instantaneous wave state.
            uint16_t        wv                    = 2048;

// The instantaneous phase of each wave.  This is an example of phase accumulators.
volatile    uint16_t        ph1                   = 0,
                            ph2                   = 0,
                            ph3                   = 0;

// Decay rate in steps for each wave's volume.  Higher values mean faster decay.
volatile    uint16_t        note_dec_step         = DECAY_COUNTS;

// Each channel has an associated volume, this is where the current volume is stored.
volatile    int16_t         amp1                  = 0,
                            amp2                  = 0,
                            amp3                  = 0;

// Engine is either playing or has completed.  This is the flag.
            uint8_t         playing               = 0;

// DIP switch option value variable.
            uint8_t         option;

// Master volume divider.  Required to avoid clipping. Amplifier overdrive IS allowed,
// ..but you may not like how it sounds.
            float           mvol_divider;

// phase accumulator steps.  Higher means higher frequencies.  It should be noted that
// there are limited samples available and too big  step won't sound nice.
//
// Very high frequecies are possible!
            uint16_t        ph1_step              = PH1_STEP_DEFAULT,
                            ph2_step              = PH2_STEP_DEFAULT,
                            ph3_step              = PH3_STEP_DEFAULT;

// per-channel drop rates for volume (presets applied)
//
            uint8_t         droprate_ch1          = DROP_RATE,
                            droprate_ch2          = DROP_RATE,
                            droprate_ch3          = DROP_RATE;

// one wants to know when an event should happen, and I don't want a full OS just for this
// therefore there is are simple counters and flag.
volatile    uint32_t        systick_counter       = 0,
                            event_delay           = 0;
volatile    uint8_t         systick_timeout       = 0;

// The wave array is stored as a const int16_t in wave.h, included here.
#include "wave.h"

// Debug variable(s)
#ifdef PEAK_CAPTURE
            uint16_t        peak = 2048;
#endif


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

__weak  void      eventTimoutCallback           ( void );
        void      HAL_IncTick                   ( void );
        void      HAL_TIM_PeriodElapsedCallback ( TIM_HandleTypeDef *htim );
        void      StartTimer                    ( void );
        uint8_t   readOption                    ( void );
        void      shiftToDACCenter              ( void );
        void      playNote                      ( uint8_t ch,
                                                  float   freq_hz,
                                                  uint8_t dec_rate,
                                                  uint16_t ch_vol
                                                );
        uint16_t  bpm_to_ms                     ( uint16_t bpm );
        void      playScore                     ( float * sc_to_play );

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  *
  * Note: there are variables you might like to experiment with like mvol_divider
  * given the great variety of speakers and this boards ability to overdrive the
  * amplifier (at least for now).
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

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DAC1_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */

  HAL_DAC_SetValue( &hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048 );
  HAL_DAC_MspInit( &hdac1 );
  HAL_DAC_Start( &hdac1, DAC_CHANNEL_1 );

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

    StartTimer();

    switch( option )
    {
    case 0:                 // Single note
      mvol_divider = 2.25;

      playNote( 1,  nE5 , 2, 1024 );
      break;

    case 1:                 // Two notes descending
      mvol_divider = 2.25;

      playNote( 1, nE5, 2, 1024 );
      HAL_Delay( 680 );
      playNote( 2, nC5, 2, 1024 );
      break;

    case 2:                 // Tri-tone descending
      mvol_divider = 2.25;

      playNote( 1, nE5, 2, 1024 );
      HAL_Delay( 680 );
      playNote( 2, nC5, 2, 1024 );
      HAL_Delay( 680 );
      playNote( 3, nG4, 2, 1024 );
      break;

    case 3:                 // Tri-tone ascending
      mvol_divider = 2.25;

      playNote( 1, nG4, 2, 1024 );
      HAL_Delay( 680 );
      playNote( 2, nC5, 2, 1024 );
      HAL_Delay( 680 );
      playNote( 3, nE5, 2, 1024 );
      break;

    case 4:                 // Musical score 1
      mvol_divider = 2.5;

      playScore( (float *) scale4 );
      break;
    }
    // Wait for end.
    //
    while( amp1 + amp2 + amp3 );
    playing = 0;

    // Reset DAC
    //
    shiftToDACCenter();

#ifndef  TEST_CYCLING_SET
    // Wait for button to be pressed and released
    //
    // The system is also reset, to prevent minor soft errors from accumulating.  This is after all left on
    // for possibly years at a time.  Note also that we don't use that solution in test cycling or debug mode.
    //
    while( !HAL_GPIO_ReadPin( TRIGGER_GPIO_Port, TRIGGER_Pin ) );
#ifndef DEBUG_MODE
    HAL_NVIC_SystemReset();
#endif
#endif

    // Reset the phase accumulators, only necessary in DEBUG_MODE
    //
    ph1 = ph2 = ph3 = 0;
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
  HAL_PWREx_ControlVoltageScaling( PWR_REGULATOR_VOLTAGE_SCALE1 );

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType        = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState              = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue   = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState          = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource         = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM              = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN              = 75;
  RCC_OscInitStruct.PLL.PLLP              = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ              = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR              = RCC_PLLR_DIV2;

  if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType       = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                    | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource    = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider   = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider  = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider  = RCC_HCLK_DIV1;

  if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_4 ) != HAL_OK )
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/** Event delay callback function.
  *
  */
__weak void eventTimoutCallback( void )
{

}


/** Updates the uwTick variable and updates the systick_counter if not already 0.
  *
  * Overrides the '__weak' version in the HAL.
  *
  * @param none
  * @retval none
  */
void HAL_IncTick(void)
{
  uwTick += uwTickFreq;

  if( systick_counter ) systick_counter--;
  if( event_delay ) event_delay--;
  
  if( !systick_counter )
  {
    // Handle volume decay
    //
    if( amp1 ) amp1 -= droprate_ch1;
    if( amp2 ) amp2 -= droprate_ch2;
    if( amp3 ) amp3 -= droprate_ch3;
    if( amp1 <= CUTOFF_POINT )  amp1 = 0;
    if( amp2 <= CUTOFF_POINT )  amp2 = 0;
    if( amp3 <= CUTOFF_POINT )  amp3 = 0;
    systick_timeout = 1;
    systick_counter = note_dec_step; 
  }  

  if( !event_delay )
  {
      eventTimoutCallback();
  }
}


/** Callback servicing timer interrupts. Overrides the HAL '__weak' version.
  *
  * Only handles interrupts from timer 3 at present, but addind support
  * for other timers is trivial.
  *
  * @param *htim
  * @retval none
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);
  HAL_DAC_SetValue( &hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, wv );
  if( htim !=&htim3 ) return;

   if( playing )
    {
      ph1 += ph1_step;        // Increment phase accumulator for note 1
      ph1 %= WAVETABLE_SZ;

      ph2 += ph2_step;        // Increment phase accumulator for note 2
      ph2 %= WAVETABLE_SZ;

      ph3 += ph3_step;        // Increment phase accumulator for note 3
      ph3 %= WAVETABLE_SZ;

// Below is a complex mess of casting to attempt to improve the audio.
// ...this may be misguided!
      wv = (int16_t)          // Create composite waveform, and yes, the DAC does have a signed
          (                   // mode, but this code should port to other platforms too.
            ( (int32_t) wave[ ph1 ] * amp1 / 1024 ) 
              +
            ( (int32_t) wave[ ph2 ] * amp2 / 1024 )
              +
            ( (int32_t) wave[ ph3 ] * amp3  / 1024 )
          ) / 4 / mvol_divider + 2048;

#ifdef PEAK_CAPTURE
      if( wv > peak ) peak = wv;
#endif
    }
}


/** Starts both the timer, and interrupts from it.
  *
  * @param: none
  * @retval: none
  */
void StartTimer( void )
{
  HAL_TIM_Base_Start_IT( &htim3 );
  HAL_TIM_Base_Start( &htim3 );
}


/** Reads the option switches
  *
  * @param none
  * @retval option value 0..15.
  */
uint8_t readOption( void )
{
  uint8_t opt_read = 0;

  opt_read  = HAL_GPIO_ReadPin( OPT1_GPIO_Port, OPT1_Pin );
  opt_read |= HAL_GPIO_ReadPin( OPT2_GPIO_Port, OPT2_Pin ) << 1;
  opt_read |= HAL_GPIO_ReadPin( OPT3_GPIO_Port, OPT3_Pin ) << 2;
  opt_read |= HAL_GPIO_ReadPin( OPT4_GPIO_Port, OPT4_Pin ) << 3;

  return opt_read;
}


/** Softly move to centere point gracefully.  This avoids pops.
  *
  * @param none
  * @retval none
  *
  */
  void shiftToDACCenter( void )
  {
    char dir = 0;

    if( wv < 2048 ) dir = 1;
    if( wv > 2048 ) dir = -1;
    if( wv == 2048 ) return;

    for( ; wv != 2048; )
    {
      if( dir == 1 ) wv++;
      else wv--;

      HAL_DAC_SetValue( &hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, wv );
    }
  }

/** Play a note
  *
  * @param ch:        the channel to play.
  * @param freq_hz:   The frequency of the note.
  * @param dec_rate:  Decay rate of the note.
  * @param ch_volume: Volume of note at initial strike.
  * @retval none.
  */
void    playNote( uint8_t ch,
                  float freq_hz,
                  uint8_t dec_rate,
                  uint16_t ch_vol
                )
{
  uint16_t ph_step;

  playing = 1;
  ph_step = freq_hz / ( SAMPLE_FREQ / WAVETABLE_SZ );

#ifdef PRINT_SCORE
  debug_printf( "%u, %f, %u\n\r", ch, freq_hz, ch_vol );
#endif
  switch( ch )
  {
    case 1:   // Channel 1
      ph1_step = ph_step;
      ph1 = 0;
      droprate_ch1 = dec_rate;
      amp1 = ch_vol;
    break;

    case 2:   // Channel 2
      ph2_step = ph_step;
      ph2 = 0;
      droprate_ch2 = dec_rate;
      amp2 = ch_vol;
    break;

    case 3:   // Channel 3
      ph3_step = ph_step;
      ph3=0;
      droprate_ch3 = dec_rate;
      amp3 = ch_vol;
    break;
  }
}

/** BPM to ms conversion function
  *
  * @param  bpm
  * @retval ms
  */
uint16_t bpm_to_ms( uint16_t bpm )
{
  #define ONE_MINUTE 60000U     // 1 minute in ms

  return ONE_MINUTE / bpm;
}

void playScore( float *sc_to_play )
{
  uint16_t  score_pointer = 1;
  int beat = bpm_to_ms( sc_to_play[ 1 ] );  // notes per bar = 4
  uint8_t droprate = sc_to_play[2];

  mvol_divider = sc_to_play[ 0 ];

  
  while( sc_to_play[ score_pointer * 3 + 1 ] != END_PLAY )
  {
    
    playNote( (uint8_t) sc_to_play[ score_pointer * 3 ],
              sc_to_play[score_pointer * 3 + 1],
              droprate, 1024 );
    HAL_Delay( beat * sc_to_play[ score_pointer * 3 + 2 ] );

    score_pointer++;
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler( void )
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while( 1 )
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
