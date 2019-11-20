#include <xc.h>
#include <stdint.h>
#include "seven.h"

/**
 * RC0 => clock 74ls373 display digit
 * RC1 => clock 74ls373 row digit
 * PORTB => display digit/row (74ls373 display and 74ls373 row(
 * PORTA => bit 0 => analog in; bit 1-2 optical coupler responsible for inout counting
 * RA3 => step motor clock
 */

//#include <pic16f872.h>
//typedef uint16_t word;
//word __at 0x2007 CONFIG = _HS_OSC & _WDT_OFF & _PWRTE_OFF & _BODEN_ON & _LVP_OFF & _CPD_OFF & _WRT_ENABLE_OFF & _DEBUG_OFF & _CP_OFF;

#pragma config FOSC=HS, WDTE=OFF, PWRTE=OFF, BOREN=ON, LVP=OFF, CP=OFF, WRT=OFF, DEBUG=OFF, CPD=OFF

#define TMR_VALUE 0

uint16_t display=0;
uint16_t advalue;
uint8_t  row=0;
uint8_t  value;
uint8_t  pos;

uint16_t wast = 0;

void refreshDisplay()
{
    switch(row) {
    case 0:
      value = to7(display & 0xf);
      pos = 8; //1;
      break;
    case 1:
      value = to7((display & 0xf0) >> 4);
      pos = 4; //2;
      break;
    case 2:
      value = to7((display & 0xf00) >> 8);
      pos = 2; //4;
      break;
    case 3:
      value = to7((display & 0xf000) >> 12);
      pos = 1; //8;
      break;	  
    }

    //if (row == 0 && RA0)
    //    value &= 0x7F;

    //if (row == 1 && RA1)
    //    value &= 0x7F;

    PORTB = value;
    RC1 = 1;
    RC1 = 0;

    PORTB = pos;
    RC0 = 1;
    RC0 = 0;

    row = (row+1) & 3;   
}

void __interrupt() tcInt(void)
{                                                                                                         /* interrupt service routine */
    if (TMR0IF) {
        static uint8_t myflip=0;
        RA5 = myflip & 1;
        myflip++;
        refreshDisplay();
        TMR0 = TMR_VALUE;
        TMR0IF = 0;
    }

    if (ADIF) {
        advalue =  ((ADRESH<<8)+ADRESL);
        ADIF = 0;
        GO = 1;
    }
}

void sleepx(int count)
{
    int i, j;
    
    for (j=0; j<count; j++)
        for (i=0; i<100; i++)
            ;
}

uint16_t bin2bcd(uint16_t hexValue)
{
    uint16_t rc = 0;   
    int8_t x=0;
    
    while (hexValue > 0 && x <= 12) {
        rc |= (hexValue % 10) << x;
        x += 4;
        hexValue /= 10;
    }

    return rc;
}

void main() {
    uint16_t count = 0;
    uint16_t rotation_counter = 0;
    uint8_t flip = 0;
    
    uint8_t rep = 0, new, old;
    // OPTION_REG 
    T0CS = 0;           // TMR0 Clock Source Select bit = Internal instruction cycle clock (CLKOUT)
    
    TMR0IE = 1;         // TMR0 Overflow Interrupt Enable bit = Enables the TMR0 interrupt
    
    nRBPU = 0;          // PORTB Pull-up Enable bit = PORTB pull-ups are enabled by individual port latch values
    
    PSA = 0;            // Assign prescaler to Timer0 Module
    PS2 = 1;            // PS{2:1:0} = 111 => TMR0 Rate = 1:256
    PS1 = 1;
    PS0 = 0;
    TMR0 = TMR_VALUE;

    // Analog Setup
    T1OSCEN = 0;        // T1OSCEN: Timer1 Oscillator Enable Control bit = Oscillator is shut off

    ADCON1 = 0xE;       // ADFM = 0 = Left justified 
                        // Six Least Significant bits of ADRESL are read as ‘0’.
                        // AN0 is analog and all others digital
    ADIE   = 1;
    PEIE   = 1;         // enable analog interrupt
    ADCON0 = 0x81;      // ADCS1:ADCS0 = 11 = FOSC/32
                        // CHS2:CHS0   = 000 = Channel 0 (RA0/AN0)
                        // GO/DONE     = ?
                        // ADON        = 1 = A/D converter module is operating
    GO      = 1;        // start conversion
    
    TRISA = 7;          // PORTA0, PORTA1 and PORTA2 are inputs
    TRISB = 0;          // PORTB = output
    TRISC = 0;          // PORTC = output

    // Enable interrupts
    GIE = 1;
    old = PORTA & 7 >> 1;
    while(1) {
#ifdef COUNTER
        new = (PORTA & 3) >> 1;
        if (new != old) {
            rep++;
            // debounce
            if (rep == 50) {
              char jmp = old | (new << 2);
              switch (jmp) {
              case 4:
              case 13:
              case 11:
              case 2:
                count++;
                break;

              case 1:
              case 7:
              case 14:
              case 8:
                count--;
                break;		
              }

              //display = bin2bcd(old + new * 10);
              display = bin2bcd(count/4);


              rep = 0;
              old = new;
            }
        } else
          rep = 0;      
#endif
        
        if (rotation_counter == 0) {
            rotation_counter = (advalue >> 8) + 1;
            RA3 = flip;
            flip = (flip + 1) & 1;
        }
        
        --rotation_counter;
        
        display = bin2bcd(advalue >> 6);
    }
}
