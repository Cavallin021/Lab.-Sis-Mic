#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0
#define BR100K 11

volatile uint32_t somador = 0;
volatile uint16_t GraphVec[2000], HeartVec[5], BpmVec[10];
volatile uint16_t i = 0, j = 0;
volatile int amostras = 0, h = 0, g = 0, BPM = 0;

/* ----------------------------  Código para inicialização do LCD (Visto 2 - Módulo 3) ---------------------------- */

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

char i2c_test(char adr) {
    UCB0I2CSA = adr;
    UCB0CTL1 |= UCTR;
    UCB0CTL1 |= UCTXSTT;
    while ((UCB0IFG & UCTXIFG) == 0);
    UCB0CTL1 |= UCTXSTP;
    while ((UCB0CTL1 & UCTXSTP) == UCTXSTP);
    if ((UCB0IFG & UCNACKIFG) == 0) return TRUE;
    else return FALSE;
}

void USCI_B0_config(void) {
    UCB0CTL1 = UCSWRST;
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
    UCB0BRW = BR100K;
    UCB0CTL1 = UCSSEL_3;
    P3SEL |= BIT1 | BIT0;
    P3REN |= BIT1 | BIT0;
    P3OUT |= BIT1 | BIT0;
}

void leds_config(void) {
    P1DIR |= BIT0; P1OUT &= ~BIT0;
    P4DIR |= BIT7; P4OUT &= ~BIT7;
}

void delay(long int x) {
    volatile long int i;
    for (i = 0; i < x; i++);
}

void i2c_write_nibble(char nibble) {
    i2c_write(nibble | 0x0C);
    delay(100);
    i2c_write(nibble & 0xF8);
    delay(100);
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
    delay(15000);
    lcd_command(0x3);
    delay(5000);
    lcd_command(0x3);
    delay(110);
    lcd_command(0x3);
    delay(110);
    lcd_command(0x2);
    delay(110);

    lcd_command(0x28);
    lcd_command(0x08);
    lcd_command(0x01);
    delay(5000);
    lcd_command(0x06);
    lcd_command(0x0C);
}

void lcd_set_cursor(char col, char row) {
    char addr = (row == 0) ? col : (0x40 + col);
    lcd_command(0x80 | addr);
}

void lcd_print(char* str) {
    while (*str) {
        lcd_data(*str++);
    }
}

void lcd_clear(void) {
    lcd_command(0x01);
    delay(2000);
}
/*------------------------------------------------------------------------------------------------*/

void adc_config(void) {
    ADC12CTL0 &= ~ADC12ENC;
    REFCTL0 = 0;
    ADC12CTL0 = ADC12SHT0_2 | ADC12MSC | ADC12ON;
    ADC12CTL1 = ADC12CSTARTADD_0 | ADC12SHS_0 | ADC12SSEL_0 | ADC12CONSEQ_0;
    ADC12CTL2 = ADC12TCOFF | ADC12RES_2;
    ADC12MCTL0 = 0;
    ADC12IE = BIT0;
    ADC12CTL0 |= ADC12ENC;
    P6SEL |= BIT0;
}

void timerA0_config(void) {
    TA0CTL = TASSEL__ACLK | MC__UP;
    TA0CCR0 = 66; // 2 milisegundos = 500 amostras a cada 1 segundo
    TA0CCTL0 = CCIE;
}

void timerA0_stop(void) {
    TA0CTL = MC_0;
}

/*----------------------------------------------------------------------------------*/

void ClearVec(volatile uint16_t* vector) {
    uint16_t i;
    for (i = 0; i < 2000; i++) {
        vector[i] = 0;
    }
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    USCI_B0_config();
    leds_config();
    adc_config();
    timerA0_config();
    ClearVec(HeartVec);
    UCB0I2CSA = 0x27;
    lcd_init();
    char buffer[16];

    int sBpm = 0, b = 0,contadorBPM = 0,BPM;

    __enable_interrupt();

    while (TRUE) {
        if (amostras >= 5) {
            for (j = 0; j < 5; j++) {
                somador += HeartVec[j];
            }
            int media = somador / 5;
            int bpm = abs((150000 / media) - 60) + 60;
            GraphVec[g++] = bpm;

            BpmVec[b++] = bpm;
            sBpm += bpm;
            contadorBPM++;
            if (contadorBPM == 5) {
                BPM = sBpm / 5;
                sBpm = 0;
                contadorBPM = 0;
                ClearVec(BpmVec);
            }

            lcd_set_cursor(5, 0);
            lcd_print("BPM: ");
            snprintf(buffer, sizeof(buffer), "%d  ", BPM);
            lcd_print(buffer);
            lcd_set_cursor(3, 1);
            lcd_print("BPM(ms):");
            snprintf(buffer, sizeof(buffer), "%d", bpm);
            lcd_print(buffer);
            ClearVec(HeartVec);
            amostras = 0;
            somador = 0;
            h = 0; 
        }
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    ADC12CTL0 |= ADC12ENC;
    ADC12CTL0 &= ~ADC12SC;
    ADC12CTL0 |= ADC12SC;
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR() {
    ADC12IFG = 0;
    if (ADC12MEM0 >= 2400 && h < 5) {
        HeartVec[h++] = ADC12MEM0;
        amostras++;
    }
}
