#include "dac.h"

/* ----------------------- WAVE DATA ------------------------- */
uint16_t sine[WAVE_RES] = {2048, 2145, 2242, 2339, 2435, 2530, 2624, 2717, 2808, 2897,
                           2984, 3069, 3151, 3230, 3307, 3381, 3451, 3518, 3581, 3640,
                           3696, 3748, 3795, 3838, 3877, 3911, 3941, 3966, 3986, 4002,
                           4013, 4019, 4020, 4016, 4008, 3995, 3977, 3954, 3926, 3894,
                           3858, 3817, 3772, 3722, 3669, 3611, 3550, 3485, 3416, 3344,
                           3269, 3191, 3110, 3027, 2941, 2853, 2763, 2671, 2578, 2483,
                           2387, 2291, 2194, 2096, 1999, 1901, 1804, 1708, 1612, 1517,
                           1424, 1332, 1242, 1154, 1068, 985, 904, 826, 751, 679,
                           610, 545, 484, 426, 373, 323, 278, 237, 201, 169,
                           141, 118, 100, 87, 79, 75, 76, 82, 93, 109,
                           129, 154, 184, 218, 257, 300, 347, 399, 455, 514,
                           577, 644, 714, 788, 865, 944, 1026, 1111, 1198, 1287,
                           1378, 1471, 1565, 1660, 1756, 1853, 1950, 2047};

uint16_t square[WAVE_RES] = {4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020,
                             4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020,
                             4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020,
                             4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020,
                             4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020, 4020,
                             75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
                             75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
                             75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75};

uint16_t saw[WAVE_RES] = {2048, 2112, 2176, 2240, 2305, 2369, 2433, 2497, 2561, 2625, 2689, 2753, 2818, 2882,
                          2946, 3010, 3074, 3138, 3202, 3267, 3331, 3395, 3459, 3523, 3587, 3651, 3716, 3780,
                          3844, 3908, 3972, 4036, 4100, 4020, 3956, 3892, 3828, 3763, 3699, 3635, 3571, 3507,
                          3443, 3379, 3315, 3250, 3186, 3122, 3058, 2994, 2930, 2866, 2801, 2737, 2673, 2609,
                          2545, 2481, 2417, 2352, 2288, 2224, 2160, 2096, 2032, 1968, 1904, 1839, 1775, 1711,
                          1647, 1583, 1519, 1455, 1390, 1326, 1262, 1198, 1134, 1070, 1006, 941, 877, 813, 749,
                          685, 621, 557, 493, 428, 364, 300, 236, 172, 108, 44, 0, 64, 128, 192, 257, 321, 385, 449,
                          513, 577, 641, 705, 770, 834, 898, 962, 1026, 1090, 1154, 1219, 1283, 1347, 1411, 1475,
                          1539, 1603, 1668, 1732, 1796, 1860, 1924, 1988};


char *waveTypes[] = {"SINE", "SQUARE", "SAW", "NOISE"};

/*--------- Internal Functions (Prototypes) ----------*/
static void TIM5_Config(void);

static void DAC1_Config(const uint16_t *waveData);

void dac_initialise(const uint16_t *waveData);

//Array to store modified signal for generation
uint16_t *generatedSignal;

//Default values for initialising timer
uint32_t OUT_FREQ = 5000; // Output waveform frequency
uint16_t TIM_PERIOD = 0;    // Autoreload reg value
TIM_TimeBaseInitTypeDef TIM5_TimeBase;

void dac_initialise(const uint16_t *waveData) {
    GPIO_InitTypeDef gpio_A;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    gpio_A.GPIO_Pin = GPIO_Pin_4;
    gpio_A.GPIO_Mode = GPIO_Mode_AN;
    gpio_A.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gpio_A);

    TIM5_Config();
    DAC1_Config(waveData);
}

static void TIM5_Config(void) {
    TIM_PERIOD = ((CNT_FREQ) / ((WAVE_RES) * (OUT_FREQ)));
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    TIM_TimeBaseStructInit(&TIM5_TimeBase);
    TIM5_TimeBase.TIM_Period = (uint16_t) TIM_PERIOD;
    TIM5_TimeBase.TIM_Prescaler = 0;
    TIM5_TimeBase.TIM_ClockDivision = 0;
    TIM5_TimeBase.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM5_TimeBase);
    TIM_SelectOutputTrigger(TIM5, TIM_TRGOSource_Update);

    TIM_Cmd(TIM5, ENABLE);
}


static void DAC1_Config(const uint16_t *waveData) {
    DAC_InitTypeDef DAC_INIT;
    DMA_InitTypeDef DMA_INIT;

    DAC_INIT.DAC_Trigger = DAC_Trigger_T5_TRGO;
    DAC_INIT.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_INIT.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(DAC_Channel_1, &DAC_INIT);

    DMA_DeInit(DMA1_Stream5);
    DMA_INIT.DMA_Channel = DMA_Channel_7;
    DMA_INIT.DMA_PeripheralBaseAddr = (uint32_t) DAC_DHR12R1_ADDR;
    DMA_INIT.DMA_Memory0BaseAddr = (uint32_t) waveData;
    DMA_INIT.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_INIT.DMA_BufferSize = WAVE_RES;
    DMA_INIT.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_INIT.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_INIT.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_INIT.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_INIT.DMA_Mode = DMA_Mode_Circular;
    DMA_INIT.DMA_Priority = DMA_Priority_High;
    DMA_INIT.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_INIT.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_INIT.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_INIT.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA1_Stream5, &DMA_INIT);

    DMA_Cmd(DMA1_Stream5, ENABLE);
    DAC_Cmd(DAC_Channel_1, ENABLE);
    DAC_DMACmd(DAC_Channel_1, ENABLE);
}

void generateSignal(uint32_t genFrequency, uint8_t signalType, float amplitude) {
    lcd_write_string("Signal Gen", 0, 0);
    lcd_write_string("on pin A4", 1, 0);

    static uint32_t prevGenFrequency;
    static uint8_t prevSignalType;
    static float prevAmplitude;

#ifdef DAC_DEBUG
    printf("[Hardware Subsystem] Signal generation beginning with sigtype: %d Amplitude: %f Frequency: %d\r\n",
           signalType, amplitude, genFrequency);
#endif

    //Update timer with new period
    OUT_FREQ = genFrequency;
    TIM5_Config();


    //Work out whether there is a need to adjust the current signal, if not, return. If there is, free old pointer.
    if (compare_float(prevAmplitude, amplitude) && (prevGenFrequency == genFrequency) &&
        (prevSignalType == signalType)) {
        return;
    } else {
        free(generatedSignal);
    }

    //No error checking in amplitude magnitude as done on Android side
    generatedSignal = calloc(WAVE_RES, sizeof(uint16_t));

    //Copy array into target efficiently using memcpy
    switch (signalType) {
        case SINE_TYPE:
            memcpy(generatedSignal, sine, WAVE_RES * sizeof(uint16_t));
            break;

        case SQUARE_TYPE:
            memcpy(generatedSignal, square, WAVE_RES * sizeof(uint16_t));
            break;

        case SAW_TYPE:
            memcpy(generatedSignal, saw, WAVE_RES * sizeof(uint16_t));
            break;

        case NOISE_TYPE:
            //Calculate noise in amplitude loop
            break;
    }

    amplitude /= MANUAL_MAX_AMP; //Normalise target voltage so is scalar between 0 and 1 (avoid clipping dac output)

    if (signalType == NOISE_TYPE) {
        //Generate wave with adjusted amplitude and noise
        for (int i = 0; i < WAVE_RES; i++) {
            generatedSignal[i] = (amplitude) * ((int) rand_between(43, 4020));
        }
    } else {
        //Generate wave with adjusted amplitude
        for (int i = 0; i < WAVE_RES; i++) {
            generatedSignal[i] *= (amplitude);
        }
    }

    dac_initialise(generatedSignal);
    prevAmplitude = amplitude;
    prevGenFrequency = genFrequency;
    prevSignalType = signalType;
}

//Clean up after ourselves following Signal Generation
void stopGenerating() {
    //Disable DMA
    DMA_Cmd(DMA1_Stream5, DISABLE);
    DAC_Cmd(DAC_Channel_1, DISABLE);
    DAC_DMACmd(DAC_Channel_1, DISABLE);
    //Disable Timer
    TIM_Cmd(TIM5, DISABLE);
}


double peakToPeak() {
    double minVal = 0;
    double maxVal = 0;

    //Sample enough of the input to ensure we have come across max and minimum
    for (int i = 0; i < NUM_PK_PK_SAMPLES; i++) {
        double readVal = read_ADC1_NOAVERAGE();
        if (readVal > maxVal) maxVal = readVal;
        if (readVal < minVal) minVal = readVal;
    }

#ifdef DAC_DEBUG
    printf("[Frequency Response] Maximum Value: %lf Minimum Value: %lf\r\n", minVal, maxVal);
#endif

    return maxVal - minVal;
}


void frequencyResponse(uint32_t sweepStart, uint32_t sweepEnd, uint32_t sweepResolution) {
    lcd_clear_display();
    lcd_write_string("Sweeping..", 0, 0);
    lcd_write_string("Please wait", 1, 0);

    //Check DAC initialised before beginning frequency response
    dac_initialise(sine);

#ifdef DAC_DEBUG
    printf("[Hardware Subsystem] Frequency Response beginning with sweep start: %d sweep end: %d sweep resolution: %d\r\n",
           sweepStart, sweepEnd, sweepResolution);
#endif

    sendPacket(FREQUENCY_RESP_STATE, 0, 0, 0); //Send start packet (freq data = 0)

    //Move from sweepstart to sweep end via stepnumber
    for (int outFrequency = sweepStart; outFrequency <= sweepEnd; outFrequency += sweepResolution) {

        OUT_FREQ = outFrequency;
        TIM5_Config();

        double freqResponseRatio = peakToPeak() / ((double) WAVE_GEN_VOLTAGE);

        //Send output packet
        sendPacket(FREQUENCY_RESP_STATE, freqResponseRatio, OUT_FREQ, 0);

#ifdef DAC_DEBUG
        printf("[Frequency Response] Gain at frequency %d is %lf\r\n", OUT_FREQ,
               freqResponseRatio); //Corrupts DAC signal
#endif
    }

    stopGenerating(); //Stop generating signal and free STM32 resources
}



