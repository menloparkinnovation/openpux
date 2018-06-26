
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
; minor version represents new features, but generally backward
; compatible with same major version.
; 
; Meaning newer minor versions can run older minor version binaries
; if they share the same major version.
; 
	
begin
  ;;    Engine Version
  ;; 	        Major, Minor, Update
  INFO_X, HEADER,     1,     0,      0
	
  ;;   Axis Count, Capabilities0, Capabilities1
  INFO_Y, CONFIG, 4,    0x00000000,    0x00000000
	
  ;; Machine specific, Passed Through in binary
  INFO_Z, CONFIG, 0, 0, 0
	
  ;; Machine specific, Passed Through in binary
  INFO_A, CONFIG, 0, 0, 0
end

; implied begin/end block commands
X,CW,100hz,16,4, Y,CCW,50hz,8,2, Z,DWELL,75hz,12,0, A,NOP,0,0,0
X,CW,100hz,16,4, Y,CCW,50hz,8,2, Z,DWELL,75hz,12,0, A,NOP,0,0,0
	
; explicit begin/end block command on a single line
begin,X,CW,100hz,16,4, Y,CCW,50hz,8,2, Z,DWELL,75hz,12,0, A,NOP,0,0,0,end

; multiple line begin/end block commands.
begin
  X,CW,0xAmhz,16,4
  Y,CCW,5mhz,8,2
  Z,DWELL,7.5mhz,12,0
  A,NOP,0,0,0
end

begin
  X,CW,0x100hz,16,4
  Y,CCW,50hz,8,2
  Z,DWELL,75hz,12,0
  A,NOP,0,0,0
end

begin
  X, CW, 0x100khz, 16, 4
  Y, CCW,   50khz,  8, 2
  Z, DWELL, 75khz, 12, 0
  A, NOP,    0,  0, 0
end

; 
; These demonstrate the suffix formats
; 
; 1hz - one Hertz
; 1khz - 10**3 Hertz
; 1mhz - 10**6 Hertz
; 
; 1s  - 1 second
; 1ms - 10**-3 second
; 1us - 10**-6 second
; 1ns - 10**-9 second
; 
; 1k - 10**3 10**3 units
; 1m - 10**6 10**6 units
; 1g - 10**6 10**9 units
; 
; 100clocks - 100 clocks at hardware clock rate
; 
; 50percent - 50% of baseline cycle such as pulse width during period.
; 

begin
  X, CW, 1ms, 1k, 22us
  Y, CCW, 50khz, 8m, 50us
  Z, DWELL, 75ms, 1g, 20ns
  A, NOP,    0,  0, 0
end

begin
  X, CW, 0x100ms, 0x16k, 0xaus
  Y, CCW,   5.0mhz,  8m, 20ns
  Z, DWELL, 75ms, 12k, 100ns
  A, NOP,    0,  0, 0
end


