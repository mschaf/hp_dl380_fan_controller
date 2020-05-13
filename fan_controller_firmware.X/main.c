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
#include "mcc_generated_files/mcc.h"
#define    FCY    32000000UL  
#include <libpic30.h> 
#include "stdio.h"
#include "string.h"
#include <stdarg.h>

/*
                         Main application
 */

void TMR1_Interrupt(void)
{
    LED_Toggle();
}

/* 
 * Inputs:
 *     | PIN | Analog PIN
 * IN1 | C0  | AN10
 * IN2 | C1  | AN11
 * IN3 | C2  | AN12
 * IN4 | B15 | AN9
 * IN5 | B14 | AN6
 * IN6 | B13 | AN7
 * 
 * Outputs:
 *      | PIN
 * OUT1 | C3  
 * OUT2 | C4
 * OUT3 | C5
 * OUT4 | A1
 * OUT5 | B2
 * OUT6 | B3
 * 
 * Leds:
 *        | Green Pin | Red Pin
 * Power  | C9        | C8
 * Fan    | C7        | C6
 * Serial | B9        | B8
 *  
 */

void set_output(int output, int value);
int read_input(int input);
void serial_printf(const char *, ...);
void serial_console_task();
void auto_output_task();
void read_inputs();
void write_config();
void power_led_task();

uint8_t read_byte;
uint8_t bytes_in_command_buffer = 0;
static char command_buffer[64];
static char write_buffer[128];

int minimal_set_value = 10;
float reduction_factor = 0.5;
int cutoff_value = 65;

int input_values[6];
int output_values[6];

int new_tx_ready = 1;

int auto_mode = 1;
int i;

int led_cycle_count = 0;

int main(void)
{
    // Setup MCC
    SYSTEM_Initialize(); 
    TMR1_Start();

    // Setup ADC
    
    AD1CON1 = 0;
    AD1CON2 = 0;
    AD1CSSL = 0;
    AD1CON3 = 0x0002;

    AD1CON1bits.ADON = 1; // turn ADC ON
    
    // Setup ananlog inputs
    
    TRISCbits.TRISC0 = 1; // IN1
    ANSCbits.ANSC0 = 1;    
    TRISCbits.TRISC1 = 1; // IN2
    ANSCbits.ANSC1 = 1;    
    TRISCbits.TRISC2 = 1; // IN3
    ANSCbits.ANSC2 = 1;    
    TRISBbits.TRISB15 = 1; // IN4
    ANSBbits.ANSB15 = 1;
    TRISBbits.TRISB14 = 1; // IN5
    ANSBbits.ANSB14 = 1;
    TRISBbits.TRISB13 = 1; // IN6
    ANSBbits.ANSB13 = 1;
    
    while (1) // repeat continuously
    {
        read_inputs();
        auto_output_task();
        serial_console_task();
        led_task();
        
        ClrWdt();
    }
    return 1;
}

void led_task(){
    led_cycle_count ++;
    if(led_cycle_count > 30){
        led_cycle_count = 0;
         led_power_green_Toggle();
    }
    if(auto_mode){
        led_fan_green_SetHigh();
        led_fan_red_SetLow();
    }else{
        led_fan_green_SetLow();
        led_fan_red_SetHigh();
    }
}

void read_inputs(){
    for(i=0;i<6;i++){
        input_values[i] = read_input(i);
    }
}

void auto_output_task(){
    if(auto_mode){
        for(i=0;i<6;i++){
            if(input_values[i] < cutoff_value){
                output_values[i] = (int)(((float) input_values[i]) * reduction_factor);
                if(output_values[i] < minimal_set_value){
                    output_values[i] = minimal_set_value;
                }
            }else{
                output_values[i] = input_values[i];
            }
            set_output(i, output_values[i]);
        }
    }
}

void serial_console_task() {
    led_serial_green_SetLow();
    led_serial_red_SetLow();
    if( USBGetDeviceState() < CONFIGURED_STATE ){
        new_tx_ready = 1;
        return;
    }
        
    if(USBIsDeviceSuspended()){
        new_tx_ready = 1;
        return;
    }
    
    led_serial_red_SetHigh();

    if( USBUSARTIsTxTrfReady()){
        led_serial_red_SetLow();
        led_serial_green_SetHigh();
        
        
        // read byte wise
        int read_bytes = getsUSBUSART(&read_byte, 1);

        if(read_bytes > 0){
            if(read_byte == '\r'){
                
                if(new_tx_ready){
                    new_tx_ready = 0;
                    serial_printf("\r\nHP DL380e G8 Fan Controller\n\r");
                    serial_printf("Commands: get, set, auto, config get, config set\n\n\r> ");
                }

                command_buffer[bytes_in_command_buffer] = 0;
                // serial_printf("\n\rcommand: %s \n\r",command_buffer);
                
                if(bytes_in_command_buffer == 0){
                    serial_printf("\r\n");
                }else if(strcmp(command_buffer, "get") == 0){
                    serial_printf("\r\nauto mode: %d\r\nchannel | reading | output\n\r---------+---------+-------\n\r", auto_mode);
                    for(i=0;i<6;i++){
                        serial_printf("    %d    |  %3d%%   |  %3d%%   \r\n", i+1, input_values[i], output_values[i]);
                    }
                }else if(strcmp(command_buffer, "auto") == 0){
                    auto_mode = 1;
                    serial_printf("\r\nset to auto mode\n\r");
                }else if(strncmp(command_buffer, "set", 3) == 0){

                    int new_value;
                    sscanf(command_buffer, "set %d", &new_value);     

                    for(i=0;i<6;i++){
                        output_values[i] = new_value;
                        set_output(i, output_values[i]);
                    }

                    auto_mode = 0;
                    serial_printf("\r\nset all outputs to: %d%%\n\r", new_value);
                }else if(strncmp(command_buffer, "config get", 10) == 0){ 
                    serial_printf( "\r\ncurrent config:\r\n");
                    serial_printf( "  minimal set value: %d%%\n\r", minimal_set_value);        
                    serial_printf( "  reduction factor : %f\n\r", reduction_factor);    
                    serial_printf( "  cutoff value     : %d%%\n\r", cutoff_value);    
                    
                }else if(strncmp(command_buffer, "config set minimal set value", 28) == 0){ 
                    sscanf(command_buffer, "config set minimal set value %d", &minimal_set_value);
                    write_config();
                    serial_printf( "\n\rminimal set value set to: %d%%\n\r", minimal_set_value);  
                    
                }else if(strncmp(command_buffer, "config set reduction factor", 27) == 0){ 
                    sscanf(command_buffer, "config set reduction factor %f", &reduction_factor);
                    write_config();
                    serial_printf( "\n\rreduction factor set to: %f\n\r", reduction_factor);   
                    
                }else if(strncmp(command_buffer, "config set cutoff value", 10) == 0){ 
                    sscanf(command_buffer, "config set cutoff value %d", &cutoff_value);
                    write_config();
                    serial_printf( "\n\rcutoff value set to: %d%%\n\r", cutoff_value);    
                    
                }else if(strncmp(command_buffer, "take off", 8) == 0){ 
                    auto_mode = 0;
                    for(i=0;i<6;i++){
                        output_values[i] = 100;
                        set_output(i, output_values[i]);
                    }
                    serial_printf( "\n\rtaking off ...\n\r");    
                }else{
                    serial_printf("\n\runknown command\n\r");
                }
                bytes_in_command_buffer = 0;

                serial_printf("> ",command_buffer);    
            }else{
                if(read_byte == 0x7F){ //backspace
                    if(bytes_in_command_buffer > 0){
                        bytes_in_command_buffer--;
                        putUSBUSART(&read_byte, 1);
                    }
                }else{
                    if(bytes_in_command_buffer <= 64){
                        if(read_byte >= 32 && read_byte <= 126){
                            command_buffer[bytes_in_command_buffer++] = read_byte;
                            putUSBUSART(&read_byte, 1);
                        }
                    } 
                }
                
            }
        }
    }
    CDCTxService();
}

void write_config(){
    
}

void serial_printf(const char *format_string, ...){
    va_list args;
    va_start(args,format_string);
    vsprintf(write_buffer,format_string,args);
    va_end(args);
    
    putsUSBUSART(write_buffer);
    
    while(!USBUSARTIsTxTrfReady()){
        CDCTxService();
    }
}

void set_output(int output, int value){
    int set_value = (int)((float) (100 - value) * 6.4f);
    if(set_value >= 640)
        set_value = 639;
    if(set_value <= 0)
        set_value = 1;

    switch(output){
        case 0:
            OC1_PrimaryValueSet(set_value);
            break;
        case 1:
            OC2_PrimaryValueSet(set_value);
            break;
        case 2:
            OC3_PrimaryValueSet(set_value);
            break;
        case 3:
            OC4_PrimaryValueSet(set_value);
            break;
        case 4:
            OC5_PrimaryValueSet(set_value);
            break;
        case 5:
            OC6_PrimaryValueSet(set_value);
            break;
            
    }
}

int read_input(int input) {
   switch(input){
        case 0:
            AD1CHS = 10;
            break;
        case 1:
            AD1CHS = 11;
            break;
        case 2:
            AD1CHS = 12;
            break;
        case 3:
            AD1CHS = 9;
            break;
        case 4:
            AD1CHS = 6;
            break;
        case 5:
            AD1CHS = 7;
            break;
            
    }     
   
    AD1CON1bits.SAMP = 1; // start sampling...
    __delay_us(100); // Ensure the correct sampling time has elapsed
    // before starting conversion.
    AD1CON1bits.SAMP = 0; // start converting
    while (!AD1CON1bits.DONE){}; // conversion done?
    int conversion = ADC1BUF0; // yes then get ADC value
    conversion = 100 - ((int)(((float)conversion / 1024.0f) * 100));
    
    return conversion;
}
