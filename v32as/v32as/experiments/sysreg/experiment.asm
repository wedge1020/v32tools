; experiment.asm: shows the outcome of the IP/IR/IV experiments on screen.
;
; Sequence of full-screen colors:
;     blue            start
;     (black, then green or red) x 6      tests 0-5 from payload.asm: green = pass
;     (black, then cyan or magenta)       ROM probe: cyan    = ROM write ignored (read-only)
;                                                    magenta = ROM was patched
;     white           done
; The raw results are also left in RAM words 0x800-0x806 (1 = pass / 1 = ROM was patched).

_start:
  ; start marker
  MOV R0, 0x0000FFFF
  MOV R1, 30
  CALL _flash

  ; copy the payload (assembled for 0x1000) into RAM and run it from there
  MOV SR, _payload
  MOV DR, 0x1000
  MOV CR, @PAYLOAD_WORDS@
  MOVS
  CALL 0x1000

  ; show tests 0-5
  MOV R4, 0x800
  MOV R5, 6
_show_loop:
  MOV R0, 0x000000FF
  MOV R1, 20
  CALL _flash
  MOV R3, [R4]
  MOV R0, 0xFF0000FF        ; red
  JF R3, _show
  MOV R0, 0x00FF00FF        ; green
_show:
  MOV R1, 45
  CALL _flash
  IADD R4, 1
  ISUB R5, 1
  MOV R6, R5
  IGT R6, 0
  JT R6, _show_loop

  ; ROM probe: try to patch an immediate that lives in cartridge ROM
  MOV R10, 0
  MOV R9, 222
  MOV IV, R9
  MOV R10, 111              ; 111 if the ROM write was ignored, 222 if it took effect
  IEQ R10, 222
  MOV [0x806], R10
  MOV R0, 0x000000FF
  MOV R1, 20
  CALL _flash
  MOV R3, [0x806]
  MOV R0, 0x00FFFFFF        ; cyan: ROM is read-only
  JF R3, _probe
  MOV R0, 0xFF00FFFF        ; magenta: ROM got patched
_probe:
  MOV R1, 45
  CALL _flash

  MOV R0, 0xFFFFFFFF
  MOV R1, 60
  CALL _flash
  HLT

; R0 = color, R1 = frames
_flash:
  OUT GPU_ClearColor, R0
  OUT GPU_Command, GPUCommand_ClearScreen
_flash_wait:
  WAIT
  ISUB R1, 1
  MOV R2, R1
  IGT R2, 0
  JT R2, _flash_wait
  RET

_payload:
  datafile "payload.dat"
