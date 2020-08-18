; Print.s
; Student names: Eralp Orkun and Stephen Do
; Last modification date: 4/9/2020
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
; Lab 7 requirement is for at least one local variable on the stack with symbolic binding
LCD_OutDec
digit EQU 0
	
	SUB SP, #8
	
	STR R0, [SP, #digit]  ;MOV R2, R0
	CMP R0, #10
	BHS Recursion
	ADD R0, #0x30
	
	PUSH{R5, LR}
	BL ST7735_OutChar
	POP{R5, LR}

    ADD SP, #8
	BX LR
Recursion
	MOV R1, #10
	UDIV R0, R1
	
	;STR R0, [SP, #digit]
	PUSH {LR, R5}
	BL LCD_OutDec
	POP {LR, R5}
	LDR R2, [SP, #digit]
	
	MOV R1, #10
	MOV R0, R2
	UDIV R0, R1
	MUL R0, R1
	SUB R0, R2, R0
	ADD R0, #0x30
	PUSH {LR, R5}
	BL ST7735_OutChar
    POP {LR, R5}

    ADD SP, #8
    BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.01, range 0.00 to 9.99
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.00 "
;       R0=3,    then output "0.03 "
;       R0=89,   then output "0.89 "
;       R0=123,  then output "1.23 "
;       R0=999,  then output "9.99 "
;       R0>999,  then output "*.** "
; Invariables: This function must not permanently modify registers R4 to R11
; Lab 7 requirement is for at least one local variable on the stack with symbolic binding
LCD_OutFix

digitOne EQU 8
digitTwo EQU 4
digitThree EQU 0		  	  
		   
     CMP R0, #1000
     BHS badInput	
	 
	 SUB SP, #16
	 
	 ;inital check for too large
	 
	 MOV R1, #10
	 UDIV R2,R0, R1
	 MUL R2, R2 , R1                  ; calculates least significant digit
	 SUB R2, R0, R2
	 STR R2, [SP, #digitThree]
	 
	 UDIV R0, R1                      ; N = N/10
	 
	 MOV R1, #10
	 UDIV R2,R0, R1
	 MUL R2, R1                  ; calculates 2nd significant digit
	 SUB R2, R0, R2
	 STR R2, [SP, #digitTwo]
	 
	 UDIV R0, R1                   ; N = N/10
	 
	 MOV R1, #10
	 UDIV R2,R0, R1
	 MUL R2, R1                  ; calculates 2nd significant digit
	 SUB R2, R0, R2
	 STR R2, [SP, #digitOne]
	 
	 UDIV R0, R1                   ; N = N/10
	 
	 LDR R0, [SP, #digitOne]   
	 ADD R0, #0X30
	 PUSH{R5, LR}
	 BL ST7735_OutChar
	 POP{R5, LR}
	 MOV R0, #0X2E
	 PUSH{R5, LR}
	 BL ST7735_OutChar
	 POP{R5, LR}
	 LDR R0, [SP, #digitTwo]
	 ADD R0, #0X30
	 PUSH{R5, LR}
	 BL ST7735_OutChar
	 POP{R5, LR}
	 LDR R0, [SP, #digitThree]   
	 ADD R0, #0X30
	 PUSH{R5, LR}
	 BL ST7735_OutChar
	 POP{R5, LR}
	 
	 ADD SP, #16
     BX   LR
	 
badInput
     MOV R0, #0X2A
	 PUSH{R5, LR}
	 BL ST7735_OutChar
	 POP{R5, LR}
	 MOV R0, #0X2E
	 PUSH{R5, LR}
	 BL ST7735_OutChar
	 POP{R5, LR}
	 MOV R0, #0X2A
	 PUSH{R5, LR}
	 BL ST7735_OutChar
	 POP{R5, LR}
	 MOV R0, #0X2A
	 PUSH{R5, LR}
	 BL ST7735_OutChar
	 POP{R5, LR}
	 
	 BX LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN          ; make sure the end of this section is aligned
     END            ; end of file
