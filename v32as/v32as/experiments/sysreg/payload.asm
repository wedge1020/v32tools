; payload.asm: the part of the experiment that runs from RAM.
;
; It is assembled with  v32asc --origin 0x1000  so that every address the assembler bakes
; in (labels, IP, the targets of MOV IR/IV writes) is the RAM address the code will really
; execute at.  The cartridge copies it to RAM address 0x1000 and calls it.
;
; Each test leaves 1 (pass) or 0 (fail) in RAM word 0x800 + n.

_payload_main:

; ---- 0: MOV Rn, IP  vs the return address CALL really pushes -----------------------
  MOV R1, IP                ; assembler constant = address of the CALL below
  CALL _t0_pop
_t0_pop:
  POP R2                    ; runtime: address of _t0_pop (the CALL's return address)
  ISUB R2, 2                ; back up over the CALL (2 words) to where R1 should point
  IEQ R2, R1
  MOV [0x800], R2

; ---- 1: MOV Rn, IR  vs the instruction word that is really in memory --------------
_t1_self:
  MOV R3, IR
  MOV R4, [_t1_self]
  IEQ R3, R4
  MOV [0x801], R3

; ---- 2: MOV IP, _label  must skip the poison instruction ----------------------------
  MOV R5, 0
  MOV IP, _t2_target
  MOV R5, 0xBAD             ; must NOT execute
_t2_target:
  IEQ R5, 0
  MOV [0x802], R5

; ---- 3: MOV IP, Rn ------------------------------------------------------------------
  MOV R6, _t3_target
  MOV R5, 0
  MOV IP, R6
  MOV R5, 0xBAD             ; must NOT execute
_t3_target:
  IEQ R5, 0
  MOV [0x803], R5

; ---- 4: MOV IR, Rn  replaces the NEXT instruction word -----------------------------
  MOV R7, 0
  MOV R8, 0
  MOV R9, 0x4F000000        ; encoding of  MOV R8, <imm>
  MOV IR, R9
  MOV R7, 111               ; becomes  MOV R8, 111  (R7 stays 0)
  IEQ R8, 111
  IEQ R7, 0
  AND R8, R7
  MOV [0x804], R8

; ---- 5: MOV IV, Rn  replaces the NEXT instruction's immediate -----------------------
  MOV R10, 0
  MOV R9, 222
  MOV IV, R9
  MOV R10, 111              ; becomes  MOV R10, 222
  IEQ R10, 222
  MOV [0x805], R10

  RET
