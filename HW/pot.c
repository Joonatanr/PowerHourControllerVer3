/*
 * pot.c
 *
 *  Created on: 16. dets 2019
 *      Author: JRE
 */

#include "pot.h"
#include "driverlib.h"

/* We control the potentiometer and indicator leds in this module. */



/* POT is connected to output 4.1 or A12 */

/* POT_LED1 -> 4.6
 * POT_LED2 -> 3.3
 * POT_LED3 -> 3.2
 * POT_LED4 -> 6.0
 *
 */

static volatile uint16_t curADCResult = 0u;
//static volatile float normalizedADCRes;

Private void setPotLed(int id, Boolean state);
Private int currentRange = 0;


Public void pot_init(void)
{
    /* Enable the LED pins as general purpose inouts. */
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN3);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN2);
    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0);

    /* Enable the FPU for floating point operation. */
    FPU_enableModule();
    FPU_enableLazyStacking();

    /* Initializing ADC (MCLK/1/4) */
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4, 0);

    /* Configuring GPIOs (4.1 A12) */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN1,
    GPIO_TERTIARY_MODULE_FUNCTION); /* NOTE!, IF DOESNT WORK, THEN TRY OTHER PRIMARY AND SECONDARY. */

    /* Configuring ADC Memory */
    MAP_ADC14_configureSingleSampleMode(ADC_MEM12, true);
    MAP_ADC14_configureConversionMemory(ADC_MEM12, ADC_VREFPOS_AVCC_VREFNEG_VSS,
    ADC_INPUT_A12, false);

    /* Configuring Sample Timer */
    MAP_ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    /* Enabling/Toggling Conversion */
    MAP_ADC14_enableConversion();
    //MAP_ADC14_toggleConversionTrigger();

    /* Enabling interrupts */
    MAP_ADC14_enableInterrupt(ADC_INT12);
    MAP_Interrupt_enableInterrupt(INT_ADC14);
    MAP_Interrupt_enableMaster();
}


Public void pot_cyclic_10ms(void)
{
    /* TODO : Add hysteresis. */
    float adc_value = curADCResult;

    /* ADC value is 14 bit, so between 0 and 16384 */ /* 4096 per quadrant, lets first try with naive solution. */
    if (adc_value > 12288u)
    {
        setPotLed(0, TRUE);
        setPotLed(1, FALSE);
        setPotLed(2, FALSE);
        setPotLed(3, FALSE);
        currentRange = 0;
    }
    else if(adc_value > 8192u)
    {
        setPotLed(0, TRUE);
        setPotLed(1, TRUE);
        setPotLed(2, FALSE);
        setPotLed(3, FALSE);
        currentRange = 1;
    }
    else if(adc_value > 4096u)
    {
        setPotLed(0, TRUE);
        setPotLed(1, TRUE);
        setPotLed(2, TRUE);
        setPotLed(3, FALSE);
        currentRange = 2;
    }
    else
    {
        setPotLed(0, TRUE);
        setPotLed(1, TRUE);
        setPotLed(2, TRUE);
        setPotLed(3, TRUE);
        currentRange = 3;
    }

    MAP_ADC14_toggleConversionTrigger();
}


Public int pot_getSelectedRange()
{
    return currentRange;
}


/* ADC Interrupt Handler. This handler is called whenever there is a conversion
 * that is finished for ADC_MEM0.
 */
void ADC14_IRQHandler(void)
{
    uint64_t status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if (ADC_INT12 & status)
    {
        curADCResult = MAP_ADC14_getResult(ADC_MEM12);
        //normalizedADCRes = (curADCResult * 3.3) / 16384;

        //ADC14_toggleConversionTrigger();
    }
}


Private void setPotLed(int id, Boolean state)
{
    uint32_t port;
    uint32_t pins;


    switch(id)
    {
        case 2u:
            port = GPIO_PORT_P4;
            pins = GPIO_PIN6;
            break;
        case 1u:
            port = GPIO_PORT_P3;
            pins = GPIO_PIN3;
            break;
        case 3u:
            port = GPIO_PORT_P3;
            pins = GPIO_PIN2;
            break;
        case 0u:
            port = GPIO_PORT_P6;
            pins = GPIO_PIN0;
            break;
        default:
         /* Should not happen. */
         return;
    }

    if (state)
    {
        GPIO_setOutputHighOnPin(port, pins);
    }
    else
    {
        GPIO_setOutputLowOnPin(port, pins);
    }
}




