;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer


;----------------------------------------------------------------------------
; Main loop here
;----------------------------------------------------------------------------
NUM			.equ 3999 ;Indicar número a ser convertido
;
 	mov 	#NUM,r5 ;R5 = número a ser convertido
 	mov 	#RESP,r6 ;R6 = ponteiro para escrever a resposta
 	call 	#ALG_ROM ;chamar subrotina
 	jmp 	$ ;travar execução
 	nop 	;exigido pelo montador
;
ALG_ROM:
		cmp 	#1000,r5	;
		jl		E900
		sub 	#1000,r5
		mov.b   #0x4D,0(r6)
		add		#1,r6
		jmp		ALG_ROM
E900:
		cmp		#900,r5
		jl		E500
		sub		#900,r5
		mov.b	#0x43,0(r6)
		add		#1,r6
		mov.b	#0x4D,0(r6)
		add		#1,r6
		jmp		ALG_ROM
E500:
		cmp		#500,r5
		jl		E400
		sub		#500,r5
		mov.b	#0x44,0(r6)
		add		#1,r6
		jmp		ALG_ROM
E400:
		cmp		#400,r5
		jl		E100
		sub		#500,r5
		mov.b	#0x43,0(r6)
		add		#1,r6
		mov.b	#0x44,0(r6)
		add		#1,r6
		jmp		ALG_ROM
E100:
		cmp		#100,r5
		jl		E90
		sub		#100,r5
		mov.b	#0x43,0(r6)
		add		#1,r6
		jmp		ALG_ROM
E90:
		cmp		#90,r5
		jl		E50
		sub		#90,r5
		mov.b	#0x58,0(r6)
		add		#1,r6
		mov.b	#0x43,0(r6)
		add		#1,r6
		jmp		ALG_ROM
E50:
		cmp		#50,r5
		jl		E10
		sub		#50,r5
		mov.b	#0x4C,0(r6)
		add		#1,r6
		jmp		ALG_ROM
E10:
		cmp		#10,r5
		jl		E9
		sub		#10,r5
		mov.b	#0x58,0(r6)
		add		#1,r6
		jmp		ALG_ROM
E9:
		cmp		#9,r5
		jl		E5
		sub		#9,r5
		mov.b	#0x49,0(r6)
		add		#1,r6
		mov.b	#0x58,0(r6)
		add		#1,r6
		jmp		ALG_ROM
E5:
		cmp		#5,r5
		jl		E4
		sub		#5,r5
		mov.b	#0x56,0(r6)
		add		#1,r6
		jmp		ALG_ROM
E4:
		cmp		#4,r5
		jl		E1
		sub		#4,r5
		mov.b	#0x49,0(r6)
		add		#1,r6
		mov.b	#0x56,0(r6)
		add		#1,r6
		jmp		ALG_ROM
E1:
		cmp		#0,r5
		jeq		fim
		mov.b	#0x49,0(r6)
		add		#1,r6
		sub		#1,r5
		jmp		E1

fim:
	mov.b	#0x00,0(r6)
	ret

 .data
; Local para armazenar a resposta (RESP = 0x2400)
RESP: .byte "RRRRRRRRRRRRRRRRRR",0

                                            

;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack
            
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            
