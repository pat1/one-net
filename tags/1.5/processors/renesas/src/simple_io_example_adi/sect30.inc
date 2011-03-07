;*******************************************************************************
;
; C Compiler for R8C/Tiny, M16C/60,30,20,10
; Copyright(C) 1999(2000-2004). Renesas Technology Corp.
; and Renesas Solutions Corp., All rights reserved.
;
;
; Written by T.Aoyama
;
; sect30.inc     : section definition
; This program is applicable when using the basic I/O library
;
; $Date: 2005/11/08 06:30:21 $
; $Revision: 1.31 $
;******************************************************************************

.if	__R8C__ != 1
;
;	for M16C/60,30,20,10 
;
;---------------------------------------------------------------
;
; 	Arrangement of section
;
;---------------------------------------------------------------
; Near RAM data area
;---------------------------------------------------------------
; SBDATA area
	.section	data_SE,DATA
	.org	400H
data_SE_top:

	.section	bss_SE,DATA,ALIGN
bss_SE_top:

	.section	data_SO,DATA
data_SO_top:

	.section	bss_SO,DATA
bss_SO_top:

; near RAM area
	.section	data_NE,DATA,ALIGN
data_NE_top:

	.section	bss_NE,DATA,ALIGN
bss_NE_top:

	.section	data_NO,DATA
data_NO_top:

	.section	bss_NO,DATA
bss_NO_top:

;---------------------------------------------------------------
; Stack area
;---------------------------------------------------------------
	.section	stack,DATA
	.blkb	STACKSIZE
stack_top:

	.blkb	ISTACKSIZE
istack_top:

;---------------------------------------------------------------
;	heap section
;---------------------------------------------------------------
	.section	heap,DATA
heap_top:
	.blkb	HEAPSIZE

;---------------------------------------------------------------
; Near ROM data area
;---------------------------------------------------------------
	.section	rom_NE,ROMDATA,ALIGN
rom_NE_top:

	.section	rom_NO,ROMDATA
rom_NO_top:

;---------------------------------------------------------------
; Far RAM data area
;---------------------------------------------------------------
	.section	data_FE,DATA
	.org		10000H
data_FE_top:

	.section	bss_FE,DATA,ALIGN
bss_FE_top:
 
	.section	data_FO,DATA
data_FO_top:
 
	.section	bss_FO,DATA
bss_FO_top:


;---------------------------------------------------------------
; Far ROM data area
;---------------------------------------------------------------
	.section	rom_FE,ROMDATA
	.org		0F0000H
rom_FE_top:

	.section	rom_FO,ROMDATA
rom_FO_top:

;---------------------------------------------------------------
; Initial data of 'data' section
;---------------------------------------------------------------
	.section	data_SEI,ROMDATA
data_SEI_top:

	.section	data_SOI,ROMDATA
data_SOI_top:

	.section	data_NEI,ROMDATA
data_NEI_top:
 
	.section	data_NOI,ROMDATA
data_NOI_top:
 
	.section	data_FEI,ROMDATA
data_FEI_top:
 
	.section	data_FOI,ROMDATA
data_FOI_top:

;---------------------------------------------------------------
; Switch Table Section
;---------------------------------------------------------------
	.section        switch_table,ROMDATA
switch_table_top:

 
;---------------------------------------------------------------
; code area
;---------------------------------------------------------------

	.section	program

	.section	interrupt
	;.org	;must be set internal ROM area
	.section	program_S

.if	__MVT__==0
;---------------------------------------------------------------
; variable vector section
;---------------------------------------------------------------
	.section	vector,ROMDATA	; variable vector table
	.org	VECTOR_ADR 

.if	M60TYPE == 1
	.lword	dummy_int		; vector 0 (BRK)	
	.lword	dummy_int		; vector 1
	.lword	dummy_int		; vector 2
	.lword	dummy_int		; vector 3
	.lword	dummy_int		; vector 4
	.lword	dummy_int		; vector 5
	.lword	dummy_int		; vector 6
	.lword	dummy_int		; vector 7
	.lword	dummy_int		; vector 8
	.lword	dummy_int		; vector 9
	.lword	dummy_int		; vector 10
	.lword	dummy_int		; DMA0 (for user) (vector 11)
	.lword	dummy_int 		; DMA1 2 (for user) (vector 12)
	.lword	dummy_int		; input key (for user) (vector 13)
	.lword	dummy_int		; AD Convert (for user) (vector 14)
	.lword	dummy_int		; vector 15
	.lword	dummy_int		; vector 16
	.lword	dummy_int		; uart0 trance (for user) (vector 17)
	.lword	dummy_int		; uart0 receive (for user) (vector 18)
	.lword	dummy_int		; uart1 trance (for user) (vector 19)
	.lword	dummy_int		; uart1 receive (for user) (vector 20)
	.lword	dummy_int		; TIMER A0 (for user) (vector 21)
	.lword	dummy_int		; TIMER A1 (for user) (vector 22)
	.lword	dummy_int		; TIMER A2 (for user) (vector 23)
	.lword	dummy_int		; TIMER A3 (for user) (vector 24)
	.lword  dummy_int		; TIMER A4 (for user) (vector 25)
	.lword	dummy_int		; TIMER B0 (for user) (vector 26)
	.lword	dummy_int		; TIMER B1 (for user) (vector 27)
	.lword	dummy_int		; TIMER B2 (for user) (vector 28)
	.lword	dummy_int		; INT0 (for user) (vector 29)
	.lword	dummy_int		; INT1 (for user) (vector 30)
	.lword	dummy_int		; INT2 (for user) (vector 31)
.else
	.lword	dummy_int		; BRK	(vector 0)
	.lword	dummy_int		; 	(vector 1)
	.lword	dummy_int		; 	(vector 2)
	.lword	dummy_int		; 	(vector 3)
	.lword	dummy_int		; int3(for user)(vector 4)
	.lword	dummy_int		; timerB5(for user)(vector 5)
	.lword	dummy_int		; timerB4(for user)(vector 6)
	.lword	dummy_int		; timerB3(for user)(vector 7)
	.lword	dummy_int		; si/o4 /int5(for user)(vector 8)
	.lword	dummy_int		; si/o3 /int4(for user)(vector 9)
	.lword	dummy_int		; Bus collision detection(for user)(v10)
	.lword	dummy_int		; DMA0(for user)(vector 11)
	.lword	dummy_int		; DMA1(for user)(vector 12)
	.lword	dummy_int		; Key input interrupt(for user)(vect 13)
	.lword	dummy_int		; A-D(for user)(vector 14)
	.lword	dummy_int		; uart2 transmit(for user)(vector 15)
	.lword	dummy_int		; uart2 receive(for user)(vector 16)
	.lword	dummy_int		; uart0 transmit(for user)(vector 17)
	.lword	dummy_int		; uart0 receive(for user)(vector 18)
	.lword	dummy_int		; uart1 transmit(for user)(vector 19)
	.lword	dummy_int		; uart1 receive(for user)(vector 20)
	.lword	dummy_int		; timer A0(for user)(vector 21)
	.lword	dummy_int		; timer A1(for user)(vector 22)
	.lword	dummy_int		; timer A2(for user)(vector 23)
	.lword	dummy_int		; timer A3(for user)(vector 24)
	.lword	dummy_int		; timer A4(for user)(vector 25)
	.lword	dummy_int		; timer B0(for user)(vector 26)
	.lword	dummy_int		; timer B1(for user)(vector 27)
	.lword	dummy_int		; timer B2(for user)(vector 28)
	.lword	dummy_int		; int0 (for user)(vector 29)
	.lword	dummy_int		; int1 (for user)(vector 30)
	.lword	dummy_int		; int2 (for user)(vector 31)
.endif
	.lword	dummy_int		; vector 32 (for user or MR30)
	.lword	dummy_int		; vector 33 (for user or MR30)
	.lword	dummy_int		; vector 34 (for user or MR30)
	.lword	dummy_int		; vector 35 (for user or MR30)
	.lword	dummy_int		; vector 36 (for user or MR30)
	.lword	dummy_int		; vector 37 (for user or MR30)
	.lword	dummy_int		; vector 38 (for user or MR30)
	.lword	dummy_int		; vector 39 (for user or MR30)
	.lword	dummy_int		; vector 40 (for user or MR30)
	.lword	dummy_int		; vector 41 (for user or MR30)
	.lword	dummy_int		; vector 42 (for user or MR30)
	.lword	dummy_int		; vector 43 (for user or MR30)	
	.lword  dummy_int		; vector 44 (for user or MR30)
	.lword	dummy_int		; vector 45 (for user or MR30)
	.lword	dummy_int		; vector 46 (for user or MR30)
	.lword	dummy_int 		; vector 47 (for user or MR30)
	.lword	dummy_int 		; vector 48
	.lword	dummy_int 		; vector 49
	.lword	dummy_int 		; vector 50
	.lword	dummy_int 		; vector 51
	.lword	dummy_int 		; vector 52
	.lword	dummy_int 		; vector 53
	.lword	dummy_int 		; vector 54
	.lword	dummy_int 		; vector 55
	.lword	dummy_int 		; vector 56
	.lword	dummy_int 		; vector 57
	.lword	dummy_int 		; vector 58
	.lword	dummy_int 		; vector 59
	.lword	dummy_int 		; vector 60 
	.lword	dummy_int 		; vector 61
	.lword	dummy_int 		; vector 62
	.lword	dummy_int 		; vector 63
.else ; __MVT__

	.section	__NC_rvector,ROMDATA
	.org		VECTOR_ADR	

.endif ; __MVT__
	

.if	__MST__ == 0
;===============================================================
; fixed vector section
;---------------------------------------------------------------
.if	 SVECTOR_ADR < 0fffdcH 
	.section	svector,ROMDATA		; specialpage vector table
	.org		SVECTOR_ADR
.endif
;===============================================================
; special page defination
;---------------------------------------------------------------
;	macro is defined in ncrt0.a30
;	Format: SPECIAL number
;
;---------------------------------------------------------------
;	SPECIAL 255
;	SPECIAL 254
;	SPECIAL 253
;	SPECIAL 252
;	SPECIAL 251
;	SPECIAL 250
;	SPECIAL 249
;	SPECIAL 248
;	SPECIAL 247
;	SPECIAL 246
;	SPECIAL 245
;	SPECIAL 244
;	SPECIAL 243
;	SPECIAL 242
;	SPECIAL 241
;	SPECIAL 240
;	SPECIAL 239
;	SPECIAL 238
;	SPECIAL 237
;	SPECIAL 236
;	SPECIAL 235
;	SPECIAL 234
;	SPECIAL 233
;	SPECIAL 232
;	SPECIAL 231
;	SPECIAL 230
;	SPECIAL 229
;	SPECIAL 228
;	SPECIAL 227
;	SPECIAL 226
;	SPECIAL 225
;	SPECIAL 224
;	SPECIAL 223
;	SPECIAL 222
;	SPECIAL 221
;	SPECIAL 220
;	SPECIAL 219
;	SPECIAL 218
;	SPECIAL 217
;	SPECIAL 216
;	SPECIAL 215
;	SPECIAL 214
;	SPECIAL 213
;	SPECIAL 212
;	SPECIAL 211
;	SPECIAL 210
;	SPECIAL 209
;	SPECIAL 208
;	SPECIAL 207
;	SPECIAL 206
;	SPECIAL 205
;	SPECIAL 204
;	SPECIAL 203
;	SPECIAL 202
;	SPECIAL 201
;	SPECIAL 200
;	SPECIAL 199
;	SPECIAL 198
;	SPECIAL 197
;	SPECIAL 196
;	SPECIAL 195
;	SPECIAL 194
;	SPECIAL 193
;	SPECIAL 192
;	SPECIAL 191
;	SPECIAL 190
;	SPECIAL 189
;	SPECIAL 188
;	SPECIAL 187
;	SPECIAL 186
;	SPECIAL 185
;	SPECIAL 184
;	SPECIAL 183
;	SPECIAL 182
;	SPECIAL 181
;	SPECIAL 180
;	SPECIAL 179
;	SPECIAL 178
;	SPECIAL 177
;	SPECIAL 176
;	SPECIAL 175
;	SPECIAL 174
;	SPECIAL 173
;	SPECIAL 172
;	SPECIAL 171
;	SPECIAL 170
;	SPECIAL 169
;	SPECIAL 168
;	SPECIAL 167
;	SPECIAL 166
;	SPECIAL 165
;	SPECIAL 164
;	SPECIAL 163
;	SPECIAL 162
;	SPECIAL 161
;	SPECIAL 160
;	SPECIAL 159
;	SPECIAL 158
;	SPECIAL 157
;	SPECIAL 156
;	SPECIAL 155
;	SPECIAL 154
;	SPECIAL 153
;	SPECIAL 152
;	SPECIAL 151
;	SPECIAL 150
;	SPECIAL 149
;	SPECIAL 148
;	SPECIAL 147
;	SPECIAL 146
;	SPECIAL 145
;	SPECIAL 144
;	SPECIAL 143
;	SPECIAL 142
;	SPECIAL 141
;	SPECIAL 140
;	SPECIAL 139
;	SPECIAL 138
;	SPECIAL 137
;	SPECIAL 136
;	SPECIAL 135
;	SPECIAL 134
;	SPECIAL 133
;	SPECIAL 132
;	SPECIAL 131
;	SPECIAL 130
;	SPECIAL 129
;	SPECIAL 128
;	SPECIAL 127
;	SPECIAL 126
;	SPECIAL 125
;	SPECIAL 124
;	SPECIAL 123
;	SPECIAL 122
;	SPECIAL 121
;	SPECIAL 120
;	SPECIAL 119
;	SPECIAL 118
;	SPECIAL 117
;	SPECIAL 116
;	SPECIAL 115
;	SPECIAL 114
;	SPECIAL 113
;	SPECIAL 112
;	SPECIAL 111
;	SPECIAL 110
;	SPECIAL 109
;	SPECIAL 108
;	SPECIAL 107
;	SPECIAL 106
;	SPECIAL 105
;	SPECIAL 104
;	SPECIAL 103
;	SPECIAL 102
;	SPECIAL 101
;	SPECIAL 100
;	SPECIAL 99
;	SPECIAL 98
;	SPECIAL 97
;	SPECIAL 96
;	SPECIAL 95
;	SPECIAL 94
;	SPECIAL 93
;	SPECIAL 92
;	SPECIAL 91
;	SPECIAL 90
;	SPECIAL 89
;	SPECIAL 88
;	SPECIAL 87
;	SPECIAL 86
;	SPECIAL 85
;	SPECIAL 84
;	SPECIAL 83
;	SPECIAL 82
;	SPECIAL 81
;	SPECIAL 80
;	SPECIAL 79
;	SPECIAL 78
;	SPECIAL 77
;	SPECIAL 76
;	SPECIAL 75
;	SPECIAL 74
;	SPECIAL 73
;	SPECIAL 72
;	SPECIAL 71
;	SPECIAL 70
;	SPECIAL 69
;	SPECIAL 68
;	SPECIAL 67
;	SPECIAL 66
;	SPECIAL 65
;	SPECIAL 64
;	SPECIAL 63
;	SPECIAL 62
;	SPECIAL 61
;	SPECIAL 60
;	SPECIAL 59
;	SPECIAL 58
;	SPECIAL 57
;	SPECIAL 56
;	SPECIAL 55
;	SPECIAL 54
;	SPECIAL 53
;	SPECIAL 52
;	SPECIAL 51
;	SPECIAL 50
;	SPECIAL 49
;	SPECIAL 48
;	SPECIAL 47
;	SPECIAL 46
;	SPECIAL 45
;	SPECIAL 44
;	SPECIAL 43
;	SPECIAL 42
;	SPECIAL 41
;	SPECIAL 40
;	SPECIAL 39
;	SPECIAL 38
;	SPECIAL 37
;	SPECIAL 36
;	SPECIAL 35
;	SPECIAL 34
;	SPECIAL 33
;	SPECIAL 32
;	SPECIAL 31
;	SPECIAL 30
;	SPECIAL 29
;	SPECIAL 28
;	SPECIAL 27
;	SPECIAL 26
;	SPECIAL 25
;	SPECIAL 24
;	SPECIAL 23
;	SPECIAL 22
;	SPECIAL 21
;	SPECIAL 20
;	SPECIAL 19
;	SPECIAL 18
.else
.if	SVECTOR_ADR < 0fffdcH
	.section	__NC_svector,ROMDATA
	.org		SVECTOR_ADR
.endif
.endif ; __MST
;
;===============================================================
; fixed vector section
;---------------------------------------------------------------
	.section	fvector,ROMDATA
;	.org	0fffdcH
;UDI:
;	.lword	dummy_int
;OVER_FLOW:
;	.lword	dummy_int
;BRKI:
;	.lword	dummy_int
;ADDRESS_MATCH:
;	.lword	dummy_int
;SINGLE_STEP:
;	.lword	dummy_int
;WDT:
;	.lword	dummy_int
;DBC:
;	.lword	dummy_int
;NMI:
;	.lword	dummy_int
	.org	0ffffcH
RESET:
	.lword	start
	;
	; ID code check function
	; .id "CodeChk"
	;
	; ROM code protect control address
	; .protect 00H
	;

.else	; __R8C__

;
; for R8C/Tiny 
;
;---------------------------------------------------------------
;
; 	Arrangement of section
;
;---------------------------------------------------------------
; Near RAM data area
;---------------------------------------------------------------
; SBDATA area
	.section	data_SE,DATA
	.org	400H
data_SE_top:

	.section	bss_SE,DATA,ALIGN
bss_SE_top:

	.section	data_SO,DATA
data_SO_top:

	.section	bss_SO,DATA
bss_SO_top:

; near RAM area
	.section	data_NE,DATA,ALIGN
data_NE_top:

	.section	bss_NE,DATA,ALIGN
bss_NE_top:

	.section	data_NO,DATA
data_NO_top:

	.section	bss_NO,DATA
bss_NO_top:

;---------------------------------------------------------------
; Stack area
;---------------------------------------------------------------
	.section	stack,DATA,ALIGN
	.blkb	STACKSIZE
stack_top:

	.blkb	ISTACKSIZE
istack_top:

;---------------------------------------------------------------
;	heap section
;---------------------------------------------------------------
	.section	heap,DATA
heap_top:
	.blkb	HEAPSIZE

;---------------------------------------------------------------
; Near ROM data area
;---------------------------------------------------------------
	.section	rom_NE,ROMDATA
	.org		0C000H		
rom_NE_top:

	.section	rom_NO,ROMDATA
rom_NO_top:

;---------------------------------------------------------------
; Initial data of 'data' section
;---------------------------------------------------------------
	.section	data_SEI,ROMDATA,ALIGN
data_SEI_top:

	.section	data_SOI,ROMDATA
data_SOI_top:

	.section	data_NEI,ROMDATA,ALIGN
data_NEI_top:
 
	.section	data_NOI,ROMDATA
data_NOI_top:

;---------------------------------------------------------------
; Switch Table Section
;---------------------------------------------------------------
	.section        switch_table,ROMDATA
switch_table_top:

;---------------------------------------------------------------
; code area
;---------------------------------------------------------------

	.section	program,CODE,ALIGN

	.section	interrupt,CODE,ALIGN

.if	__MVT__ == 0 
;---------------------------------------------------------------
; variable vector section
;---------------------------------------------------------------
	.section	vector,ROMDATA	; variable vector table
	.org	VECTOR_ADR 

	.lword	dummy_int		; vector 0
	.lword	dummy_int		; vector 1
	.lword	dummy_int		; vector 2
	.lword	dummy_int		; vector 3
	.lword	dummy_int		; vector 4
	.lword	dummy_int		; vector 5
	.lword	dummy_int		; vector 6
	.lword	dummy_int		; vector 7
	.lword	dummy_int		; vector 8
	.lword	dummy_int		; vector 9
	.lword	dummy_int		; vector 10
	.lword	dummy_int		; vector 11
	.lword	dummy_int		; vector 12
	.lword	dummy_int		; vector 13
	.lword	dummy_int		; vector 14
	.lword	dummy_int		; vector 15
	.lword	dummy_int		; vector 16
    .lword  dummy_int       ; vector 17
    .lword  dummy_int       ; vector 18
	.lword	dummy_int		; vector 19
	.lword	dummy_int		; vector 20
	.lword	dummy_int		; vector 21

    .glb _tick_timer_isr    ; Tick Timer X ISR
	.lword	_tick_timer_isr ; vector 22

	.lword	dummy_int		; vector 23

    .glb _tx_bit_isr        ; RF Trasmit bit ISR
    .lword _tx_bit_isr      ; vector 24
    
	.glb _dataclk_isr       ; DATACLK ISR
	.lword	_dataclk_isr    ; vector 25

	.lword	dummy_int		; vector 26
	.lword	dummy_int		; vector 27
	.lword	dummy_int		; vector 28
	.lword	dummy_int		; vector 29
	.lword	dummy_int		; vector 30
	.lword	dummy_int		; vector 31
	.lword	dummy_int		; vector 32 
	.lword	dummy_int		; vector 33 
	.lword	dummy_int		; vector 34 
	.lword	dummy_int		; vector 35 
	.lword	dummy_int		; vector 36 
	.lword	dummy_int		; vector 37 
	.lword	dummy_int		; vector 38 
	.lword	dummy_int		; vector 39 
	.lword	dummy_int		; vector 40 
	.lword	dummy_int		; vector 41 
	.lword	dummy_int		; vector 42 
	.lword	dummy_int		; vector 43 
	.lword  dummy_int		; vector 44 
	.lword	dummy_int		; vector 45 
	.lword	dummy_int		; vector 46 
	.lword	dummy_int 		; vector 47 
	.lword	dummy_int 		; vector 48
	.lword	dummy_int 		; vector 49
	.lword	dummy_int 		; vector 50
	.lword	dummy_int 		; vector 51
	.lword	dummy_int 		; vector 52
	.lword	dummy_int 		; vector 53
	.lword	dummy_int 		; vector 54
	.lword	dummy_int 		; vector 55
	.lword	dummy_int 		; vector 56
	.lword	dummy_int 		; vector 57
	.lword	dummy_int 		; vector 58
	.lword	dummy_int 		; vector 59
	.lword	dummy_int 		; vector 60 
	.lword	dummy_int 		; vector 61
	.lword	dummy_int 		; vector 62
	.lword	dummy_int 		; vector 63
.else ; __MVT__
	.section	__NC_rvector,ROMDATA
	.org		VECTOR_ADR
.endif

;===============================================================
; fixed vector section
;---------------------------------------------------------------
	.section	fvector,ROMDATA		; fixed vector table
;	.org	0ffdcH
;UDI:
;	.lword	dummy_int
;OVER_FLOW:
;	.lword	dummy_int
;BRKI:
;	.lword	dummy_int
;ADDRESS_MATCH:
;	.lword	dummy_int
;SINGLE_STEP:
;	.lword	dummy_int
;WDT:
;	.lword	dummy_int
;RESERVED:
;	.lword	dummy_int
;RESERVED:
;	.lword	dummy_int
	.org	0fffcH
RESET:
	.lword	start
	;
	; ID code check function
	; .id "CodeChk"
	;
	; option function select register
	;
	.ofsreg	0FFH

.endif	; __R8C

;----------------------------------------------------------------
; far ROM data area
;----------------------------------------------------------------
;	
;	.section	rom_FE,ROMDATA
;	.org		10000H
;
;	.section	rom_FO,ROMDATA
; 
;	.section	data_FEI,ROMDATA,ALIGN
;data_FEI_top:
;
;	.section	data_FOI,ROMDATA
;data_FOI_top:
;
;****************************************************************************
;
;	C Compiler for R8C/Tiny, M16C/60,30,20,10
; Copyright(C) 1999(2000-2004). Renesas Technology Corp.
; and Renesas Solutions Corp., All rights reserved.
;
;****************************************************************************

