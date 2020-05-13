/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system intialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.95-b-SNAPSHOT
        Device            :  PIC24FJ128GB204
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.36
        MPLAB 	          :  MPLAB X v5.10
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/usb/usb.h"
#include "mcc_generated_files/adc1.h"
#define    FCY    32000000UL    // Instruction cycle frequency, Hz - required for __delayXXX() to work
#include <libpic30.h>        // __delayXXX() functions macros defined here
#include <stdint.h>
#include <stdio.h>
/*
                         Main application
 */
static uint8_t readBuffer[64];
static uint8_t writeBuffer[64];
uint16_t duty_cycle = 0;
uint16_t readValue;

void My_ISR(void)
{
    //LED_Toggle();
    ADC1_Stop();                    // trigger next conversion
}

int main(void)
{
    // initialize the device
    SYSTEM_Initialize();

  // ADC1_ChannelSelect(ADC1_CHANNEL_AN4);
    ADC1_Stop();                  // begin first conversion
    //TMR1_SetInterruptHandler(My_ISR);
    TMR1_Start();
    
    while (1)
    {
        if( USBGetDeviceState() < CONFIGURED_STATE )
        {
            continue;
        }

        if( USBIsDeviceSuspended()== true )
        {
            continue;
        }

        if( USBUSARTIsTxTrfReady() == true)
        {
            ADC1_Initialize();
            ADC1_ChannelSelect(ADC1_CHANNEL_CTMU);
            ADC1_Start();
            __delay_ms(10);
            ADC1_Stop();
            while(!ADC1_IsConversionComplete())
            {
                ADC1_Tasks();   
            }
            readValue = ADC1_ConversionResultGet();
            sprintf(writeBuffer, "duty_cycle: %d, adc: %d\n\r", duty_cycle, readValue);
            putsUSBUSART(writeBuffer);
            
            
            duty_cycle += 50;
            if(duty_cycle > 640)
                duty_cycle = 0;

            OC1_PrimaryValueSet(duty_cycle);
            __delay_ms(100);
        }
            
        CDCTxService();
        
       
    }

    return 1;
}
/**
 End of File
*/

