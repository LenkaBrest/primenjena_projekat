#include<p30fxxxx.h>
#include <stdlib.h>
#include "driverGLCD.h"
#include "adc.h"
#include "tajmer.h"




//_FOSC(CSW_FSCM_OFF & XT_PLL4);//instruction takt je isti kao i kristal//
_FOSC(CSW_ON_FSCM_OFF & HS3_PLL4);
_FWDT(WDT_OFF);
_FGS(CODE_PROT_OFF);

unsigned int X, Y,x_vrednost, y_vrednost;


//const unsigned int ADC_THRESHOLD = 900; 
const unsigned int AD_Xmin =354;
const unsigned int AD_Xmax =4095;
const unsigned int AD_Ymin =744;
const unsigned int AD_Ymax =3960;

unsigned int sirovi0,sirovi1, sirovi2, sirovi3;
unsigned int broj,broj1,broj2,temp0,temp1; 

unsigned char tacan[]={2, 9, 0, 7};
unsigned char niz[4];
unsigned char i=0,j;
unsigned char greska;
unsigned char provera=0;

//#define DRIVE_A PORTBbits.RB10
//#define DRIVE_B PORTCbits.RC13
#define DRIVE_A PORTCbits.RC13
#define DRIVE_B PORTCbits.RC14

void ConfigureTSPins(void)
{
	//ADPCFGbits.PCFG10=1;
	//ADPCFGbits.PCFG7=digital;
    ADPCFGbits.PCFG10=1;
    ADPCFGbits.PCFG11=1;
    

	//TRISBbits.TRISB10=0;
	TRISCbits.TRISC13=0;
    TRISCbits.TRISC14=0;
    
    //SERVO
    TRISFbits.TRISF6=0; //servo
    TRISBbits.TRISB11=1;//PIR
    //TRISDbits.TRISD8=1;
    TRISDbits.TRISD9=1;//flame
    TRISBbits.TRISB7=1; //sensordodira
    TRISAbits.TRISA11=0; //buzzer
	
	//LATCbits.LATC14=0;
	//LATCbits.LATC13=0;
}

//int h;

void initUART1(void)
{
    
    U1BRG=0x0015;//ovim odredjujemo baudrate
    
    U1MODEbits.ALTIO=0;//biramo koje pinove koristimo za komunikaciju osnovne ili alternativne

    IEC0bits.U1RXIE=1;//omogucavamo rx1 interupt

    U1STA&=0xfffc;

    U1MODEbits.UARTEN=1;//ukljucujemo ovaj modul

    U1STAbits.UTXEN=1;//ukljucujemo predaju
}

void __attribute__((__interrupt__)) _U1RXInterrupt(void) 
{
    IFS0bits.U1RXIF = 0;
    niz[i++]=U1RXREG;

} 

void WriteUART1(unsigned int data)
{
	  while(!U1STAbits.TRMT);

    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
}

void RS232_putst(register const char *str)
{
    while((*str)!=0)
    {
        WriteUART1(*str);
        if (*str==13) WriteUART1(10);
        if (*str==10) WriteUART1(13);
        str++;
    }
}

/***********************************************************************
* Ime funkcije      : WriteUART1dec2string                     		   *
* Opis              : Funkcija salje 4-cifrene brojeve (cifru po cifru)*
* Parameteri        : unsigned int data-podatak koji zelimo poslati    *
* Povratna vrednost : Nema                                             *
************************************************************************/
void WriteUART1dec2string(unsigned int data)
{
	unsigned char temp;

	temp=data/1000;
	WriteUART1(temp+'0');
	data=data-temp*1000;
	temp=data/100;
	WriteUART1(temp+'0');
	data=data-temp*100;
	temp=data/10;
	WriteUART1(temp+'0');
	data=data-temp*10;
	WriteUART1(data+'0');
}

void Draw_Table(void)
{
    GoToXY(21,1);		
		GLCD_Printf ("1");
        GoToXY(63,1);		
		GLCD_Printf ("2");
        GoToXY(105,1);		
		GLCD_Printf ("3");
        GoToXY(21,3);		
		GLCD_Printf ("4");
        GoToXY(63,3);		
		GLCD_Printf ("5");
        GoToXY(105,3);		
		GLCD_Printf ("6");
        GoToXY(21,5);		
		GLCD_Printf ("7");
        GoToXY(63,5);		
		GLCD_Printf ("8");
        GoToXY(105,5);		
		GLCD_Printf ("9");
        GoToXY(16,7);		
		GLCD_Printf ("<-");
        GoToXY(63,7);		
		GLCD_Printf ("0");
        GoToXY(100,7);		
		GLCD_Printf ("->");
        
		GLCD_Rectangle(11,6,31,18);
        GLCD_Rectangle(53,6,73,18);
        GLCD_Rectangle(95,6,115,18);
        GLCD_Rectangle(11,22,31,34);
        GLCD_Rectangle(53,22,73,34);
        GLCD_Rectangle(95,22,115,34);
        GLCD_Rectangle(11,38,31,50);
        GLCD_Rectangle(53,38,73,50);
        GLCD_Rectangle(95,38,115,50);
        GLCD_Rectangle(11,54,31,63);
        GLCD_Rectangle(53,54,73,63);
        GLCD_Rectangle(95,54,115,63);
}

void Delay(unsigned int N)
{
	unsigned int i;
	for(i=0;i<N;i++);
}

unsigned int brojac_ms,stoperica,ms,sekund;
unsigned int brojac_ms1, stoperica1, ms1;

void Delay_motor (int vreme)//funkcija za kasnjenje u milisekundama
	{
		stoperica = 0;
		while(stoperica < vreme);
	}



void __attribute__ ((__interrupt__)) _T2Interrupt(void) // svakih 1ms
{

		TMR2 =0;
     ms=1;//fleg za milisekundu ili prekid;potrebno ga je samo resetovati u funkciji

	brojac_ms++;//brojac milisekundi
    stoperica++;//brojac za funkciju Delay_ms

    if (brojac_ms==1000)//sek
        {
          brojac_ms=0;
          sekund=1;//fleg za sekundu
		 } 
	IFS0bits.T2IF = 0; 
       
}

void Delay_buzzer (int vreme)//funkcija za kasnjenje u milisekundama
	{
		stoperica1 = 0;
		while(stoperica1 < vreme);
	}

void __attribute__ ((__interrupt__)) _T1Interrupt(void) // svakih 2.567s
{

		TMR1 =0;
   

	brojac_ms1++;//brojac milisekundi
 stoperica1++;//brojac za funkciju Delay_ms

    if (brojac_ms1==1000)//sek
        {
          brojac_ms1=0;
          sekund=1;//fleg za sekundu
		 } 
	IFS0bits.T1IF = 0; 
       
}



void Touch_Panel (void)
{
// vode horizontalni tranzistori
	DRIVE_A = 1;  
	DRIVE_B = 0;
    
     LATCbits.LATC13=1;
     LATCbits.LATC14=0;

	Delay(500); //cekamo jedno vreme da se odradi AD konverzija
				
	// ocitavamo x	
	x_vrednost = temp0;//temp0 je vrednost koji nam daje AD konvertor na BOTTOM pinu		

	// vode vertikalni tranzistori
     LATCbits.LATC13=0;
     LATCbits.LATC14=1;
	DRIVE_A = 0;  
	DRIVE_B = 1;

	Delay(500); //cekamo jedno vreme da se odradi AD konverzija
	
	// ocitavamo y	
	y_vrednost = temp1;// temp1 je vrednost koji nam daje AD konvertor na LEFT pinu	
	
//Ako želimo da nam X i Y koordinate budu kao rezolucija ekrana 128x64 treba skalirati vrednosti x_vrednost i y_vrednost tako da budu u opsegu od 0-128 odnosno 0-64
//skaliranje x-koordinate

    X=(x_vrednost-354)*0.0342;



//X= ((x_vrednost-AD_Xmin)/(AD_Xmax-AD_Xmin))*128;	
//vrednosti AD_Xmin i AD_Xmax su minimalne i maksimalne vrednosti koje daje AD konvertor za touch panel.


//Skaliranje Y-koordinate
	Y= ((y_vrednost-744)*0.0199);

//	Y= ((y_vrednost-AD_Ymin)/(AD_Ymax-AD_Ymin))*64;
}

void Unos(void)
{
    if ((11<X)&&(X<31)&& (Y<58)&&(Y>46))
    {
        niz[i]=1;
        i++;
        } 
    if ((53<X)&&(X<73)&& (46<Y)&&(Y<58))
    {
          
        niz[i]=2;
     i++;
    Delay(250);}
    if ((95<X)&&(X<115)&& (46<Y)&&(Y<58))
    {   
    niz[i]=3;
    i++;
    Delay(250);}
    if ((11<X)&&(X<31)&& (30<Y)&&(Y<42))
    {   
    niz[i]=4;
    i++;
    Delay(250);}
    if ((53<X)&&(X<73)&& (30<Y)&&(Y<42))
    {   
    niz[i]=5;
    i++;
    Delay(250);}
    if ((53<X)&&(X<73)&& (30<Y)&&(Y<42))
    {  
    niz[i]=6;
    i++;
    Delay(250);}
    if ((11<X)&&(X<31)&& (14<Y)&&(Y<26))
    {   
    niz[i]=7;
    i++;
    Delay(250);}
    if ((53<X)&&(X<73)&& (14<Y)&&(Y<26))
    {   
    niz[i]=8;
    i++;
    Delay(250);}
    if ((95<X)&&(X<115)&& (14<Y)&&(Y<26))
    {  
    niz[i]=9;
    i++;
    Delay(250);}
    if ((11<X)&&(X<31)&& (0<Y)&&(Y<10))
    {  
    i--;
    Delay(250);}
    if ((53<X)&&(X<73)&& (0<Y)&&(Y<10))
    {  
    niz[i]=0;
    i++;
    Delay(250);}
    if ((95<X)&&(X<115)&& (0<Y)&&(Y<10))
    {  
        i=4;
    Delay(250);}
}

int Provera(void)
{
    greska = 0;
     if(i==4)
    {
        if(niz[0]=='2' && niz[1]=='9'&& niz[2]=='0' && niz[3]=='7' )
            greska = 0;
        else
            greska = 1;   

        for(i=0; i<4; i++)
            niz[i]=0;
     }
    i = 0;
     return greska;
}
        
        //Glcd_PutChar(greska+48);
        /*
        if(Provera()){
            
                 GLCD_ClrScr();
                 GoToXY(0,0);
                 GLCD_Printf ("Pogresna sifra,unesite opet");
                 Delay(500);
                 
                 GLCD_ClrScr();
                 
        }
        else
        {
        GLCD_ClrScr();
        GoToXY(0,0);
        GLCD_Printf ("Dobro je");
        Delay(5000);
        provera=1;
        //GLCD_ClrScr();
        
        
        } 
        i=0;
        greska=0;
    }
}*/
	
void __attribute__((__interrupt__)) _ADCInterrupt(void) 
{
							
	
	sirovi0=ADCBUF2;//0
	sirovi1=ADCBUF3;//1
    sirovi2 = ADCBUF0;
    sirovi3 = ADCBUF1;
    
    
	temp0=sirovi0;
	temp1=sirovi1;

    IFS0bits.ADIF = 0;
} 

void Write_GLCD(unsigned int data)
{
unsigned char temp;

temp=data/1000;
Glcd_PutChar(temp+'0');
data=data-temp*1000;
temp=data/100;
Glcd_PutChar(temp+'0');
data=data-temp*100;
temp=data/10;
Glcd_PutChar(temp+'0');
data=data-temp*10;
Glcd_PutChar(data+'0');
}

//vraca 1 ako detektuje alkohol
int mq3(void)
{
    int yy=0;
    /*for(broj1=0;broj1<1000;broj1++)
		for(broj2=0;broj2<3000;broj2++);*/
    //WriteUART1dec2string(sirovi2);
    if(sirovi2>700)
    {
        //WriteUART1dec2string(sirovi2);
        yy=1;
    }
    else if(sirovi2>150)
        yy = 2;
    /*if(yy==1)
    {
        RS232_putst("Promasili ste kucu.");
        pocetak = 0;
        yy=0;
    }
    else
    {
        pocetak = 0;
        yy=0;
    }
        /*for(broj1=0;broj1<1000;broj1++)
		for(broj2=0;broj2<3000;broj2++);*/
    //}
    return yy;
}

int pir(void)
{
    int pir=0;
    for(broj1=0;broj1<1000;broj1++)
		for(broj2=0;broj2<3000;broj2++);
    if(PORTBbits.RB11)
            pir =1;
    /*int q;
        for( q= 0; q < 1000; q++);*/
    return pir; 
}       
        /*if(pir==1)
        {
            RS232_putst("Pokret je detektovan");
            pir=0;
        }
        else
            RS232_putst("Pokret nije detektovan");*/

void buzzer(void)
{
    for(i=0; i<1000; i++)
    {
        LATAbits.LATA11=1;
        Delay_buzzer(7);
        LATAbits.LATA11=0;
        Delay_buzzer(3);
        //LATAbits.LATA11=1;
    }
}

void otvori_vrata_skroz(void)
{
    for(i=0; i<50; i++)
        {
                    LATFbits.LATF6 = 1;
                    Delay_motor(2);
                    LATFbits.LATF6= 0;
                    Delay_motor(198);
                    LATFbits.LATF6 = 1;
        }
}
void zatvori_vrata()
{
for(i=0; i<50; i++)
        {
                    LATFbits.LATF6 = 1;
                    Delay_motor(20);
                    LATFbits.LATF6= 0;
                    Delay_motor(180);
                    LATFbits.LATF6 = 1;
        }
}
void napola_vrata()
{
    for(i=0; i<50; i++)
        {
                    LATFbits.LATF6 = 1;
                    LATFbits.LATF6 = 1;
                    Delay_motor(10);
                    LATFbits.LATF6= 0;
                    Delay_motor(190);
                    LATFbits.LATF6 = 1;
        }
}





//int pir = 0;
int flame=0;
int sound=0;
int pocetak=0;
int otvoreno=0;
int stanje=0;

void main(void)
{
    ConfigureLCDPins();
	ConfigureTSPins();

	GLCD_LcdInit();

	GLCD_ClrScr();
    initUART1();
	
	ADCinit();
	ConfigureADCPins();
	ADCON1bits.ADON=1;

    //Draw_Table();
    
    Init_T2();
    Init_T1();
    stanje =2;
	while(1)
	{ 
         //RS232_putst("Pogresili ste kucu.");
        switch(stanje)
        {
                case 0: for(broj2=0;broj2<3000;broj2++);
                        if(PORTBbits.RB7==1)
                            stanje = 2;
                        else
                            stanje = 0;
                        break;
                case 1: for(broj2=0;broj2<3000;broj2++);
                        Draw_Table();
                        Unos();
                        if(Provera())
                            stanje = 2;
                        else
                        {
                            otvori_vrata_skroz();
                            stanje = 5;
                        }
                        greska = 0;
                        break;
                case 2: for(broj2=0;broj2<3000;broj2++);
                        if(mq3()==1) //ne ispise ni ovo iz if ni ovo iz else????
                        {
                            GLCD_Printf("Pogresili ste kucu.");
                            stanje = 0;
                        }
                        else if(mq3() == 2)
                            stanje = 3;
                        break;
                case 3: for(broj2=0;broj2<3000;broj2++);
                        RS232_putst("Unesite sifru");
                        if(Provera())
                        {
                            RS232_putst("Pogresna sifra.");
                            stanje = 4;
                        }
                        else
                        {
                            otvori_vrata_skroz();
                            stanje = 5;
                        }
                        greska = 0;
                        break;
                case 4: for(broj2=0;broj2<3000;broj2++);
                        if(pir())
                        {
                            RS232_putst("Lopov");
                            buzzer();
                            stanje = 0;
                        }
                        else
                        {
                            stanje = 0;
                        }
                        break;
                case 5: for(broj2=0;broj2<3000;broj2++);
                        zatvori_vrata();
                        RS232_putst("Nalozi");
                        if(PORTDbits.RD9)
                        {
                            buzzer();
                            napola_vrata();
                            stanje = 0;
                        }
                        else
                            stanje = 5;
                        break;
        }
                    
    }
}

/*void main(void)
{

	ConfigureLCDPins();
	ConfigureTSPins();

	GLCD_LcdInit();

	GLCD_ClrScr();
    initUART1();
	
	ADCinit();
	ConfigureADCPins();
	ADCON1bits.ADON=1;

    //Draw_Table();
    
    Init_T2();
    Init_T1();

	while(1)
	{
	//Touch_Panel ();
    //LcdSetDot(X,64-Y);
    //provera = 1;
        //RS232_putst ("Promasio si kucu.");
        if(pocetak == 0)
            pocetak = 1;
        
        if(pocetak == 1)
        {
            if(PORTBbits.RB7);
                provera = 1;
        }
            
            
            
            //Draw_Table();
   
    if(provera==1)
    { 
       /* Draw_Table();
        Unos();
        if(Provera() && i==4)
        {          
                 GLCD_ClrScr();
                 GoToXY(0,0);
                 GLCD_Printf ("Pogresna sifra. Pokrenuce se alko test");
                 Delay(500);                 
                 GLCD_ClrScr();
                 if(mq3())
                 {
                    GLCD_ClrScr();
                    GoToXY(0,0);
                    RS232_putst ("Promasio si kucu.");
                    pocetak=0;
                 }
                 else
                 {
                    RS232_putst("Pokusajte jos jednom sa racunara.");
                    while(i!=4);
                 // WriteUART1('1');
                   if(Provera()==1)
                   {
                       Delay(500); 
                       if(pir())
                       {
                            RS232_putst("Lopove"); 
                            buzzer();
                       }
                   }
                   else
                       otvoreno =1;
                 }
        /*}
        else
        {
            GLCD_ClrScr();
            GoToXY(0,0);
            GLCD_Printf ("Dobro je");
            Delay(5000);
            otvoreno = 1;

        }
        
        if(otvoreno == 1)
        {
                        otvori_vrata_skroz();
                        Delay(5000);
                        RS232_putst("Zalozi vatru"); 
                        zatvori_vrata();
                        while(!PORTDbits.RD9);
                        napola_vrata();
        }
       /* provera=1;
        //GLCD_ClrScr();
        
        
        } 
        i=0;
        greska=0;
    }
       // Provera();
    //}
    else
    {
        //RS232_putst("Pogresno uneta sifra. Pokusajte jos jednom sa racunara.");
        //while(i!=4);
                 // WriteUART1('1');
               /* if(Provera()==1)
                    RS232_putst("Ne valja.");*/
                    
                //else
                //{
                    //RS232_putst("Dobro uneta sifra.");
                    //SERVO
//        {
        /*for(i=0; i<50; i++)
        {
                    LATFbits.LATF6 = 1;
                    Delay_motor(20);
                    LATFbits.LATF6= 0;
                    Delay_motor(180);
                    LATFbits.LATF6 = 1;
        }
                    //for(i=0; i<1000; i++);

        /*for(i=0; i<50; i++)
        {
                    LATFbits.LATF6 = 1;
                    Delay_ms(17);
                    LATFbits.LATF6= 0;
                    Delay_ms(185);
                    LATFbits.LATF6 = 1;
        }*/
                  //  for(i=0; i<10000; i++);
        /*for(i=0; i<50; i++)
        {
                    LATFbits.LATF6 = 1;
                    LATFbits.LATF6 = 1;
                    Delay_motor(10);
                    LATFbits.LATF6= 0;
                    Delay_motor(190);
                    LATFbits.LATF6 = 1;
        }
        for(i=0; i<50; i++)
        {
                    LATFbits.LATF6 = 1;
                    Delay_motor(2);
                    LATFbits.LATF6= 0;
                    Delay_motor(198);
                    LATFbits.LATF6 = 1;
        }
       // }
                    //BUZZER
                    
                    
                    for(i=0; i<1000; i++)
                    {
                    LATAbits.LATA11=1;
                    Delay_buzzer(7);
                    LATAbits.LATA11=0;
                    Delay_buzzer(3);
                    //LATAbits.LATA11=1;
                    }*/
                    
                    //mq3();
                   // RS232_putst(PORTD);
                    
                //}
        //i=0;
        
        //GLCD_ClrScr();
        //Write_GLCD(sirovi3);
        
        
        /*
        if(PORTDbits.RD9)
            flame =1;
        if(flame=1)
        {
            for(i=0; i<100; i++)
                    {
                    LATAbits.LATA11=1;
                    Delay_buzzer(7);
                    LATAbits.LATA11=0;
                    Delay_buzzer(3);
            }
                    LATFbits.LATF6 = 1;
                    LATFbits.LATF6 = 1;
                    Delay_motor(10);
                    LATFbits.LATF6= 0;
                    Delay_motor(190);
                    LATFbits.LATF6 = 1;
            //}
            
            flame=0;
        }
        else
        {
            GLCD_ClrScr();
            GLCD_Printf ("cist zrak");
        }
        
        if(PORTBbits.RB7)
            sound =1;
        for( q= 0; q< 1000; q++);
        if(sound==1)
        {
            GLCD_Printf ("deres se");
            sound=0;
        }
        
       /* Delay_ms(20000);
    stanje = ~stanje;
    LATAbits.LATA11 
    if(stanje == 1)
        cekanje = 1;
    else
        cekanje = 1;
    Delay_ms(cekanje);*/
    //LATAbits.LATA11 = 0;
    //Delay_ms(40 - cekanje);

       

    //pocetak = 0;
    //provera = 0;
    /*}
	}//while

}//main*/

			
		
												
