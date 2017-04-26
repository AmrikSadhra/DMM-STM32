/*------------------------------------------------------*
 * Name:    DMM.c 																			*
 *------------------------------------------------------*/
#include "DMM.h"

/*--------- Internal Functions (Prototypes) ----------*/
void frequencyResponseMenu(int sweepStart, int sweepEnd, int sweepResolution);
void signalGenerationMenu(uint32_t genFrequency, float genAmplitude, uint8_t sigGenType);
void menu(void);

/* Stage Initialisation check booleans */
bool alphaInit = false;
bool betaInit = false;
bool gammaInit = false;

/* Initialise Mode Switch GPIO Pins for Mux (MUX IDENTIFIER HERE) */
void mode_switch_init(){
		//Initialisation of stage Alpha
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		GPIO_InitTypeDef GPIO_InitStruct2;
		GPIO_InitStruct2.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
		GPIO_InitStruct2.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct2.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStruct2.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct2.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOB, &GPIO_InitStruct2);
	
		//Initialisation of Stage Beta
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 ;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/* Function to intiialise all used peripherals    */
void initialise_Peripherals(void){ 
			lcd_init(LCD_LINES_TWO, LCD_CURSOR_OFF, LCD_CBLINK_OFF, BUFFER_SIZE);
			ADC1_init();
			serial_init(9600);
			bluetooth_init(9600);
			switch_init();
			LED_Init();
			mode_switch_init();
			//Connect Read stage to Voltage
			stageAlpha(PROBE_VOLTAGE);
}


//Stage Alpha - 3 Stage Selection: Ground, Voltage,Capacitance
void stageAlpha(int mode){	
			if(!alphaInit){
				mode_switch_init();
				alphaInit = true;
			}
			
			//Clear Range switch bits
			GPIOB->ODR &= ~(3UL << 4);
			switch(mode){
				case PROBE_GROUND:						//J5 4,3 (GPIOB 5 4): 
					GPIOB->ODR &= ~(3UL << 4); 	//   0 0 Ground       &= ~(3UL << 4) Clear to 00
					break;
				case PROBE_VOLTAGE:
					GPIOB->ODR |= (1UL << 4);		//   0 1 Voltage      |=  (1UL << 4)/
					break;
				case PROBE_CAPACIITANCE:
					GPIOB->ODR |= (2UL << 4);		//   1 0 Capacitance  |=  (2UL << 4)
					break;
				default:
					break;
			}
}

//Stage Beta - 4 Mode Switch: Current/Resistance/RMS/Voltage Circuit
void stageBeta(int mode){
	if(!betaInit){
				mode_switch_init();
				betaInit = true;
			}

		//Clear bits 
		GPIOE->ODR &= ~(3UL << 5);
		switch(mode){
			case 0://Voltage            //J7 5,4 (GPIOE 6 5): 
				GPIOE->ODR &= ~(3UL << 5);//   0 0 Voltage      &= ~(3UL << 5) Clear to 00
				break;
			case 1://Current
				GPIOE->ODR |= (1UL << 5); //   0 1 Current      |=  (1UL << 5)
				break;
			case 2://RMS
				GPIOE->ODR |= (2UL << 5); //   1 0 RMS          |=  (2UL << 5)
				break;
			case 3://Resistance
				GPIOE->ODR |= (3UL << 5); //   1 1 Resistance   |=  (3UL << 5)
				break;
			default:
				break;
		}
}

//Stage Gamma
//J7 3,2 (GPIOE 4 3): Range for V/R/I/Universe
//   0 0 Gain 1
//   0 1 Gain 1000   
void stageGamma(int mode){
	if(!gammaInit){
				mode_switch_init();
				gammaInit = true;
	}

	switch(mode){
				case 0://10v in 
					//Do nothing for Range 0 as pins already cleared
					break;			
				case 1://1v in 
					//Range 1
					GPIOE->ODR |= (1UL << 3);
					break;
				case 2://0.1v
					//Range 2
					GPIOE->ODR |= (2UL << 3);
					break;
				case 3://0.01v
					//Range 3
					GPIOE->ODR |= (3UL << 3);
					break;
	}
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
  SystemCoreClockUpdate();                      /* Get Core Clock Frequency   */
  if (SysTick_Config(SystemCoreClock / 1000)) { /* SysTick 1 msec interrupts  */
    while (1);                                  /* Capture error              */
  }
	
		initialise_Peripherals();
		lcd_clear_display();
		lcd_write_string("Multimeter Starting..", 0, 0);
  
	while(1) {                                    /* Loop forever               */
		menu();
	}
}

void menu(){
	//Bluetooth Override of Buttons
	char *bluetoothSwitchPacket = DequeueString(bluetoothQueue);
	
	#ifdef DMM_DEBUG
		printf("[Android Client] Data Received: %s\r\n", bluetoothSwitchPacket);
	#endif	
	
	//Read ADC and use this value for menu calculations
	double ADC1_valueScaled = read_ADC1();
	
	//Menu Position variables
	static int bluetoothMenuPosition;
	static int prevBluetoothMenuPosition;
	static int prevLocalMenuPosition;
	static int menuPosition = 1;
	
	//Frequency Sweep Parameters
	uint32_t sweepStart = 0, sweepEnd = 0, sweepResolution = 0;
	//Signal Generation Parameters
	uint32_t genFrequency = 0;
	float genAmplitude = 0.0;
	uint8_t sigGenType = 0;
	
		//Query the Bluetooth Data to identify mode switches
		if(strcmp(bluetoothSwitchPacket,"<m:1>") == 0) bluetoothMenuPosition = VOLTAGE_READ_STATE;
		if(strcmp(bluetoothSwitchPacket,"<m:2>") == 0) bluetoothMenuPosition = CURRENT_READ_STATE;
		if(strcmp(bluetoothSwitchPacket,"<m:3>") == 0) bluetoothMenuPosition = RESISTANCE_READ_STATE;
		if(strcmp(bluetoothSwitchPacket,"<m:4") == 0){ 
			bluetoothMenuPosition = 4;
			//Parse frequency sweep data from packet
			int n = sscanf(bluetoothSwitchPacket, "<m:4;start:%d;end:%d;steps:%d>", &sweepStart, &sweepEnd, &sweepResolution);
			#ifdef DMM_DEBUG
				if(n != 3) printf("[Android Client] Failed to parse Frequency data from Client Frequency Response request!\r\n"); //If not parsed 3 items from string
			#endif
		}
		if(strcmp(bluetoothSwitchPacket,"<m:5") == 0){ 
			bluetoothMenuPosition = 5;
			
			char *genType; //Temporary variable with which to parse signal type to int
			//TODO: Will sscanf work with char buffer? Get Jonathan to refactor to send ints and document which int corresponds to which Wave (dac.h defines)
			//Parse signal generation data from packet
			int n = sscanf(bluetoothSwitchPacket, "<m:5;freq:%d;ampl:%f;type:%s>", &genFrequency, &genAmplitude, genType);
			
			//Convert Strings to Wave integers to reduce processing overhead for Strings
			if(strcmp(genType,"sinusoidal") == 0){
				sigGenType = SINE_TYPE;
			} else if(strcmp(genType,"square") == 0){
				sigGenType = SQUARE_TYPE;
			} else if(strcmp(genType,"triangle") == 0){
				sigGenType = SAW_TYPE;
			} 
			
			#ifdef DMM_DEBUG
			if(n != 3) printf("[Android Client] Failed to parse data from Client Signal generation request! Parsed: %d\r\n", n); //If not parsed 3 items from string
			#endif
		}
		if(strcmp(bluetoothSwitchPacket,"<m:6>") == 0) bluetoothMenuPosition = AC_VOLTAGE_READ_STATE;
		if(strcmp(bluetoothSwitchPacket,"<m:7>") == 0) bluetoothMenuPosition = CAPACITANCE_STATE;
		if(strcmp(bluetoothSwitchPacket,"<m:8>") == 0) bluetoothMenuPosition = DIODE_STATE;
		if(strcmp(bluetoothSwitchPacket,"<m:9>") == 0) bluetoothMenuPosition = LIGHT_INTENSITY_STATE;

	
		//Enable bluetooth override of localMenuPositoon
		if(prevBluetoothMenuPosition != bluetoothMenuPosition){
				#ifdef DMM_DEBUG
					printf("[Android Client] Changing to menu: %d due to Android menu switch request\r\n", bluetoothMenuPosition); //If not parsed 3 items from string
				#endif
				//Bluetooth has changed value, update menu Position so bluetooth commands menu
				menuPosition = bluetoothMenuPosition;	
				//Update previous menu position
				prevBluetoothMenuPosition = bluetoothMenuPosition;
		}
	
	//Detect menu position change and perform menu switch logic to kill old DMM function
	if(prevLocalMenuPosition !=  localMenuPosition){
		if(prevLocalMenuPosition == 5){ //If switching away from Signal Generation
			stopGenerating(); //Stop DMA operation for sig gen
		}
		//Local menu position has changed value, update menu Position so STM board commands menu
		menuPosition = localMenuPosition;
		//Update previous menu position
		prevLocalMenuPosition = localMenuPosition;
	} else {
		if(prevLocalMenuPosition == 5){ //If still in signal Generation, no need to change menu
			return;
		}
	}

	//Clear display in advance of menu update
	lcd_clear_display();
	
	double current =0;
	double resistance = 0;
	
	//If Menu goes outside implemented features, these strings will propagate to LCD
	char lcd_line1[16] ="    MODE  ";
	char lcd_line2[16] ="  CHANGING";
	
	switch(menuPosition){
		case 0: //If menu position not initialised, go to voltage (No break statement)
		case VOLTAGE_READ_STATE: //voltage
			stageBeta(0);
				switch(ADC1_currentRange){
					case 0:
						sprintf(lcd_line1,"Volt:0->10V");
						sprintf(lcd_line2,"%.2lf V",ADC1_valueScaled);
						break;
					case 1:
						sprintf(lcd_line1,"Volt:0->1V");
						sprintf(lcd_line2,"%lf V",ADC1_valueScaled);
						break;
					case 2:
						sprintf(lcd_line1,"Volt:0->100mV");
						sprintf(lcd_line2,"%lf mV",ADC1_valueScaled*1000);
						break;
					case 3:
						sprintf(lcd_line1,"Volt:0->10mV");
						sprintf(lcd_line2,"%lf mV",ADC1_valueScaled*1000);
						break;
					default://Invalid range
						ADC1_currentRange = 0;
						break;
			}
			sendPacket(1, ADC1_valueScaled, ADC1_currentRange, 0);
			break;
			
			case CURRENT_READ_STATE://current
			stageBeta(1);
			current = ADC1_valueScaled/10;
			switch(ADC1_currentRange){
				case 0:
					sprintf(lcd_line1,"Amp:-1->1A");
					sprintf(lcd_line2,"%lf A",current);
					break;
				case 1:
					sprintf(lcd_line1,"Amp:-100m->100mA");
					sprintf(lcd_line2,"%lf A",current);
					break;
				case 2:
					sprintf(lcd_line1,"Amp:-10m->10mA");
					sprintf(lcd_line2,"%lf mA",current*1000);
					break;
				case 3:
					sprintf(lcd_line1,"Amp:-1m->1mA");
					sprintf(lcd_line2,"%lf mA",current*1000);
					break;
				default://Invalid range
					ADC1_currentRange = 0;
					break;
			}
			sendPacket(2, current, ADC1_currentRange, 0);
			break;
			
		case RESISTANCE_READ_STATE://resistance
			stageBeta(3);
			resistance = fabs(ADC1_valueScaled)/0.000010;
			sprintf(lcd_line2,"%lf",resistance);
			switch(ADC1_currentRange){
				case 0:
					sprintf(lcd_line1,"Res:0->1M Ohm");
					break;
				case 1:
					sprintf(lcd_line1,"Res:0->100k Ohm");
					break;
				case 2:
					sprintf(lcd_line1,"Res:0->10k Ohm");
					break;
				case 3:
					sprintf(lcd_line1,"Res:0->1k Ohm");
					break;
				default://invalid range
					ADC1_currentRange =0;
					break;
			}
			sendPacket(RESISTANCE_READ_STATE, resistance, ADC1_currentRange, 0);
			break;
			
		case FREQUENCY_RESP_STATE:	//Frequency Response
			frequencyResponseMenu(sweepStart, sweepEnd, sweepResolution);
			break;
		
		case SIG_GEN_STATE:					//Signal Generation
			signalGenerationMenu(genFrequency,  genAmplitude, sigGenType);
			return;
		
		case AC_VOLTAGE_READ_STATE: //Measure AC voltage and frequency
		
			break;
		
		case CAPACITANCE_STATE://Measure Capacitance
			//sprintf(lcd_line1,"Capacitance");
			//sprintf(lcd_line2,"%dF", numHighTicks);
			if(timerDone) measureCapacitance();
			break;
		
		case DIODE_STATE:
			localMenuPosition = VOLTAGE_READ_STATE; //Return from non implemented feature to voltage
		break;
		
		case LIGHT_INTENSITY_STATE: //Measure light intensity (only available through Android due to not enough buttons)
			sprintf(lcd_line1,"Light Intensity");
			sprintf(lcd_line2,"%.1f %%", ((ADC1_valueScaled + 10)/20)*100);
			sendPacket(LIGHT_INTENSITY_STATE, ADC1_valueScaled, 0, 0);
			break;
		
		default:
			localMenuPosition = VOLTAGE_READ_STATE; //If entered stray state, return to voltage mode
			break;
	}
	//Update LCD Display with menu text
	lcd_write_string(lcd_line1, 0, 0);
	lcd_write_string(lcd_line2, 1, 0);
}


void signalGenerationMenu(uint32_t genFrequency, float genAmplitude, uint8_t sigGenType){
	//Android hasnt set any data, so use board buttons to manually set sweep parameters
	if((genFrequency == 0)&&(compare_float(genAmplitude, 0.0f))&&(sigGenType == 0)){
		
		//Buffers holding Text for LCD
		char line1Buf[16] = "     Signal   ";
		char line2Buf[16] = "   Generation ";
		
		//Write initial menu message
		lcd_write_string(line1Buf, 0, 0);
		lcd_write_string(line2Buf, 1, 0);
		Delay(1000);
		
		//Set Defaults
		uint32_t stepSize = MANUAL_DEFAULT_STEP;
		genFrequency = MANUAL_DEFAULT_FREQ;
		int entriesDone = 0;
		localMenuPosition = UPDATE_DISPLAY; //Update display immediately
			
		while((entriesDone != 3)&&(entriesDone != -1)){ //Whilst Still data to collect (3 values needed), and not cancelled (Entries done = -1)
			switch(localMenuPosition){
				case MANUAL_VALDOWN_BUTTON:
					switch(entriesDone){
						case 0: //Wave Type Data collection
							//Drop to lower wave type if not wave type 0 
							if(sigGenType != 0) sigGenType--; 
						break;
						
						case 1: //Frequency Data collection
						//If freqDown is valid 
						if((genFrequency - stepSize) >= MANUAL_MIN_FREQ){
							genFrequency -= stepSize;
						}
						break;
						
						case 2: //Amplitude Data Collection
						if((genAmplitude - MANUAL_AMP_STEPSIZE) >= MANUAL_MIN_AMP){
							genAmplitude -= MANUAL_AMP_STEPSIZE;
						}
						break;
					}
				localMenuPosition = UPDATE_DISPLAY; //Update Display next time through switch statement
				break;
					
				case MANUAL_VALUP_BUTTON:
					switch(entriesDone){
						case 0: //Wave Type Data collection
							 //Bump up to higher wave type if not max wave type
							if(sigGenType != 2) sigGenType++;
						break;
						
						case 1: //Frequency Data collection
							//If freqUp is valid 
							if((genFrequency + stepSize) <= MANUAL_MAX_FREQ){
								genFrequency += stepSize;
							}
						break;
						
						case 2: //Amplitude Data Collection
							if((genAmplitude + MANUAL_AMP_STEPSIZE) <= MANUAL_MAX_AMP){
								genAmplitude += MANUAL_AMP_STEPSIZE;
							}
						break;
					}
				localMenuPosition = UPDATE_DISPLAY;
				break;
					
				case MANUAL_STEPDOWN_BUTTON:
						switch(entriesDone){
							case 1: //Frequency Step Data collection
								//If stepDown is valid 
								if((stepSize / MANUAL_STEP_MULTIPLIER) >= MANUAL_MIN_STEPSIZE){
									stepSize /= MANUAL_STEP_MULTIPLIER;
								}
								break;
						}
				localMenuPosition = UPDATE_DISPLAY;
				break;
				
				case MANUAL_STEPUP_BUTTON:
					switch(entriesDone){
							case 1: //Frequency Step Data collection
								//If stepUp is valid 
								if((stepSize * MANUAL_STEP_MULTIPLIER) <= MANUAL_MAX_STEPSIZE){
									stepSize *= MANUAL_STEP_MULTIPLIER;
								}
							break;
						}
				localMenuPosition = UPDATE_DISPLAY; 
				break;
					
				case UPDATE_DISPLAY:
					lcd_clear_display(); //Clear display so ready for new data
					switch(entriesDone){
						case 0: //Update LCD and serial with wave type data
							#ifdef DMM_DEBUG
								printf("[Signal Generation] Wave type changing. WaveType: %s\r\n", waveTypes[sigGenType]); 
							#endif
							sprintf(line1Buf, "%s", "Signal Type"); //Shared LCD line
							sprintf(line2Buf, "%s", waveTypes[sigGenType]);
						break;
						
						case 1: //Update LCD and serial with frequency data
							#ifdef DMM_DEBUG
								printf("[Signal Generation] Frequency Vars changing. StepSize: %d GenFrequency: %d\r\n", stepSize, genFrequency); 
							#endif	
							sprintf(line1Buf, "Freq:%d Hz", genFrequency);
							sprintf(line2Buf, "Step:%d Hz", stepSize);
						break;
						
						case 2: //Update LCD and serial with amplitude data
							#ifdef DMM_DEBUG
								printf("[Signal Generation] Amplitude changing. Amplitude: %f\r\n", genAmplitude); 
							#endif	
							sprintf(line1Buf, "%s", "Amplitude");
							sprintf(line2Buf, "%.1f v", genAmplitude);
						break;
					}
					//Update display with new data
					lcd_write_string(line1Buf, 0, 0);
					lcd_write_string(line2Buf, 1, 0);
					localMenuPosition =  NO_ACTION;  //Reset MenuPosition var to avoid infinite repeat
					break;
				
				case NO_ACTION:
					//Wait, no action to perform 
					break;
				
				case MANUAL_DONE_BUTTON:
					entriesDone++; //We're done with current variable, move to next
					localMenuPosition = UPDATE_DISPLAY;
					break;
				
				case MANUAL_CANCEL_BUTTON:
					if(entriesDone != -1) entriesDone--; //We're done with current variable, move to next
					localMenuPosition = UPDATE_DISPLAY;
					break;
			}
		}
		lcd_clear_display(); //Clear display for siggen/mode change text
		if(entriesDone != -1){ //Switch between generating Sine, Square, SAW, If user not cancelled 
			//We're done, notify user of signal generation
			
			generateSignal(genFrequency, sigGenType, genAmplitude);
			localMenuPosition = SIG_GEN_STATE; //Stay in siggen mode until user switch
		} else { 
			#ifdef DMM_DEBUG
				printf("[Signal Generation] User cancelled signal generation\r\n"); 		
			#endif	
			//Update LCD as returning immediately after function to voltage read
			lcd_write_string("    MODE  ", 0, 0);
			lcd_write_string("  CHANGING", 1, 0);
			localMenuPosition = VOLTAGE_READ_STATE;
		}
	} else {//Start Signal generation with Android sent data
			generateSignal(genFrequency, sigGenType, genAmplitude);
			localMenuPosition = SIG_GEN_STATE; //Stay in siggen mode until user switch
	}
}

void frequencyResponseMenu(int sweepStart, int sweepEnd, int sweepResolution){
	//Android hasnt set any data, so use board buttons to manually set sweep parameters
	if((sweepStart == 0)&&(sweepEnd == 0)&&(sweepResolution == 0)){

		//Buffers holding Text for LCD
		char line1Buf[16] = "    Frequency   ";
		char line2Buf[16] = "    Response    ";
		
		//Write initial menu message
		lcd_write_string(line1Buf, 0, 0);
		lcd_write_string(line2Buf, 1, 0);
		Delay(1000);
		
		//Set Defaults
		uint32_t stepSize = MANUAL_DEFAULT_STEP;
		sweepStart = MANUAL_DEFAULT_FREQ;
		sweepEnd =  MANUAL_DEFAULT_FREQ + 10;
		sweepResolution = 1;
		
		int entriesDone = 0;
		localMenuPosition = UPDATE_DISPLAY; //Update display immediately
			
		while((entriesDone != 3)&&(entriesDone != -1)){ //Whilst Still data to collect (3 values needed), and not cancelled (Entries done = -1)
			switch(localMenuPosition){
				case MANUAL_VALDOWN_BUTTON:
					switch(entriesDone){
						case 0: //SweepStart Data collection
							//If freqDown is valid 
							if((sweepStart - stepSize) >= MANUAL_MIN_FREQ){
								sweepStart -= stepSize;
							}
						break;
						
						case 1: //SweepEnd Data collection
						//If freqDown is valid 
							if((sweepEnd - stepSize) >= MANUAL_MIN_FREQ + 10){
								sweepEnd -= stepSize;
							}
						break;
						
						case 2: //SweepResolution Data Collection
						if((sweepResolution - MANUAL_SWEEP_STEPSIZE) >= MANUAL_MIN_STEP){
							sweepResolution -= MANUAL_SWEEP_STEPSIZE;
						}
						break;
					}
				localMenuPosition = UPDATE_DISPLAY; //Update Display next time through switch statement
				break;
					
				case MANUAL_VALUP_BUTTON:
					switch(entriesDone){
						case 0: //SweepStart Data collection
							//If freqUp is valid 
							if((sweepStart + stepSize) <= MANUAL_MAX_FREQ){
								sweepStart += stepSize;
							}
						break;
						
						case 1: //SweepEnd Data collection
							//If freqUp is valid 
							if((sweepEnd + stepSize) <= MANUAL_MAX_FREQ){
								sweepEnd += stepSize;
							}
						break;
						
						case 2: //SweepResolution Data Collection
							if((sweepResolution + MANUAL_SWEEP_STEPSIZE) <= MANUAL_MAX_STEP){
								sweepResolution += MANUAL_SWEEP_STEPSIZE;
							}
						break;
					}
				localMenuPosition = UPDATE_DISPLAY;
				break;
					
				case MANUAL_STEPDOWN_BUTTON:
						switch(entriesDone){
							case 0: //SweepStart Step Data collection
							case 1: //SweepEnd Step Data collection
								//If stepDown is valid 
								if((stepSize / MANUAL_STEP_MULTIPLIER) >= MANUAL_MIN_STEPSIZE){
									stepSize /= MANUAL_STEP_MULTIPLIER;
								}
								break;
						}
				localMenuPosition = UPDATE_DISPLAY;
				break;
				
				case MANUAL_STEPUP_BUTTON:
					switch(entriesDone){
						case 0: 	//SweepStart Step Data collection
							case 1: //SweepEnd Step Data collection
								//If stepUp is valid 
								if((stepSize * MANUAL_STEP_MULTIPLIER) <= MANUAL_MAX_STEPSIZE){
									stepSize *= MANUAL_STEP_MULTIPLIER;
								}
							break;
						}
				localMenuPosition = UPDATE_DISPLAY; 
				break;
					
				case UPDATE_DISPLAY:
					lcd_clear_display(); //Clear display so ready for new data
					switch(entriesDone){
						case 0: //Update LCD and serial with sweep start data
							#ifdef DMM_DEBUG
								printf("[Frequency Response] Sweep start changing: %d\r\n", sweepStart); 
							#endif
							sprintf(line1Buf, "Start:%d Hz", sweepStart);
							sprintf(line2Buf, "Step:%d", stepSize);
						break;
						
						case 1: //Update LCD and serial with sweep start data
							#ifdef DMM_DEBUG
								printf("[Frequency Response] Sweep end changing: %d\r\n", sweepEnd); 
							#endif
							sprintf(line1Buf, "End:%d Hz", sweepEnd);
							sprintf(line2Buf, "Step:%d", stepSize);
						break;
						
						case 2: //Update LCD and serial with sweep Resolution data
							#ifdef DMM_DEBUG
								printf("[Frequency Response] Sweep step size changing: %d\r\n", sweepResolution); 
							#endif	
							sprintf(line1Buf, "%s", "Step Size");
							sprintf(line2Buf, "%d", sweepResolution);
						break;
					}
					//Update display with new data
					lcd_write_string(line1Buf, 0, 0);
					lcd_write_string(line2Buf, 1, 0);
					localMenuPosition =  NO_ACTION;  //Reset MenuPosition var to avoid infinite repeat
					break;
				
				case NO_ACTION:
					//Wait, no action to perform 
					break;
				
				case MANUAL_DONE_BUTTON:
					entriesDone++; //We're done with current variable, move to next
					localMenuPosition = UPDATE_DISPLAY;
					break;
				
				case MANUAL_CANCEL_BUTTON:
					if(entriesDone != -1) entriesDone--; //We're done with current variable, move to next
					localMenuPosition = UPDATE_DISPLAY;
					break;
			}
		}

		if(entriesDone != -1){ //Start frequency response with Android sent data, If user not cancelled 
			//We're done. Notify user of ongoing sweep
			frequencyResponse(sweepStart, sweepEnd, sweepResolution);
			//TODO: Make the screen display a pretty frequency response
			//TODO: Alter Frequency Response to return a double array of results if called from menu
		} else { 
			#ifdef DMM_DEBUG
				printf("[Frequency Response] User cancelled frequency response\r\n"); 		
			#endif	
		}
		localMenuPosition = VOLTAGE_READ_STATE; //Move to Voltage measuring mode when response done or exiting
		lcd_clear_display();
	} else {//Start frequency response with Android sent data
		frequencyResponse(sweepStart, sweepEnd, sweepResolution);
	}
}
