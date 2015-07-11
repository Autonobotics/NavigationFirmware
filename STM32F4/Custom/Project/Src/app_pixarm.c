/**
  ******************************************************************************
  * @file    Src/app_i2c.c 
  * @author  Autonobotic Team
  * @version V1.0
  * @date    6-July-2015
  * @brief   Application I2C Implementation
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "app_pixarm.h"
#include "app_uart_generic.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define INPUT_PIXARM_BUFFER_SIZE (sizeof(AppPixarmCblk.inputBuffer.buffer))
#define OUTPUT_PIXARM_BUFFER_SIZE (sizeof(AppPixarmCblk.outputBuffer.buffer))
#define PIXARM_TRANSACTION_RETRY_LIMIT 5
    
/* Private macro -------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
UART_HandleTypeDef PixarmHandle;
TIM_HandleTypeDef htim10;

/* Private variables ---------------------------------------------------------*/
static sAPP_PIXARM_CBLK AppPixarmCblk = {
    NULL,
    NULL,
#ifdef PIXARM_WATCHDOG_ENABLE
    UART_PERIPHERAL_IDLE,
#endif
    PIXARM_INIT,
    PIXARM_INIT,
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    UART_INITIAL
};


/* Private function prototypes -----------------------------------------------*/
#ifdef PIXARM_WATCHDOG_ENABLE
static void pixarm_watchdog_init(void);
#endif
static eAPP_STATUS pixarm_transmit(void);
static eAPP_STATUS pixarm_receive(void);
static eAPP_STATUS pixarm_handle_request(sAPP_NAVIGATION_CBLK* navigation_cblk);
static eAPP_STATUS pixarm_state_machine(sAPP_NAVIGATION_CBLK* navigation_cblk);

/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
/*                 Internal Support Functions                                 */
/******************************************************************************/
#ifdef PIXARM_WATCHDOG_ENABLE
static void pixarm_watchdog_init(void)
{
    /*
    TIM10 is connected to APB2 bus, which has on F407 device 84MHz clock
    But, timer has internal PLL, which double this frequency for timer, up to 168MHz
    
    Set timer prescaler:
    timer_tick_frequency = Timer_default_frequency / (prescaller_set + 1)
    timer_tick_frequency = 168000000 / (999 + 1) = 168000
    
    Period results in 100/168000 seconds or approx. 0.6 ms.
    */
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    
    htim10.Instance = TIM10;
    htim10.Init.Prescaler = 999;
    htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim10.Init.Period = 1000;
    htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim10);
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim10, &sClockSourceConfig);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim10, &sMasterConfig);
    
    AppPixarmCblk.tim10Handle = & htim10;
}
#endif

static eAPP_STATUS pixarm_transmit(void)
{
    HAL_StatusTypeDef status;
    uint32_t retryCount = 0;
    
#ifdef PIXARM_WATCHDOG_ENABLE
    // Start the watchdog Timer
    HAL_TIM_Base_Start_IT(AppPixarmCblk.tim10Handle);
    AppPixarmCblk.uart_state = UART_PERIPHERAL_TRANSMITTING;
#endif
    
    /* Start the UART peripheral transmission process */
    AppPixarmCblk.requestState = UART_REQUEST_PROCESSING;
    while ( HAL_OK != (status = HAL_UART_Transmit_IT(AppPixarmCblk.handle, 
                                                     AppPixarmCblk.outputBuffer.buffer, 
                                                     OUTPUT_PIXARM_BUFFER_SIZE)))
    {
        retryCount++;
        if ( retryCount > PIXARM_TRANSACTION_RETRY_LIMIT )
        {
            // Log Error and return failed
            APP_Log("PIXARM: Error during UART transmission. IT Status: %d.\r\n", status);
            return STATUS_FAILURE;
        }
    }
    return STATUS_SUCCESS;
}

static eAPP_STATUS pixarm_receive(void)
{
    HAL_StatusTypeDef status;
    uint32_t retryCount = 0;
    
#ifdef PIXARM_WATCHDOG_ENABLE
    // Start the watchdog Timer
    HAL_TIM_Base_Start_IT(AppPixarmCblk.tim10Handle);
    AppPixarmCblk.uart_state = UART_PERIPHERAL_RECEIVING;
#endif
    
    /* Put UART peripheral in reception mode */ 
    Flush_Buffer(AppPixarmCblk.inputBuffer.buffer, INPUT_PIXARM_BUFFER_SIZE);
    Flush_Buffer(AppPixarmCblk.outputBuffer.buffer, OUTPUT_PIXARM_BUFFER_SIZE);
    AppPixarmCblk.requestState = UART_WAITING;
    while( HAL_OK != (status = HAL_UART_Receive_IT(AppPixarmCblk.handle, 
                                                   AppPixarmCblk.inputBuffer.buffer,
                                                   INPUT_PIXARM_BUFFER_SIZE)))
    {
        retryCount++;
        if ( retryCount > PIXARM_TRANSACTION_RETRY_LIMIT )
        {
            // Log Error and return failed
            APP_Log("PIXARM: Error during UART reception. IT Status: %d.\r\n", status);
            return STATUS_FAILURE;
        }
    }
    return STATUS_SUCCESS;
}

/******************************************************************************/
/*                 State Machine Management Functions                         */
/******************************************************************************/
static eAPP_STATUS pixarm_handle_request(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status;
    uAPP_PIXARM_MESSAGES request = AppPixarmCblk.inputBuffer;
    
    switch (request.common.cmd)
    {
        case PIXARM_CMD_READ_REQ:
            // Validate the Flag as end
            if ( PIXARM_FLAG_END != request.readReq.flag )
            {
                // Log Failure and return
                APP_Log("PIXARM: FLAG Bits wrong on READ_REQ Command.\r\n");
                return STATUS_FAILURE;
            }
            
#ifdef DEBUG_PIXARM
            APP_Log("PIXARM: Received CMD_READ_REQ."ENDLINE);
#endif
        
            // Process the Read Req, pulling out Rotation Completion Data
            if ( ROTATION_UNKNOWN != navigation_cblk->image_board_data.rotation )
            {
                if ( APP_Navigation_Check_Rotation(request.readReq.rotation_absolute, 
                                               navigation_cblk->navigation_data.rotation_absolute) )
                {
                    // If the previous rotation finished, override this cycles rotation calculation
                    navigation_cblk->navigation_flags.rotation_status = TRUE;
                    navigation_cblk->image_board_data.rotation = ROTATION_UNKNOWN;
                    navigation_cblk->navigation_data.rotation_absolute = ROTATION_UNKNOWN;
                }
            }
        
            // Send back a Read Data
            AppPixarmCblk.outputBuffer.readData.cmd = PIXARM_CMD_READ_DATA;
            AppPixarmCblk.outputBuffer.readData.x_intensity = navigation_cblk->navigation_data.x_axis;
            AppPixarmCblk.outputBuffer.readData.y_intensity = navigation_cblk->navigation_data.y_axis;
            AppPixarmCblk.outputBuffer.readData.z_intensity = navigation_cblk->navigation_data.z_axis;
            AppPixarmCblk.outputBuffer.readData.rotation_absolute = navigation_cblk->navigation_data.rotation_absolute;
            AppPixarmCblk.outputBuffer.readData.flag = PIXARM_FLAG_END;
            
#ifdef DEBUG_PIXARM
            APP_Log("PIXARM: Sending CMD_READ_DATA."ENDLINE);
#endif
        
            // Transmit Response
            status = pixarm_transmit();
            return status;
        
        case PIXARM_CMD_SYNC:
        case PIXARM_CMD_DYNC:
        case PIXARM_CMD_ACK:
        case PIXARM_CMD_READ_DATA:
        default:
            APP_Log("PIXARM: Recieved Invalid Command %x in state %x.\r\n", request.common.cmd, AppPixarmCblk.state);
            AppPixarmCblk.state = PIXARM_ERROR;
            return STATUS_FAILURE;
    }
    
}


static eAPP_STATUS pixarm_state_machine(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_FAILURE;
    
    switch( AppPixarmCblk.state )
    {   
        case PIXARM_PROCESS:
            status = pixarm_handle_request(navigation_cblk);
            return status;

        case PIXARM_HANDSHAKE:
        case PIXARM_SHUTDOWN:
        case PIXARM_INIT:
        case PIXARM_ERROR:
        default:
            APP_Log("PIXARM Driver in Bad State %d.\r\n", AppPixarmCblk.state);
            AppPixarmCblk.state = PIXARM_ERROR;
            return STATUS_FAILURE;
    }
}


/* Public functions ---------------------------------------------------------*/
/******************************************************************************/
/*                 Callback Functions                                     */
/******************************************************************************/
#ifdef PIXARM_WATCHDOG_ENABLE
void PIXARM_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if ( TIM10 == htim->Instance )
    {
        // Handle Message Timeout
        APP_Log("PIXARM: UART Message Timeout.\r\n");
        __HAL_UART_DISABLE(AppPixarmCblk.handle);
        __HAL_UART_ENABLE(AppPixarmCblk.handle);
        APP_UART_Generic_Flush_Buffer(AppPixarmCblk.handle);
        
        // Handle the different timeout possibilites differently
        if ( UART_PERIPHERAL_TRANSMITTING == AppPixarmCblk.uart_state )
        {
            // Cancel the previous sending state and retry
            AppPixarmCblk.requestState = UART_REQUEST_WAITING;
        }
        else if ( UART_PERIPHERAL_RECEIVING == AppPixarmCblk.uart_state )
        {
            AppPixarmCblk.requestState = UART_NO_REQUEST;
        }
        else
        {
            APP_Log("PIXARM: UART Timeout in Unknown State.\r\n");
            AppPixarmCblk.state = PIXARM_ERROR;
            BSP_LED_On(BSP_PIXARM_ERROR_LED);
        }
        AppPixarmCblk.uart_state = UART_PERIPHERAL_IDLE;
        
        // Reset the Watchdog
        HAL_TIM_Base_Stop_IT(htim);
        __HAL_TIM_SET_COUNTER(htim, 0);
    }
}
#endif

/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of IT Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void PIXARM_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
#ifdef PIXARM_WATCHDOG_ENABLE
    // Reset the Watchdog
    HAL_TIM_Base_Stop_IT(AppPixarmCblk.tim10Handle);
    __HAL_TIM_SET_COUNTER(AppPixarmCblk.tim10Handle, 0);
    AppPixarmCblk.uart_state = UART_PERIPHERAL_IDLE;
#endif
    
    // Transmission was successful, so ready to listen for a new request
    AppPixarmCblk.requestState = UART_NO_REQUEST;
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void PIXARM_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#ifdef PIXARM_WATCHDOG_ENABLE
    // Reset the Watchdog
    HAL_TIM_Base_Stop_IT(AppPixarmCblk.tim10Handle);
    __HAL_TIM_SET_COUNTER(AppPixarmCblk.tim10Handle, 0);
    AppPixarmCblk.uart_state = UART_PERIPHERAL_IDLE;
#endif
    
    // Reception was successful, so ready for processing
    AppPixarmCblk.requestState = UART_REQUEST_WAITING;
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void PIXARM_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
#ifdef PIXARM_WATCHDOG_ENABLE
    // Reset the Watchdog
    HAL_TIM_Base_Stop_IT(AppPixarmCblk.tim10Handle);
    __HAL_TIM_SET_COUNTER(AppPixarmCblk.tim10Handle, 0);
    AppPixarmCblk.uart_state = UART_PERIPHERAL_IDLE;
#endif
    
    /* Turn On BSP_PIXARM_ERROR_LED */
    BSP_LED_On(BSP_PIXARM_ERROR_LED);
    
    // Log Error and Return Failure
    APP_Log("PIXARM: UART Error Occured: %s. Transitioning to Error Recovering.\r\n",
            APP_UART_Generic_Translate_Error(huart->ErrorCode));
    AppPixarmCblk.prev_state = AppPixarmCblk.state;
    AppPixarmCblk.state = PIXARM_ATTEMPT_RECOVERY;
}


/**
  * @brief  Performs initialization of the Application-specific I2C.
  * @param  None
  * @retval None
  */
void APP_PIXARM_Init(void)
{
    /*##-1- Configure the UART peripheral #######################################*/
    PixarmHandle.Instance          = PIXARM_USART;
    PixarmHandle.Init.BaudRate     = 115200;
    PixarmHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    PixarmHandle.Init.StopBits     = UART_STOPBITS_1;
    PixarmHandle.Init.Parity       = UART_PARITY_NONE;
    PixarmHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    PixarmHandle.Init.Mode         = UART_MODE_TX_RX;
    PixarmHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if(HAL_UART_Init(& PixarmHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }
    
    AppPixarmCblk.handle = & PixarmHandle;
    APP_UART_Generic_Flush_Buffer(AppPixarmCblk.handle);
    
#ifdef PIXARM_WATCHDOG_ENABLE
    pixarm_watchdog_init();
#endif
    
    AppPixarmCblk.state = PIXARM_HANDSHAKE;
}


eAPP_STATUS APP_PIXARM_Initiate(void)
{
    uAPP_PIXARM_MESSAGES message;
    uint8_t errorCount = 0;
    
    // Validate we are in correct state
    if ( PIXARM_HANDSHAKE != AppPixarmCblk.state )
    {
        APP_Log("PIXARM in incorrect state for Handshake Procedure.\r\n");
        return STATUS_FAILURE;
    }
    
    // Start the Handshake Procedure (Syncronous Process)
    APP_Log("Starting PIXARM Handshake.\r\n");
    
    // Send the Packet
    do {
        if (HAL_OK != HAL_UART_Receive(AppPixarmCblk.handle, 
                                       AppPixarmCblk.inputBuffer.buffer, 
                                       INPUT_PIXARM_BUFFER_SIZE,
                                       PIXARM_POLL_TIMEOUT))
        {
            APP_Log("PIXARM: Error in reception of SYNC.\r\n");
            errorCount++;
        }
        else
        {
            break;
        }
        if ( (uint8_t) PIXARM_CONNECTION_ATTEMPTS <= errorCount)
        {
            // Log Error and Return Failed
            APP_Log("PIXARM: Error in reception of SYNC, abandoning Process.\r\n");
            return STATUS_FAILURE;
        }
        
    } while( errorCount < (uint8_t) PIXARM_CONNECTION_ATTEMPTS );
    
    // Process the Handshake SYNC Packet
    message.sync = AppPixarmCblk.inputBuffer.sync;
    
    // Validate the CMD
    if ( PIXARM_CMD_SYNC != message.sync.cmd )
    {
        // Log Failure and return
        APP_Log("PIXARM: CMD Bits wrong on SYNC Command.\r\n");
        return STATUS_FAILURE;
    }
    
    // Validate the Flag as end
    if ( PIXARM_FLAG_END != message.sync.flag )
    {
        // Log Failure and return
        APP_Log("PIXARM: FLAG Bits wrong on SYNC Command.\r\n");
        return STATUS_FAILURE;
    }
    
    // Format the Handshake SYNC Packet
    AppPixarmCblk.outputBuffer.sync.cmd = PIXARM_CMD_SYNC;
    AppPixarmCblk.outputBuffer.sync.flag = PIXARM_FLAG_END;
    
    if ( HAL_OK != HAL_UART_Transmit(AppPixarmCblk.handle, 
                                     AppPixarmCblk.outputBuffer.buffer, 
                                     OUTPUT_PIXARM_BUFFER_SIZE, 
                                     PIXARM_POLL_TIMEOUT))
    {
        // Log Error and return failed
        APP_Log("I2C: Error in transmission of SYNC.\r\n");
        return STATUS_FAILURE;
    }
    
    // Change states
    APP_Log("Finished I2C Handshake, starting Interrupt Process.\r\n");
    AppPixarmCblk.state = PIXARM_PROCESS;
    
    /* Put I2C peripheral in reception process */ 
    if ( STATUS_FAILURE != pixarm_receive() )
    {
        return STATUS_SUCCESS;
    }
    else
    {
        APP_Log("I2C: Error in reception call after Handshake.\r\n");
        AppPixarmCblk.state = PIXARM_ERROR;
        BSP_LED_On(BSP_PIXARM_ERROR_LED);
        return STATUS_FAILURE;
    }
}


eAPP_STATUS APP_PIXARM_Process_Message(sAPP_NAVIGATION_CBLK* navigation_cblk)
{
    eAPP_STATUS status = STATUS_SUCCESS;
    
    // If we are not in an invalid state
    if ( (PIXARM_INIT != AppPixarmCblk.state)
      && (PIXARM_ERROR != AppPixarmCblk.state ) )
    {
        // If a message is waiting, process it
        if ( UART_REQUEST_WAITING == AppPixarmCblk.requestState )
        {
            // Process Message
            status = pixarm_state_machine(navigation_cblk);
        }
        else if ( UART_NO_REQUEST == AppPixarmCblk.requestState )
        {
            // Enter the Reception state
            status = pixarm_receive();
        }
    }
    else if (PIXARM_ATTEMPT_RECOVERY == AppPixarmCblk.state)
    {
        APP_UART_Generic_Recover_From_Error(AppPixarmCblk.handle);
        AppPixarmCblk.state = AppPixarmCblk.prev_state;
    }
    
    // Signal any failures via LED
    if ( STATUS_FAILURE == status )
    {
        BSP_LED_On(BSP_PIXARM_ERROR_LED);
        AppPixarmCblk.state = PIXARM_ERROR;
    }
    return status;
}

/* -------------------------- End of File ------------------------------------*/
