#include <msp430.h>
#include <stdio.h>

void config_leds(void);
void key_map(void);
void TA_config(void);
void TB_config(void);
void capture_values(int *vet);
void decode_values(int *vet, volatile unsigned long *cod);
void execute_action(volatile unsigned long cod);
void clear_flag_and_timer(void);
int i;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    key_map();
    config_leds();
    TA_config();
    TB_config();

    volatile unsigned long cod = 0;
    int vet[32];

    while (1) {
        while ((TA1CCTL1 & CCIFG) == 0);

        if ((TA1CCR1 > 13950) && (TA1CCR1 < 14600)) {
            TA1CCTL1 = CM_1 | SCS | CAP;

            while ((TA1CCTL1 & CCIFG) == 0);
            clear_flag_and_timer();

            capture_values(vet);
            decode_values(vet, &cod);
            execute_action(cod);
        }

        TA1CCTL1 = CM_2 | SCS | CAP;
    }
}

void clear_flag_and_timer(void) {
    TA1CTL |= TACLR;
    TA1CCTL1 &= ~CCIFG;
}

void capture_values(int *vet) {
    for (i = 0; i < 32; i++) {
        while ((TA1CCTL1 & CCIFG) == 0);
        vet[i] = TA1CCR1;
        clear_flag_and_timer();
    }
}

void decode_values(int *vet, volatile unsigned long *cod) {
    *cod = 0;
    for (i = 0; i < 32; i++) {
        if (vet[i] > 1120 && vet[i] < 1320) {
            vet[i] = 0;
        } else if (vet[i] > 2230 && vet[i] < 2470) {
            vet[i] = 1;
        }
    }

    for (i = 0; i < 32; i++) {
        *cod = *cod >> 1;
        if (vet[i] == 1) {
            *cod |= 0x80000000L;
        }
    }
}

void execute_action(volatile unsigned long cod) {
    volatile unsigned long x = cod >> 24;

    switch (x) {
        case 0xA5: TB0CCR1 = 1047 * 6; break;
        case 0xAD: TB0CCR1 = 1047 * 8;if (TA2CCR2 != 524) { TA2CCR2 -= 116; } break;
        case 0xB8: P1OUT ^= BIT0; break;
        case 0xB9: P1OUT &= ~BIT0; break;
        case 0xBA: P1OUT |= BIT0; break;
        case 0xBB: TB0CCR1 = 1047 * 10; break;
        case 0xBC: TB0CCR1 = (1047 * 10) - TB0CCR1; break;
        case 0xBF: TB0CCR1 = 1047 * 0; break;
        case 0xE3: TB0CCR1 = 1047 * 5; break;
        case 0xE6: TB0CCR1 = 1047 * 3; break;
        case 0xE7: TB0CCR1 = 1047 * 2;if (TA2CCR2 != 2621) { TA2CCR2 += 116; } break;
        case 0xE9: TB0CCR1 = 1047 * 0; break;
        case 0xEA: TB0CCR1 = 1047 * 9;TA2CCR2 = 1572; break;
        case 0xF2: TB0CCR1 = 1047 * 1; break;
        case 0xF6: TB0CCR1 = 1047 * 7;TA2CCR2 = 2621; break;
        case 0xF7: TB0CCR1 = 1047 * 4; break;
        case 0xF8: TA2CCR2 = 524; break;
    }
}

void TB_config(void) {
    TB0CTL = TBSSEL_2 | MC_1;
    TB0CCR0 = 10485;
    TB0CCTL1 = OUTMOD_6;
}

void TA_config(void) {
    TA1CTL = TASSEL_2 | MC_2;
    TA1CCTL1 = CM_2 | SCS | CAP;
    TA1CCR0 = 14051;

    P2DIR &= ~BIT0;
    P2SEL |= BIT0;

    TA2CTL = TASSEL_1 | MC_1;
    TA2CCR0 = 32767;
}

void key_map(void) {
    P4DIR |= BIT7;
    P4SEL |= BIT7;
    PMAPKEYID = 0x02D52;
    P4MAP7 = PM_TB0CCR1A;
}

void config_leds(void) {
    P1DIR |= BIT0;  // (Verm) P1.0 = saída
    P1OUT &= ~BIT0; // Verm apagado
    P4DIR |= BIT7;  // (Verde) P4.7 = saída
    P4OUT &= ~BIT7; // Verde apagado
}
