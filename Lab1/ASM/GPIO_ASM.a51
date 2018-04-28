$NOMOD51 ; Ask Keil not to define 8051 registers
$INCLUDE (C8051F020.INC) ; SFR definitions for the C8051F020
;-----------------------------------------------------------------------------
; Reset and Interrupt Vectors
cseg at 000h ; Define an absolute code segment located at 0x00
ljmp Reset ; On reset, jump to label "Reset"
;-----------------------------------------------------------------------------
; Code segment "Main"
Main segment CODE ; The assembler chooses the location of the code segment
rseg Main ; Switch to this code segment
using 0 ; Specify register bank
Reset:
	clr EA ; Disable all interrupts
	mov WDTCN, #0DEh ; Disable the watchdog
	mov WDTCN, #0ADh ;
	anl OSCICN, #014h ; Disable missing clock detector
	; and set internal osc at 2 MHz as the clock source
	mov XBR0, #000h ; Set and enable the crossbar
	mov XBR1, #000h ;
	mov XBR2, #040h ;
	orl P1MDOUT, #040h ; Set P1.6 to push-pull
MainLoop:
	jnb P3.7, LedOn
	clr P1.6
	jmp MainLoop
LedOn:
	setb P1.6
	jmp MainLoop
;-----------------------------------------------------------------------------
; End of file
end