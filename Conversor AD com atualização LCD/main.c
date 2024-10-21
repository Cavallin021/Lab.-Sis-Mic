#include <msp430.h>
#include <stdio.h>
#include <math.h>

#define TRUE 1
#define FALSE 0
#define VRX 1
#define VRY 0

typedef unsigned char uint8;

unsigned char flag = 0;
int vrx,vry,tensx,tensy,minxI,minyI,decx,decy,maxxI,maxyI,minxD,minyD,maxxD,maxyD = 0;
char minxDstr[20];

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void debounce(long x);
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void set_USCIB();
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void i2c_write(char dado);
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void i2c_write_nibble(char nibble);
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void lcd_command(char cmd);
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void lcd_data(char data);
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void lcd_set_cursor(char col, char row);
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void lcd_print(char* str);
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void TAconfig();
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void GPIO_config();
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void adc_config();
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void sinal_analogico(void);
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
int sw_mon();
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void USCI_config();
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void bt_char(char c);
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void bt_str(char *vet);
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void basic_config();
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void uart_config();
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void lcd_init();
/*-------------------------------------------------------------------------------------------------------------------------------------------*/

#pragma vector = ADC12_VECTOR
__interrupt void isr_media(){
    volatile unsigned int *pt;
    unsigned int cont,somaX,somaY = 0;
    int maxx,maxy = 0;
    int minx,miny = 4095;
    pt = &ADC12MEM0;
    for(cont = 0; cont <= 7; cont++){
        if(cont % 2 == 0){
            somaX += *pt;
            maxx = (*pt > maxx) ? *pt : maxx;
            minx = (*pt < minx) ? *pt : minx;
        }
        else{
            somaY += *pt;
            maxy = (*pt > maxy) ? *pt : maxy;
            miny = (*pt < miny) ? *pt : miny;
        }
        ++pt;
    }
    vrx = somaX >>2;
    vry = somaY >>2;
    maxxI = (int)(0.0008*maxx);
    maxyI = (int)(0.0008*maxy);
    minxI = maxxI;
    minyI = (int)(0.0008*miny);
    maxxD = ((int)(100*((0.0008*maxx) - maxxI)));
    maxyD = ((int)(100*((0.0008*maxy) - maxyI)));
    minyD = (int)(100*((0.0008*miny) - minyI));
    tensx = (int)(0.0008*vrx);
    tensy = (int)(0.0008*vry);
    decx = (int)(1000*((0.0008*vrx) - tensx));
    decy = (int)(1000*((0.0008*vry) - tensy));
    flag = 1;
}



/*-------------------------------------------------------------------------------------------------------------------------------------------*/

int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    basic_config();
    uart_config();
    UCB0I2CSA = 0x27;
    char canal = VRX;
    char stringlcd[60];
    char uartstring[60];
    int contador = 0;
    char stringcontador[60];
    char stringvrx[60];
    char stringvry[60];
    char stringdecx[60];
    char stringdecy[60];
    i2c_write(0);
    lcd_init();
    bt_str("\rCont: ---Canal A1----   ---Canal A2----\r\n");
    while (TRUE) {
        if (flag == 1) {
            flag = 0;
            lcd_set_cursor(1, 0);
            debounce(1200);
            lcd_set_cursor(3, 0);
            debounce(1200);
            if (maxxD - 20 < 0) {
                minxI = (maxxI - 1 >= 0) ? maxxI - 1 : 0;
                minxD = (maxxI > 0) ? 100 - maxxD : 0;
                if (minxD > 10) {
                    snprintf(minxDstr, sizeof(minxDstr), "%d", minxD);
                } else {
                    snprintf(minxDstr, sizeof(minxDstr), "0%d", minxD);
                }
            } else if (maxxD - 20 < 10) {
                minxI = maxxI;
                int jaj = maxxD - 20;
                snprintf(minxDstr, sizeof(minxDstr), "0%d", jaj);
            } else {
                minxD = maxxD - 20;
                snprintf(minxDstr, sizeof(minxDstr), "%d", minxD);
            }
            if (contador < 10) {
                snprintf(stringcontador, sizeof(stringcontador), "000%d", contador);
            } else if (contador < 100) {
                snprintf(stringcontador, sizeof(stringcontador), "00%d", contador);
            } else if (contador < 1000) {
                snprintf(stringcontador, sizeof(stringcontador), "0%d", contador);
            } else {
                snprintf(stringcontador, sizeof(stringcontador), "%d", contador);
            }
            if (vrx < 10) {
                snprintf(stringvrx, sizeof(stringvrx), "000%d", vrx);
            } else if (vrx < 100) {
                snprintf(stringvrx, sizeof(stringvrx), "00%d", vrx);
            } else if (vrx < 1000) {
                snprintf(stringvrx, sizeof(stringvrx), "0%d", vrx);
            } else {
                snprintf(stringvrx, sizeof(stringvrx), "%d", vrx);
            }

            if (vry < 10) {
                snprintf(stringvry, sizeof(stringvry), "000%d", vry);
            } else if (vry < 100) {
                snprintf(stringvry, sizeof(stringvry), "00%d", vry);
            } else if (vry < 1000) {
                snprintf(stringvry, sizeof(stringvry), "0%d", vry);
            } else {
                snprintf(stringvry, sizeof(stringvry), "%d", vry);
            }

            if (canal == VRX) {
                if (vrx < 10) {
                    snprintf(stringdecx, sizeof(stringdecx), "0%d", decx);
                    snprintf(stringlcd, sizeof(stringlcd), "A1=%d,%d0V   000%d\nMn=%d,%s  Mx=%d,%d0", tensx, decx, vrx, minxI, minxDstr, maxxI, maxxD);
                    snprintf(uartstring, sizeof(uartstring), "\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n", stringcontador, stringvrx, tensx, stringdecx, stringvry, tensy, stringdecy);

                } else if (vrx < 100) {
                    snprintf(stringdecx, sizeof(stringdecx), "0%d", decx);
                    snprintf(stringlcd, sizeof(stringlcd), "A1=%d,%d0V   00%d\nMn=%d,%s  Mx=%d,%d0", tensx, decx, vrx, minxI, minxDstr, maxxI, maxxD);
                    snprintf(uartstring, sizeof(uartstring), "\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n", stringcontador, stringvrx, tensx, stringdecx, stringvry, tensy, stringdecy);

                } else if (vrx < 1000) {
                    snprintf(stringdecx, sizeof(stringdecx), "%d", decx);
                    snprintf(stringlcd, sizeof(stringlcd), "A1=%d,%dV   0%d\nMn=%d,%s  Mx=%d,%d", tensx, decx, vrx, minxI, minxDstr, maxxI, maxxD);
                    snprintf(uartstring, sizeof(uartstring), "\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n", stringcontador, stringvrx, tensx, stringdecx, stringvry, tensy, stringdecy);

                } else {
                    snprintf(stringdecx, sizeof(stringdecx), "%d", decx);
                    snprintf(stringlcd, sizeof(stringlcd), "A1=%d,%dV   %d\nMn=%d,%s  Mx=%d,%d", tensx, decx, vrx, minxI, minxDstr, maxxI, maxxD);
                    snprintf(uartstring, sizeof(uartstring), "\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n", stringcontador, stringvrx, tensx, stringdecx, stringvry, tensy, stringdecy);
                }
            } else {
                snprintf(stringdecy, sizeof(stringdecy), "%d", decy);
                snprintf(stringlcd, sizeof(stringlcd), "A2=%d,%dV   %d\nMn=%d,%d  Mx=%d,%d", tensy, decy, vry, minyI, minyD, maxyI, maxyD);
                snprintf(uartstring, sizeof(uartstring), "\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n", stringcontador, stringvrx, tensx, stringdecx, stringvry, tensy, stringdecy);
            }

            lcd_set_cursor(0, 0);
            lcd_print(stringlcd);
            bt_str(uartstring);
                TA2CCR2 =  524 + round((4095 - vry)*0.33);
            }
            debounce(1000);
            if(contador == 9999){
                contador = 0;
            }
            else{
                contador++;
            }
            if(sw_mon() == TRUE) canal ^= 1;
        }
}



/*----------------------------------------------------------------------------------------------*/


void GPIO_config(){
    P6DIR &= ~BIT3;
    P6OUT |= BIT3;
    P6REN |= BIT3;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;
}


void sinal_analogico(void){
    TA2CTL = TASSEL__SMCLK | MC__UP;
    TA2CCTL2 = OUTMOD_7;
    TA2CCR0 = 20971;
    TA2CCR2 = 687;

    P2SEL |= BIT5;
    P2DIR |= BIT5;
}


void adc_config(){
    volatile unsigned char *pt1;
    volatile unsigned int cont;
    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12ON;
    ADC12CTL1 = ADC12CONSEQ_3 |
                ADC12SHS_1 |
                ADC12CSTARTADD_0 |
                ADC12SSEL_3;
    ADC12CTL2 = ADC12RES_2;
    pt1 = &ADC12MCTL0;
    for(cont = 0; cont <= 7; cont++){
        if(cont % 2 == 0){
            *pt1 = ADC12SREF_0 | ADC12INCH_1;
        }
        else{
            *pt1 = ADC12SREF_0 | ADC12INCH_2;
        }
        ++pt1;
    }
    ADC12MCTL7 |= ADC12EOS;
    ADC12IE |= ADC12IE7;
    P6SEL |= BIT1|BIT2;
    ADC12CTL0 |= ADC12ENC;
}

void TA_config(){
    TA0CTL = TASSEL_2 | MC__UP;
    TA0CCTL1 = OUTMOD_7;
    TA0CCR0 = 32768;
    TA0CCR1 = TA0CCR0/2;
}

int sw_mon(){
    static int psw = 380;
    if((P6IN&BIT3) == 0){
        if(psw == 380){
            debounce(1000);
            psw = 44;
            return TRUE;
        }
    }
    else{
        if(psw == 44){
            debounce(1000);
            psw = 380;
            return FALSE;
        }
    }
    return FALSE;
}


void debounce(long x){
    volatile long i;
    for(i = 0; i < x; i++);
}


void basic_config(){
    GPIO_config();
    sinal_analogico();
    adc_config();
    TA_config();
    __enable_interrupt();
}


/*------------------------------------------------------------------------------------------------*/


void USCI_config(){
    UCA1CTL1 |= UCSWRST;
    UCA1BRW = 6;         
    UCA1MCTL |= UCBRF_13 | UCOS16;
    P4SEL |= BIT4;        
    UCA1CTL1 |= UCSSEL_2;
    UCA1CTL1 &= ~UCSWRST; 
}

void set_USCIB(){
    UCB0CTL1 = UCSWRST;
    UCB0CTL0 = UCMST | UCSYNC | UCMODE_3;
    UCB0BRW = 11;
    UCB0CTL1 = UCSSEL_3;

    P3SEL |= BIT1 | BIT0;
    P3REN |= BIT1 | BIT0;
    P3OUT |= BIT1 | BIT0;
}


void bt_char(char c){
    while((UCA1IFG&UCTXIFG) == 0);
    UCA1TXBUF = c;
}

void bt_str(char *vet){
    unsigned int i = 0;
    while (vet[i] != '\0'){
        bt_char(vet[i++]);
    }
}

void uart_config(){
    USCI_config();
    set_USCIB();
}

/*--------------------------------------------------------------------------------------------------------*/

void i2c_write(char dado) {
    UCB0CTL1 |= UCTR | UCTXSTT;
    while ((UCB0IFG & UCTXIFG) == 0);
    UCB0TXBUF = dado;
    while ((UCB0CTL1 & UCTXSTT) == UCTXSTT);
    if ((UCB0IFG & UCNACKIFG) == UCNACKIFG) {
        P1OUT |= BIT0;
        while(1);
    }
    UCB0CTL1 |= UCTXSTP;
    while ((UCB0CTL1 & UCTXSTP) == UCTXSTP);
}

void i2c_write_nibble(char nibble) {
    i2c_write(nibble | 0x0C);
    debounce(500);
    i2c_write(nibble & 0xF8);
    debounce(500);
}

void lcd_command(char cmd) {
    i2c_write_nibble(cmd & 0xF0);
    i2c_write_nibble((cmd << 4) & 0xF0);
}

void lcd_data(char data) {
    i2c_write_nibble((data & 0xF0) | 0x01);
    i2c_write_nibble(((data << 4) & 0xF0) | 0x01);
}

void lcd_init(void) {
    debounce(15000);
    lcd_command(0x3);
    debounce(5000);
    lcd_command(0x3);
    debounce(110);
    lcd_command(0x3);
    debounce(110);
    lcd_command(0x2);
    debounce(110);

    lcd_command(0x28);
    lcd_command(0x08);
    lcd_command(0x01);
    debounce(5000);
    lcd_command(0x06);
    lcd_command(0x0C);
}

void lcd_set_cursor(char col, char row) {
    char addr = (row == 0) ? col : (0x40 + col);
    lcd_command(0x80 | addr);
}

void lcd_print(char* str) {
    int col = 0;
    int row = 0;
    while (*str) {
        if (col == 16) {
            row++;
            col = 0;
            lcd_set_cursor(0, row);
            while (*str == ' ' && row == 1) {
                str++;
            }
        }
        lcd_data(*str++);
        col++;
    }
    i2c_write(0);
}
