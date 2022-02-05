/** MIT License
 * Copyright (c) 2021, Z0 Devel
 * All rights reserved.
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 **/


//******************************************************************************
//! MSP430F5510 example on sampling the internal MSP430 temperature at ADC channel 10
//! Based on TI ADC examples
//!
//!
//! A single sample is made on A10 (internal Temperature sensor) with reference to internal 1.5V
//! Vref. Software sets ADC10SC to start sample and conversion - ADC10SC
//! automatically cleared at EOC. ADC10_A internal oscillator times sample (16x)
//! and conversion. ADC10BUSY flag is polled for EOC
//! ACLK = n/a, MCLK = SMCLK = default DCO ~1.05 MHz, ADC10CLK = ADC10OSC
//!
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - ADC10_A peripheral
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - None.
//!
//******************************************************************************
#include "driverlib.h"

/**
 * Calibrated temperature data per MSP unit at 30 C and 85 C.
 * See device datasheet for TLV table memory mapping
 */
#define CALADC10_15V_30C  *((unsigned int *)0x1A1A)
#define CALADC10_15V_85C  *((unsigned int *)0x1A1C)

/**
 * volatile variables, so they can be easily debugged
 */
volatile float temperatureDegC,temperatureDegF;
volatile uint16_t measure;

/**
 * Views of the registers of interest in this example, just used to see how are they configured through driverlib
 */
volatile uint16_t reg_adc10ctl0;
volatile uint16_t reg_adc10ctl1;
volatile uint16_t reg_adc10ctl2;
volatile uint16_t reg_adc10mctl0;
volatile uint16_t reg_adc10ie;
volatile uint16_t reg_refctl0;

void main (void)
{
    /* Stop Watchdog Timer */
    WDT_A_hold(WDT_A_BASE);

    /* Initialize the ADC10_A Module */
    /*
     * Base Address for the ADC10_A Module
     * Use internal ADC10_A bit as sample/hold signal to start conversion
     * USE MODOSC 5MHZ Digital Oscillator as clock source
     * Use default clock divider of 1
     */
    ADC10_A_init(ADC10_A_BASE,
        ADC10_A_SAMPLEHOLDSOURCE_SC,
        ADC10_A_CLOCKSOURCE_ADC10OSC,
        ADC10_A_CLOCKDIVIDER_1);

    ADC10_A_enable(ADC10_A_BASE);

    /*
     * Base Address for the ADC10_A Module
     * Sample/hold for 16 clock cycles
     * Do not enable Multiple Sampling
     */
    ADC10_A_setupSamplingTimer(ADC10_A_BASE,
        ADC10_A_CYCLEHOLD_16_CYCLES,
        ADC10_A_MULTIPLESAMPLESDISABLE);

    /* Configure Memory Buffer */
    /*
     * Base Address for the ADC10_A Module
     * Use (Vcc-Vss)/2 as input
     * Use positive reference of Internally generated Vref
     * Use negative reference of AVss
     */
    ADC10_A_configureMemory(ADC10_A_BASE,
        ADC10INCH_10,
        ADC10_A_VREFPOS_INT,
        ADC10_A_VREFNEG_AVSS);

    /* Configure internal reference */

    /* If ref generator busy, WAIT */
    while ( REF_BUSY == Ref_isRefGenBusy(REF_BASE) ) ;

    /* Select internal ref = 1.5V */
    Ref_setReferenceVoltage(REF_BASE,
        REF_VREF1_5V);

    /* Internal Reference ON */
    Ref_enableReferenceVoltage(REF_BASE);

    /* Delay (~75us) for Ref to settle */
    __delay_cycles(75);

    /* 0000 0010 0001 0000b */
    reg_adc10ctl0 = ADC10CTL0;

    /* 0000 0010 0000 0000b */
    reg_adc10ctl1 = ADC10CTL1;

    /* 0000 0000 0001 0000b */
    reg_adc10ctl2 = ADC10CTL2;

    /* 0000 0000 0001 1010b */
    reg_adc10mctl0 = ADC10MCTL0;
    
    /* 0000 0000 0000 0000b */
    reg_refctl0 = ADC10IE;

    /* 0000 0001 1000 0001b */
    reg_refctl0 = REFCTL0;


    while (1)
    {
        /*
         * Enable and Start the conversion
         * Single-Channel, Single-Conversion Mode
         */
        ADC10_A_startConversion(ADC10_A_BASE,
            ADC10_A_SINGLECHANNEL);

        /* Wait until EOC */
        while (ADC10_A_isBusy(ADC10_A_BASE)) ;

        /* Debug only */
        __no_operation();


        measure = ADC10_A_getResults(ADC10_A_BASE);

        temperatureDegC = (float) (((long) measure - CALADC10_15V_30C) * (85 - 30))
                       / (CALADC10_15V_85C - CALADC10_15V_30C) + 30.0f;

        temperatureDegF = temperatureDegC * 9.0f / 5.0f + 32.0f;

        /* Debug only */
        __no_operation();

    }
}

