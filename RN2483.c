/******************************************************************************
 * File           : RN2483 Library
 *					https://github.com/eriknl1982/Loralem-RN2483-sample/tree/master/libraries/RN2483/src
 *****************************************************************************/
#include "RN2483.h"

void USART_Setup(void) // We are going to setup the USART output. How about pin9(TX) and pin10(RX)? Oh and we can also enable the interrupt (I think).
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  
  // Initialize USART1
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  
  // Setup Tx- and Rx pin
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);
  
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
	// Setup USART with the settings that RN require from me...
  USART_StructInit(&USART_InitStructure);
	
  USART_InitStructure.USART_BaudRate = 57600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);
  
	// USART1 interrupts enable in NVIC
  NVIC_EnableIRQ(USART1_IRQn);
  NVIC_SetPriority(USART1_IRQn, 0);
  NVIC_ClearPendingIRQ(USART1_IRQn);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
  USART_Cmd(USART1, ENABLE);
	
	// Enable the nice RN led on the LoraBee. Because why not
	SendSettingCommand(RN_SYS_SET_PIN, "1");
}

void SendCommand(char *str) // So we are going to send a string to the RN module? Smart
{
	//Send string in pieces
	while(*str)
	{
		USART_Send(*str++);
	}
	
	//End line
	USART_Send('\r');
	USART_Send('\n');
}

void USART_Send(uint16_t data) // Lets send some data to the USART output. We like to use USART1
{
	//Check if USART1 is ready to send
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){;}
		
	USART_SendData(USART1, data);
}

void SendSettingCommand(char *setting, char *value) // Lets send a mac set command to our rn module. Because why not
{
  //Send setting string
  while (*setting)
  {
      USART_Send(*setting++);
  }

	//Send space
  USART_Send(0x20);	
	
  //Send value
  while (*value)
  {
      USART_Send(*value++);
  }	
	
	//End line
	USART_Send('\r');
	USART_Send('\n');
}

void sendDataCommand(char *string, uint8_t* appData, uint8_t dataLength) // Lets send some awesome data to our lora server
{
	uint8_t byteCount;
	
  //Send string
  while (*string)
  {
      USART_Send(*string++);
  }
	
	//Send Port No & a space
  for (byteCount = 0; byteCount < 3; byteCount++)
  {
      USART_Send(appData[byteCount]);
  }
	
	//Send another space because RN likes this
  USART_Send(0x20);
	
  //Send data
  for (byteCount = 4; byteCount < dataLength; byteCount++)
  {
      
  }
	
	//End line
	USART_Send('\r');
	USART_Send('\n');
}

bool ReceiveData(char *string) // Is our string the same as the string of the RN module??
{
	NVIC_DisableIRQ(USART1_IRQn);
	if(strcmp((char *)responseBuffer, string) == 0)
	{
		NVIC_EnableIRQ(USART1_IRQn);
		return true;
	}
	else
	{
		NVIC_EnableIRQ(USART1_IRQn);
		return false;
	}

}

void RN_Handler(void) // Should be placed in the main while. If not, then its not
{
	bool receivedData = CheckBufferReady();
	
	switch(activeSystemState)
	{
		default:
			break;
		case rnJoining:
			if(RN_Join(receivedData))
			{
				activeSystemState = rnRunning;
			}
			break;
		case rnRunning:
			RN_Running(receivedData);
			break;
	}
}

void RN_Receiver(void) // Used in the interrupter file (USART1_IRQHandler). If not, maybe there is something wrong?
{
	uint16_t tempBuffer = 0;
	while(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		tempBuffer = USART_ReceiveData(USART1);
		
		if(tempBuffer == 0x0D) // The awesome HEX code for CR. 
		{
			responseBuffer[responseBufferIndex] = 0x00; // We dont need CR. We have NULL
			responseBufferIndex = 0;
			responseReady = true;
		}
    else if ( (tempBuffer == 0x00) || (tempBuffer == 0x0A) )
    {
			// Why do we get NULL and New Line from our module :/
    }
    else
    {
      responseBuffer[responseBufferIndex] = tempBuffer; // So, now we add the RN ascii code to our buffer. 
      responseBufferIndex++;
    }
	}
}

bool CheckBufferReady(void) // Check if we have something from the RN to read
{
	if(responseReady)
	{
		responseReady = false;
		return true;
	}
	else
	{
		return false;
	}
}

bool RN_Join(bool msgReady) // Lets join the lora network
{
	bool joinComplete = false;
	
	switch(activeJoinState)
	{
		default: // ERROR: You should never enter this state
			break;
		case rnStartup:
			SendCommand(RN_SYS_RESET); // Always reboot RN before we start with annoying him
			activeJoinState = rnReset;
			break;
		case rnReset:
			if(msgReady)
			{
				if(strncmp((char *)responseBuffer, "RN", 2) == 0) // Yee the module is on
				{
					activeJoinState = rnSetInfo;
				}
				else
				{
					activeJoinState = rnComFailure;
				}
			}
			break;
		case rnSetInfo:
			if(appJoin == 0) // Set the ABP settings. We dont check if info is correct, it just need to be correct
			{
				SendSettingCommand(RN_MAC_SET_DEV_ADDR, devAddr);
				Delay(1000);
				SendSettingCommand(RN_MAC_SET_NWK_SESS_KEY, nwkSKey);	
				Delay(1000);				
				SendSettingCommand(RN_MAC_SET_APP_SESS_KEY, appSKey);
				Delay(1000);
				activeJoinState = rnJoinMethod;
			}
			else if(appJoin == 1) // Set the OTAA settings. We dont check if info is correct, it just need to be correct
			{
				SendSettingCommand(RN_MAC_SET_APP_EUI, appEui);
				Delay(1000);
				SendSettingCommand(RN_MAC_SET_APP_KEY, appKey);
				Delay(1000);
				activeJoinState = rnJoinMethod;
			}
			break;
		case rnJoinMethod:
			if(msgReady)
			{
				if(appJoin == 0) // We are going to use ABP to join the Lora network. Smart??
				{
					SendCommand(RN_JOIN_ABP_MODE);
					activeJoinState = rnCheckAbp;
				}
				else if(appJoin == 1) // Lets use OTAA to join the Lora network. Smarter??
				{
					SendCommand(RN_JOIN_OTAA_MODE);
					activeJoinState = rnCheckOtaa;
				}
				joiningAttempts++;
				if(joiningAttempts > RN_RETRY_ATTEMPTS) // Why would we continue with an error that we most likely cant solve...
				{
					activeJoinState = rnComFailure;
				}
			}
			break;
		case rnCheckAbp:
			if(msgReady)
			{
				if(ReceiveData("ok")) // Starting ABP. Lets do this
				{
					activeJoinState = rnWaitAbp;
				}
				else if(ReceiveData("keys_not_init")) // GGrrr... Read state "rnNeedKeys"
				{
					activeJoinState = rnNeedKeys;
				}
				else
				{
					SendCommand(RN_JOIN_ABP_MODE);
				}
			}
			break;
		case rnWaitAbp:
			if(msgReady)
			{
				if(ReceiveData("accepted")) // Wow, ABP is accepted.
				{
					activeJoinState = rnJoined;
				}
				else
				{
					activeJoinState = rnStartup;
				}
			}
			break;
		case rnCheckOtaa:
			if (msgReady)
			{
					if(ReceiveData("ok")) // Starting OTAA. Here we go
					{
							activeJoinState = rnWaitOtaa;
					}
					else if(ReceiveData("keys_not_init"))  // no..no..no.. Read state "rnNeedKeys"
					{
							activeJoinState = rnNeedKeys;
					}
					else if(ReceiveData("invalid_param")) // Hmm command not found? Lets try again
					{
							activeJoinState = rnJoinMethod;
					}
					else if(ReceiveData("no_free_ch")) // No channel available. Restart RN so channels get available. Maybe after some rest RN will do someting. ZZzzZzzz
					{   
							activeJoinState = rnStartup;
					}
			}
			break;
		case rnWaitOtaa:
			if (msgReady)
			{
					if(ReceiveData("accepted")) //OTAA accepted. Ready to roll
					{
							activeJoinState = rnJoined;
					}
					else if(ReceiveData("denied")) //OTAA denied. Retry joining. Can happen when there is no connection available.
					{
							activeJoinState = rnJoinMethod;
					}
					else if(ReceiveData("no_free_ch")) // No channel available. Let punish RN and reboot him so we get our channels back
					{  
							activeJoinState = rnStartup;
					}
			}
			break;
		case rnJoined:
			joiningAttempts = 0;
			joinComplete = true; // Yee, joining is complete. Lets send some awesome messages
			break;
		case rnNeedKeys:
			// Keys are not provided in .H file or they are invalid. Why... How can you do this to me :-(
			
		  break;
		case rnComFailure:
			// I think RN is sick. He is not responding correct. (Maybe he is dead :/ )
		  break;
	}
	
	return joinComplete;
}

void RN_Running(bool msgReady) // When we are ready to rock and roll on the Lora network. Ofcourse after we are accepted by the lora network bosses
{
	
}

void Delay(const int d) // Sometimes we have to wait to get something done
{
  volatile int i;

  for(i=d; i>0; i--){ ; }

  return;
}
