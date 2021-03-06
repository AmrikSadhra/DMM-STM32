#include "adc.h"


uint16_t calibrationValue = 0;
bool isInitialised = false;
uint8_t ADC1_currentRange = 0;
bool rangeChange = false;

/*--------- Internal Functions (Prototypes) ----------*/
uint16_t calibrate_ADC1(void);

void ADC1_init(void);

void autorange_init() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); //Enable GPIOE for voltage gain mux selection
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); //Enable GPIOB for resistance gain enable

    GPIO_InitTypeDef GPIO_InitStruct;

    //Enable Voltage Gain mux pins
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    //Enable Resistance Gain pins
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}


void ADC1_init(void) {
    //Fix for infinite call in calibration routine
    if (!isInitialised) isInitialised = true;
    else return;

    //Set registers to bring up ADC1 in correct mode
    RCC->APB2ENR |= ((1UL << 8));         /* Enable ADC1 clock  */
    RCC->AHB1ENR |= ((1UL << 2));         /* Enable GPIOC clock */
    GPIOC->MODER = 0xffffffff;
    GPIOC->PUPDR = 0;
    ADC1->CR1 = 0x00;
    ADC1->CR1 |= (1UL << 11);
    ADC1->CR2 = 0x00;
    ADC1->CR2 |= (1UL << 10);                            /* right alignment of 12 bits */
    ADC->CCR = 0x00;
    ADC1->SQR1 = 0x01;                                            /* 1 conversion at a time */
    ADC1->SMPR1 = 0x00;
    ADC1->SMPR1 = 0x0300;
    ADC1->SQR1 = 0x01;
    ADC1->SQR3 = 0x0e;                                            /* ADC_IN14 = 0x0e: ADC_IN15 = 0x0f */
    ADC1->CR2 |= (1UL << 0);

    autorange_init();

    calibrationValue = calibrate_ADC1();
}

//Function to read ADC and return raw value
uint16_t read_ADC1_raw(void) {
    //Initialise ADC1 inline if not up
    if (!isInitialised) ADC1_init();

    //Begin ADC1 read
    ADC1->CR2 |= (1UL << 30);        /* set SWSTART to 1 to start conversion */
    Delay(10);

    //Return raw 32bit uint (MAGIC)
    return ((ADC1->DR << 4) & 0xFF00);
}

uint16_t calibrate_ADC1() {
#ifdef ADC_CALIBRATION_ENABLED
    //Connect read stage to Ground to calibrate ADC
    dmmModeSelect(CALIBRATE_ADC_STATE);

    uint32_t runningTotal = 0;

    for(int i = 0; i < NUM_CAL_SAMPLES; i++){
        runningTotal = read_ADC1_raw();
    }

    //Connect read stage to voltage input, calibration over
    dmmModeSelect(VOLTAGE_READ_STATE);
return (uint16_t) runningTotal/5;
#else
    return 0;
#endif
}


//Read ADC numSamples times to produce average
double runningAverage(uint8_t numSamples) {
    uint32_t adc_raw = 0;

    for (int i = 0; i < numSamples; i++) {
        adc_raw += read_ADC1_raw();
    }

    return (double) (adc_raw / numSamples) - calibrationValue;
}


/* Function to Automatically set the range dependent upon ADC1 value */
void autoRange(uint16_t adc1_raw,
               bool resistanceMode) { //Resistance mode drops Voltage gain to minimum, used Resistance mux to assert higher gain
    bool rangeSwitch = false;
    static uint8_t ADC1_prevRange;

    double ADC1_rangeSwitch = map(adc1_raw, 0, ADC_VALUE_3V, 0, 3);
    bool lowerThreshold = ((ADC1_rangeSwitch < MODE_SWITCHDOWN_HIGHER) && (ADC1_rangeSwitch > MODE_SWITCHDOWN_LOWER));
    bool higherThreshold = ((ADC1_rangeSwitch > MODE_SWITCHUP_HIGHER) || (ADC1_rangeSwitch < MODE_SWITCHUP_LOWER));

    static bool extraRange;

    if (!resistanceMode) {
        extraRange = false;
        //If RAW value greater than max value it can return - switch tolerance, probably time to change range (unless we maxed on range)
        if (higherThreshold && (ADC1_currentRange != 0)) {
            //High threshold
            ADC1_currentRange--;
            rangeSwitch = true;
        } else if (lowerThreshold && (ADC1_currentRange != 2)) {//If we're close (and not at lowest range)
            //Low threshold
            ADC1_currentRange++;
            rangeSwitch = true;
        }
    } else {//Measuring resistance
        if (!extraRange) { //If no extra range, check if its needed
            if (lowerThreshold && (ADC1_currentRange == 2)) {//If we're close (and not at lowest range)
                //Low threshold
                ADC1_currentRange = 0; //Drop to lowest range to get Max gain from voltage side
                extraRange = true;     //Enable extra gain for resistance measurement
                rangeSwitch = true;
            } else if (lowerThreshold && (ADC1_currentRange != 2)) {
                ADC1_currentRange++;
                rangeSwitch = true;
            }

            //If extra range not needed, proceed as normal with range switching
            if (higherThreshold && (ADC1_currentRange !=
                                    0)) {//If not in min gain, and at ADC threshold max of 10v, drop to lower range for less gain
                //High threshold
                ADC1_currentRange--;
                rangeSwitch = true;
            } else if (higherThreshold && (ADC1_currentRange == 0)) {
                //Do nothing
            }

        } else { //If extra range enabled
            if (lowerThreshold &&
                ADC1_currentRange != 2) {//If not at max gain, and ADC value low, increase gain by moving up a range
                ADC1_currentRange++;
                rangeSwitch = true;
            } else if (lowerThreshold && ADC1_currentRange == 2) {
                //Do nothing
            }

            if (higherThreshold && (ADC1_currentRange != 0)) {
                ADC1_currentRange--;
                rangeSwitch = true;
            } else if (higherThreshold && (ADC1_currentRange == 0)) {
                extraRange = false;
                ADC1_currentRange = 2;
                rangeSwitch = true;
            }
        }
    }

    if (rangeSwitch) {
#ifdef ADC_DEBUG
        printf("[Hardware Subsystem] ADC1 Range switch requested. Switching, raw ADC read: %d. Old Range: %d New Range: %d\r\n",
               adc1_raw, ADC1_prevRange, ADC1_currentRange);
#endif

        //Clear Range switch bits
        GPIOE->ODR &= ~(3UL << 3);


        switch (ADC1_currentRange) {
            case 0://10v in
                //If in extra range mode, give the Circuit more gain through resistance amplification circuit
                if (extraRange) {
                    GPIOB->ODR |= (1 << 8);
                } else {
                    GPIOB->ODR &= ~(1 << 8);
                }
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
            case 3://0.01
                //Range 3
                GPIOE->ODR |= (3UL << 3);
                break;
        }
        ADC1_prevRange = ADC1_currentRange;
    }
}

//Function to read ADC and return scaled value
double read_ADC1(bool resistanceMode) {
    if (!isInitialised) ADC1_init();

    //Average data coming in (smooth)
    double ADC1_valueAveraged = runningAverage(NUM_AVG_SAMPLES);
    double ADC1_valueScaled = 0.0f;

    //Adjust range
    autoRange((uint16_t) ADC1_valueAveraged, resistanceMode);

    switch (ADC1_currentRange) {
        case 0:
            ADC1_valueScaled = map(ADC1_valueAveraged, 0, ADC_VALUE_3V, -10, 10);
            break;
        case 1:
            ADC1_valueScaled = map(ADC1_valueAveraged, 0, ADC_VALUE_3V, -1, 1);
            break;
        case 2:
            ADC1_valueScaled = map(ADC1_valueAveraged, 0, ADC_VALUE_3V, -0.1, 0.1);
            break;
        case 3:
            ADC1_valueScaled = map(ADC1_valueAveraged, 0, ADC_VALUE_3V, -0.01, 0.01);
            break;
    }

    double offset = 0.0039 * ADC1_valueScaled - 0.0225;

#ifdef ADC_DEBUG
    printf("[Hardware Subsystem] ADC_1 Scaled Voltage %lf, Calibrated : %lf\r\n", ADC1_valueScaled, offset);
#endif

    return ADC1_valueScaled + offset;
}

//Function to read ADC1 as quickly as possible
double read_ADC1_NOAVERAGE(void) {
    //Average data coming in (smooth)
    double ADC1_value = read_ADC1_raw() - calibrationValue;
    double ADC1_valueScaled = 0.0f;

    //Adjust range
    autoRange((uint16_t) ADC1_value, false);

    switch (ADC1_currentRange) {
        case 0:
            ADC1_valueScaled = map(ADC1_value, 0, ADC_VALUE_3V, -10, 10);
            break;
        case 1:
            ADC1_valueScaled = map(ADC1_value, 0, ADC_VALUE_3V, -1, 1);
            break;
        case 2:
            ADC1_valueScaled = map(ADC1_value, 0, ADC_VALUE_3V, -0.1, 0.1);
            break;
        case 3:
            ADC1_valueScaled = map(ADC1_value, 0, ADC_VALUE_3V, -0.01, 0.01);
            break;
    }

    return ADC1_valueScaled;
}
