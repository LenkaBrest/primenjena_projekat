#include <p30fxxxx.h>
#include "tajmer.h"





#define TMR2_period 81 /*  Fosc = 806.4kHz,
					          1/Fosc = 1.24us !!!, 1.24us * 806 = 1ms
                                24 za buzzer  */
#define TMR1_period 24 /*  Fosc = 806.4kHz,
					          1/Fosc = 1.24us !!!, 1.24us * 24 = 0.03ms ,
                                Fzv = 3.8kHz Tzv=0.26ms */

void Init_T2(void)
{
	TMR2 = 0;
	PR2 = TMR2_period;
	
	T2CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
	//IPC1bits.T2IP = 3 // T2 interrupt pririty (0-7)
	//SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS0bits.T2IF = 0; // clear interrupt flag
	IEC0bits.T2IE = 1; // enable interrupt

	T2CONbits.TON = 1; // T2 on 
}

//---------------------------------------

void Init_T1(void)
{
	TMR1 = 0;
	PR1 = TMR1_period;
	
	T2CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
	//IPC1bits.T2IP = 3 // T2 interrupt priority (0-7)
	//SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS0bits.T1IF = 0; // clear interrupt flag
	IEC0bits.T1IE = 1; // enable interrupt

	T1CONbits.TON = 1; // T2 on 
}
