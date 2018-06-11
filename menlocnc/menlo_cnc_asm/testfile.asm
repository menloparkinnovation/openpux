
;
; Assembly language source file for menlo_cnc machine controller.
;
; Instruction sequences are in blocks that are executed
; in parallel on the hardware across each axis. The instruction
; block does not complete until all axis commands complete.
;
; A single line can represent multiple axis opcodes.
;
; Or a begin/end block can be used.
;
; White space is skipped.
;
; An axis target can only be referenced once per block.
;
; block begin/end must be balaned.
;
; There is no branching or loops. 
;
; format:
;
; axis, opcode, arg0, arg1, arg2
;
; valid axis:
;
;  X, Y, Z
;  A, B, C
;  U, V, W
;
;  All other single letter names are reserved.
;
; Number representation:
;
;   Only unsigned 32 bit numbers are allowed.
;
;   0x100 - hex value
;   100   - decimal value
;

; 
; header:
;  - Required
;  - Must be before any config or executable lines.
; 
; Since it is dangerous to run the wrong program on the wrong
; machine the menlo_cnc target machine and configuation
; is explicitly specified and compiled into any resulting
; binary files. It is expected that any program that runs a compiled
; binary validates it against the machine configuration either
; through auto detection, command parameters, or both.
; 
; menlo_cnc product 1, major version 1, minor version 0
; 
; Product 1 is the basic 4 axis CNC controller.
; 
; major version represents a breaking API change.
; 
; minor version represents new features, but generally
; forward compatible.
; 
INFO, HEADER, 1, 1, 0
	
; 
;  config
;  - Required
;  - Must be after header and before any executable lines.
; 
; configuration of target machine.
; 
; number of axis
; 
;  options0
; 
;  options1
; 
INFO, CONFIG, 4, 0, 0

; implied begin/end block commands
X,CW,100,16,4, Y,CCW,50,8,2, Z,DWELL,75,12,0, A,NOP,0,0,0
X,CW,100,16,4, Y,CCW,50,8,2, Z,DWELL,75,12,0, A,NOP,0,0,0
	
; explicit begin/end block command on a single line
begin,X,CW,100,16,4, Y,CCW,50,8,2, Z,DWELL,75,12,0, A,NOP,0,0,0,end

; multiple line begin/end block commands.
begin
  X,CW,0x100,16,4
  Y,CCW,50,8,2
  Z,DWELL,75,12,0
  A,NOP,0,0,0
end

begin
  X,CW,0x100,16,4
  Y,CCW,50,8,2
  Z,DWELL,75,12,0
  A,NOP,0,0,0
end

begin
  X, CW, 0x100, 16, 4
  Y, CCW,   50,  8, 2
  Z, DWELL, 75, 12, 0
  A, NOP,    0,  0, 0
end


