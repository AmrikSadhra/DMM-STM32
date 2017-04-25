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
			serial_init(9600);
			bluetooth_init(9600);
			switch_init();
			LED_Init();
			mode_switch_init();
			//Connect Read stage to Voltage
			stageAlpha(1);
}

//has three stages: ground, voltage,capacitance
//Stage Alpha
//J5 4,3 (GPIOB 5 4): 
//   0 0 Ground       &= ~(3UL << 4) Clear to 00
//   0 1 Voltage      |=  (1UL << 4)
//   1 0 Capacitance  |=  (2UL << 4)
void stageAlpha(int mode){	
			if(!alphaInit){
				mode_switch_init();
				alphaInit = true;
			}
			//Clear Range switch bits
			GPIOB->ODR &= ~(3UL << 4);
			switch(mode){
				case 0:
					GPIOB->ODR &= ~(3UL << 4);
					break;
				case 1:
					GPIOB->ODR |= (1UL << 4);//setting mode to voltage
					break;
				case 2:
					GPIOB->ODR |= (2UL << 4);
					break;
				default:
					break;
			}
}

//Stage Beta
//J7 5,4 (GPIOE 6 5): Mode Switch: Current/Resistance/Voltage Circuit
//   0 0 Voltage      &= ~(3UL << 5) Clear to 00
//   0 1 Current      |=  (1UL << 5)
//   1 0 RMS          |=  (2UL << 5)
//   1 1 Resistance   |=  (3UL << 5)
void stageBeta(int mode){
	if(!betaInit){
				mode_switch_init();
				betaInit = true;
			}

	//Clear bits 
	GPIOE->ODR &= ~(3UL << 5);
	switch(mode){
		case 0://Voltage
			GPIOE->ODR &= ~(3UL << 5);
			break;
		case 1://Current
			GPIOE->ODR |= (1UL << 5);
			break;
		case 2://RMS
			GPIOE->ODR |= (2UL << 5);
			break;
		case 3://Resistance
			GPIOE->ODR |= (3UL << 5);
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
		//TODO: This is ugly as fuck. Should I dequeue Serial commands here?
		menu();
  }
}

void menu(){
	//Bluetooth Override of Buttons
	char *bluetoothSwitchPacket = DequeueString(bluetoothQueue);
	
	#ifdef DMM_DEBUG
		printf("[Android Client] Data Received: %s\r\n", bluetoothSwitchPacket);
	#endif	
	
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
	if(strcmp(bluetoothSwitchPacket,"<m:1>") == 0) bluetoothMenuPosition = 1;
	if(strcmp(bluetoothSwitchPacket,"<m:2>") == 0) bluetoothMenuPosition = 2;
	if(strcmp(bluetoothSwitchPacket,"<m:3>") == 0) bluetoothMenuPosition = 3;
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
	
	//Clear display in advance of menu update
	lcd_clear_display();
	
	
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
			stopGenerating(); //Stop DMA operation
		}
		//Local menu position has changed value, update menu Position so STM board commands menu
		menuPosition = localMenuPosition;
		//Update previous menu position
		prevLocalMenuPosition = localMenuPosition;
	}
	
	double current =0;
	double resistance = 0;
	
	//If Menu goes outside implemented features, these strings will propagate to LCD
	char lcd_line1[16] ="ERROR";
	char lcd_line2[16] ="ERROR";
	
	switch(menuPosition){
		case 0: //If menu position not initialised, go to voltage (No break statement)
		case 1: //voltage
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
			sendPacket(1, ADC1_valueScaled, ADC1_currentRange);
			break;
			
		case 2://current
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
			sendPacket(2, current, ADC1_currentRange);
			break;
			
		case 3://resistance
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
			sendPacket(3, resistance, ADC1_currentRange);
			break;
		case 4://Frequency Response
			frequencyResponseMenu(sweepStart, sweepEnd, sweepResolution);
			break;
		case 5://Signal Generation
			signalGenerationMenu(genFrequency,  genAmplitude, sigGenType);
			break;
		case 7://Measure Capacitance
			sprintf(lcd_line1,"Capacitance");
			sprintf(lcd_line2,"%d", numHighTicks);
			if(timerDone) measureCapacitance();
			break;
		case 8:
			break;
		default:
			break;
	}
	//Update LCD Display with menu text
	lcd_write_string(lcd_line1, 0, 0);
	lcd_write_string(lcd_line2, 1, 0);
}


void signalGenerationMenu(uint32_t genFrequency, float genAmplitude, uint8_t sigGenType){
	//Android hasnt set any data, so use board buttons to manually set sweep parameters
	if((genFrequency == 0)&&(compare_float(genAmplitude, 0.0f))&&(sigGenType == 0)){
		//TODO: Manual UI for Setting parameters
		//TODO: Light LED's appropriately to show available selection
		//Switch between generating Sine, Square, SAW. Set amplitude
		generateSignal(SINE_TYPE, 1.0);
	} else {//Start Signal generation with Android sent data
			//TODO: While loop to wait for menuPosition to change to indicate user input
		
			//Switch between generating Sine, Square, SAW. Set amplitude
			genAmplitude = 1.0;
			generateSignal(SQUARE_TYPE, genAmplitude);
	}
}

void frequencyResponseMenu(int sweepStart, int sweepEnd, int sweepResolution){
	//Android hasnt set any data, so use board buttons to manually set sweep parameters
	if((sweepStart == 0)&&(sweepEnd == 0)&&(sweepResolution == 0)){
		//TODO: Manual UI for Setting parameters
		//TODO: Light LED's appropriately to show available selection
		//TODO: Make the screen display a pretty frequency response
		//TODO: Alter Frequency Response to return a double array of results if called from menu
			frequencyResponse(10, 20, 1);
	} else {//Start frequency response with Android sent data
		frequencyResponse(sweepStart, sweepEnd, sweepResolution);
	}
}
