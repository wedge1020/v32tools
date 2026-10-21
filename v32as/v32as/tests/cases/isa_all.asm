; every instruction in every operand form (generated)
_start:
  HLT
  WAIT
  RET
  MOVS
  SETS
; ---- one register
  PUSH R0
  PUSH R1
  PUSH R4
  PUSH R10
  PUSH CR
  PUSH SR
  PUSH DR
  PUSH BP
  PUSH SP
  PUSH R11
  PUSH R15
  POP R0
  POP R1
  POP R4
  POP R10
  POP CR
  POP SR
  POP DR
  POP BP
  POP SP
  POP R11
  POP R15
  CMPS R0
  CMPS R1
  CMPS R4
  CMPS R10
  CMPS CR
  CMPS SR
  CMPS DR
  CMPS BP
  CMPS SP
  CMPS R11
  CMPS R15
  CIF R0
  CIF R1
  CIF R4
  CIF R10
  CIF CR
  CIF SR
  CIF DR
  CIF BP
  CIF SP
  CIF R11
  CIF R15
  CFI R0
  CFI R1
  CFI R4
  CFI R10
  CFI CR
  CFI SR
  CFI DR
  CFI BP
  CFI SP
  CFI R11
  CFI R15
  CIB R0
  CIB R1
  CIB R4
  CIB R10
  CIB CR
  CIB SR
  CIB DR
  CIB BP
  CIB SP
  CIB R11
  CIB R15
  CFB R0
  CFB R1
  CFB R4
  CFB R10
  CFB CR
  CFB SR
  CFB DR
  CFB BP
  CFB SP
  CFB R11
  CFB R15
  NOT R0
  NOT R1
  NOT R4
  NOT R10
  NOT CR
  NOT SR
  NOT DR
  NOT BP
  NOT SP
  NOT R11
  NOT R15
  BNOT R0
  BNOT R1
  BNOT R4
  BNOT R10
  BNOT CR
  BNOT SR
  BNOT DR
  BNOT BP
  BNOT SP
  BNOT R11
  BNOT R15
  ISGN R0
  ISGN R1
  ISGN R4
  ISGN R10
  ISGN CR
  ISGN SR
  ISGN DR
  ISGN BP
  ISGN SP
  ISGN R11
  ISGN R15
  IABS R0
  IABS R1
  IABS R4
  IABS R10
  IABS CR
  IABS SR
  IABS DR
  IABS BP
  IABS SP
  IABS R11
  IABS R15
  FSGN R0
  FSGN R1
  FSGN R4
  FSGN R10
  FSGN CR
  FSGN SR
  FSGN DR
  FSGN BP
  FSGN SP
  FSGN R11
  FSGN R15
  FABS R0
  FABS R1
  FABS R4
  FABS R10
  FABS CR
  FABS SR
  FABS DR
  FABS BP
  FABS SP
  FABS R11
  FABS R15
  FLR R0
  FLR R1
  FLR R4
  FLR R10
  FLR CR
  FLR SR
  FLR DR
  FLR BP
  FLR SP
  FLR R11
  FLR R15
  CEIL R0
  CEIL R1
  CEIL R4
  CEIL R10
  CEIL CR
  CEIL SR
  CEIL DR
  CEIL BP
  CEIL SP
  CEIL R11
  CEIL R15
  ROUND R0
  ROUND R1
  ROUND R4
  ROUND R10
  ROUND CR
  ROUND SR
  ROUND DR
  ROUND BP
  ROUND SP
  ROUND R11
  ROUND R15
  SIN R0
  SIN R1
  SIN R4
  SIN R10
  SIN CR
  SIN SR
  SIN DR
  SIN BP
  SIN SP
  SIN R11
  SIN R15
  ACOS R0
  ACOS R1
  ACOS R4
  ACOS R10
  ACOS CR
  ACOS SR
  ACOS DR
  ACOS BP
  ACOS SP
  ACOS R11
  ACOS R15
  LOG R0
  LOG R1
  LOG R4
  LOG R10
  LOG CR
  LOG SR
  LOG DR
  LOG BP
  LOG SP
  LOG R11
  LOG R15
; ---- register, register|integer
  IEQ R0, R1
  IEQ R1, CR
  IEQ R4, BP
  IEQ R10, R15
  IEQ CR, R4
  IEQ SR, SR
  IEQ DR, SP
  IEQ BP, R0
  IEQ SP, R10
  IEQ R11, DR
  IEQ R15, R11
  IEQ R0, 0
  IEQ R1, 1
  IEQ R4, 7
  IEQ R10, 255
  IEQ CR, 65536
  IEQ SR, 0x10
  IEQ DR, 0xFF
  IEQ BP, 0xFFFFFFFF
  IEQ SP, 0x80000000
  IEQ R11, -1
  IEQ R15, -128
  IEQ R0, 'A'
  IEQ R1, '\n'
  IEQ R4, true
  IEQ R10, false
  IEQ CR, 2147483647
  INE R0, R1
  INE R1, CR
  INE R4, BP
  INE R10, R15
  INE CR, R4
  INE SR, SR
  INE DR, SP
  INE BP, R0
  INE SP, R10
  INE R11, DR
  INE R15, R11
  INE R0, 0
  INE R1, 1
  INE R4, 7
  INE R10, 255
  INE CR, 65536
  INE SR, 0x10
  INE DR, 0xFF
  INE BP, 0xFFFFFFFF
  INE SP, 0x80000000
  INE R11, -1
  INE R15, -128
  INE R0, 'A'
  INE R1, '\n'
  INE R4, true
  INE R10, false
  INE CR, 2147483647
  IGT R0, R1
  IGT R1, CR
  IGT R4, BP
  IGT R10, R15
  IGT CR, R4
  IGT SR, SR
  IGT DR, SP
  IGT BP, R0
  IGT SP, R10
  IGT R11, DR
  IGT R15, R11
  IGT R0, 0
  IGT R1, 1
  IGT R4, 7
  IGT R10, 255
  IGT CR, 65536
  IGT SR, 0x10
  IGT DR, 0xFF
  IGT BP, 0xFFFFFFFF
  IGT SP, 0x80000000
  IGT R11, -1
  IGT R15, -128
  IGT R0, 'A'
  IGT R1, '\n'
  IGT R4, true
  IGT R10, false
  IGT CR, 2147483647
  IGE R0, R1
  IGE R1, CR
  IGE R4, BP
  IGE R10, R15
  IGE CR, R4
  IGE SR, SR
  IGE DR, SP
  IGE BP, R0
  IGE SP, R10
  IGE R11, DR
  IGE R15, R11
  IGE R0, 0
  IGE R1, 1
  IGE R4, 7
  IGE R10, 255
  IGE CR, 65536
  IGE SR, 0x10
  IGE DR, 0xFF
  IGE BP, 0xFFFFFFFF
  IGE SP, 0x80000000
  IGE R11, -1
  IGE R15, -128
  IGE R0, 'A'
  IGE R1, '\n'
  IGE R4, true
  IGE R10, false
  IGE CR, 2147483647
  ILT R0, R1
  ILT R1, CR
  ILT R4, BP
  ILT R10, R15
  ILT CR, R4
  ILT SR, SR
  ILT DR, SP
  ILT BP, R0
  ILT SP, R10
  ILT R11, DR
  ILT R15, R11
  ILT R0, 0
  ILT R1, 1
  ILT R4, 7
  ILT R10, 255
  ILT CR, 65536
  ILT SR, 0x10
  ILT DR, 0xFF
  ILT BP, 0xFFFFFFFF
  ILT SP, 0x80000000
  ILT R11, -1
  ILT R15, -128
  ILT R0, 'A'
  ILT R1, '\n'
  ILT R4, true
  ILT R10, false
  ILT CR, 2147483647
  ILE R0, R1
  ILE R1, CR
  ILE R4, BP
  ILE R10, R15
  ILE CR, R4
  ILE SR, SR
  ILE DR, SP
  ILE BP, R0
  ILE SP, R10
  ILE R11, DR
  ILE R15, R11
  ILE R0, 0
  ILE R1, 1
  ILE R4, 7
  ILE R10, 255
  ILE CR, 65536
  ILE SR, 0x10
  ILE DR, 0xFF
  ILE BP, 0xFFFFFFFF
  ILE SP, 0x80000000
  ILE R11, -1
  ILE R15, -128
  ILE R0, 'A'
  ILE R1, '\n'
  ILE R4, true
  ILE R10, false
  ILE CR, 2147483647
  AND R0, R1
  AND R1, CR
  AND R4, BP
  AND R10, R15
  AND CR, R4
  AND SR, SR
  AND DR, SP
  AND BP, R0
  AND SP, R10
  AND R11, DR
  AND R15, R11
  AND R0, 0
  AND R1, 1
  AND R4, 7
  AND R10, 255
  AND CR, 65536
  AND SR, 0x10
  AND DR, 0xFF
  AND BP, 0xFFFFFFFF
  AND SP, 0x80000000
  AND R11, -1
  AND R15, -128
  AND R0, 'A'
  AND R1, '\n'
  AND R4, true
  AND R10, false
  AND CR, 2147483647
  OR R0, R1
  OR R1, CR
  OR R4, BP
  OR R10, R15
  OR CR, R4
  OR SR, SR
  OR DR, SP
  OR BP, R0
  OR SP, R10
  OR R11, DR
  OR R15, R11
  OR R0, 0
  OR R1, 1
  OR R4, 7
  OR R10, 255
  OR CR, 65536
  OR SR, 0x10
  OR DR, 0xFF
  OR BP, 0xFFFFFFFF
  OR SP, 0x80000000
  OR R11, -1
  OR R15, -128
  OR R0, 'A'
  OR R1, '\n'
  OR R4, true
  OR R10, false
  OR CR, 2147483647
  XOR R0, R1
  XOR R1, CR
  XOR R4, BP
  XOR R10, R15
  XOR CR, R4
  XOR SR, SR
  XOR DR, SP
  XOR BP, R0
  XOR SP, R10
  XOR R11, DR
  XOR R15, R11
  XOR R0, 0
  XOR R1, 1
  XOR R4, 7
  XOR R10, 255
  XOR CR, 65536
  XOR SR, 0x10
  XOR DR, 0xFF
  XOR BP, 0xFFFFFFFF
  XOR SP, 0x80000000
  XOR R11, -1
  XOR R15, -128
  XOR R0, 'A'
  XOR R1, '\n'
  XOR R4, true
  XOR R10, false
  XOR CR, 2147483647
  SHL R0, R1
  SHL R1, CR
  SHL R4, BP
  SHL R10, R15
  SHL CR, R4
  SHL SR, SR
  SHL DR, SP
  SHL BP, R0
  SHL SP, R10
  SHL R11, DR
  SHL R15, R11
  SHL R0, 0
  SHL R1, 1
  SHL R4, 7
  SHL R10, 255
  SHL CR, 65536
  SHL SR, 0x10
  SHL DR, 0xFF
  SHL BP, 0xFFFFFFFF
  SHL SP, 0x80000000
  SHL R11, -1
  SHL R15, -128
  SHL R0, 'A'
  SHL R1, '\n'
  SHL R4, true
  SHL R10, false
  SHL CR, 2147483647
  IADD R0, R1
  IADD R1, CR
  IADD R4, BP
  IADD R10, R15
  IADD CR, R4
  IADD SR, SR
  IADD DR, SP
  IADD BP, R0
  IADD SP, R10
  IADD R11, DR
  IADD R15, R11
  IADD R0, 0
  IADD R1, 1
  IADD R4, 7
  IADD R10, 255
  IADD CR, 65536
  IADD SR, 0x10
  IADD DR, 0xFF
  IADD BP, 0xFFFFFFFF
  IADD SP, 0x80000000
  IADD R11, -1
  IADD R15, -128
  IADD R0, 'A'
  IADD R1, '\n'
  IADD R4, true
  IADD R10, false
  IADD CR, 2147483647
  ISUB R0, R1
  ISUB R1, CR
  ISUB R4, BP
  ISUB R10, R15
  ISUB CR, R4
  ISUB SR, SR
  ISUB DR, SP
  ISUB BP, R0
  ISUB SP, R10
  ISUB R11, DR
  ISUB R15, R11
  ISUB R0, 0
  ISUB R1, 1
  ISUB R4, 7
  ISUB R10, 255
  ISUB CR, 65536
  ISUB SR, 0x10
  ISUB DR, 0xFF
  ISUB BP, 0xFFFFFFFF
  ISUB SP, 0x80000000
  ISUB R11, -1
  ISUB R15, -128
  ISUB R0, 'A'
  ISUB R1, '\n'
  ISUB R4, true
  ISUB R10, false
  ISUB CR, 2147483647
  IMUL R0, R1
  IMUL R1, CR
  IMUL R4, BP
  IMUL R10, R15
  IMUL CR, R4
  IMUL SR, SR
  IMUL DR, SP
  IMUL BP, R0
  IMUL SP, R10
  IMUL R11, DR
  IMUL R15, R11
  IMUL R0, 0
  IMUL R1, 1
  IMUL R4, 7
  IMUL R10, 255
  IMUL CR, 65536
  IMUL SR, 0x10
  IMUL DR, 0xFF
  IMUL BP, 0xFFFFFFFF
  IMUL SP, 0x80000000
  IMUL R11, -1
  IMUL R15, -128
  IMUL R0, 'A'
  IMUL R1, '\n'
  IMUL R4, true
  IMUL R10, false
  IMUL CR, 2147483647
  IDIV R0, R1
  IDIV R1, CR
  IDIV R4, BP
  IDIV R10, R15
  IDIV CR, R4
  IDIV SR, SR
  IDIV DR, SP
  IDIV BP, R0
  IDIV SP, R10
  IDIV R11, DR
  IDIV R15, R11
  IDIV R0, 0
  IDIV R1, 1
  IDIV R4, 7
  IDIV R10, 255
  IDIV CR, 65536
  IDIV SR, 0x10
  IDIV DR, 0xFF
  IDIV BP, 0xFFFFFFFF
  IDIV SP, 0x80000000
  IDIV R11, -1
  IDIV R15, -128
  IDIV R0, 'A'
  IDIV R1, '\n'
  IDIV R4, true
  IDIV R10, false
  IDIV CR, 2147483647
  IMOD R0, R1
  IMOD R1, CR
  IMOD R4, BP
  IMOD R10, R15
  IMOD CR, R4
  IMOD SR, SR
  IMOD DR, SP
  IMOD BP, R0
  IMOD SP, R10
  IMOD R11, DR
  IMOD R15, R11
  IMOD R0, 0
  IMOD R1, 1
  IMOD R4, 7
  IMOD R10, 255
  IMOD CR, 65536
  IMOD SR, 0x10
  IMOD DR, 0xFF
  IMOD BP, 0xFFFFFFFF
  IMOD SP, 0x80000000
  IMOD R11, -1
  IMOD R15, -128
  IMOD R0, 'A'
  IMOD R1, '\n'
  IMOD R4, true
  IMOD R10, false
  IMOD CR, 2147483647
  IMIN R0, R1
  IMIN R1, CR
  IMIN R4, BP
  IMIN R10, R15
  IMIN CR, R4
  IMIN SR, SR
  IMIN DR, SP
  IMIN BP, R0
  IMIN SP, R10
  IMIN R11, DR
  IMIN R15, R11
  IMIN R0, 0
  IMIN R1, 1
  IMIN R4, 7
  IMIN R10, 255
  IMIN CR, 65536
  IMIN SR, 0x10
  IMIN DR, 0xFF
  IMIN BP, 0xFFFFFFFF
  IMIN SP, 0x80000000
  IMIN R11, -1
  IMIN R15, -128
  IMIN R0, 'A'
  IMIN R1, '\n'
  IMIN R4, true
  IMIN R10, false
  IMIN CR, 2147483647
  IMAX R0, R1
  IMAX R1, CR
  IMAX R4, BP
  IMAX R10, R15
  IMAX CR, R4
  IMAX SR, SR
  IMAX DR, SP
  IMAX BP, R0
  IMAX SP, R10
  IMAX R11, DR
  IMAX R15, R11
  IMAX R0, 0
  IMAX R1, 1
  IMAX R4, 7
  IMAX R10, 255
  IMAX CR, 65536
  IMAX SR, 0x10
  IMAX DR, 0xFF
  IMAX BP, 0xFFFFFFFF
  IMAX SP, 0x80000000
  IMAX R11, -1
  IMAX R15, -128
  IMAX R0, 'A'
  IMAX R1, '\n'
  IMAX R4, true
  IMAX R10, false
  IMAX CR, 2147483647
; ---- register, register|float (integer literals convert)
  FEQ R0, R1
  FEQ R1, CR
  FEQ R4, BP
  FEQ R10, R15
  FEQ CR, R4
  FEQ SR, SR
  FEQ DR, SP
  FEQ BP, R0
  FEQ SP, R10
  FEQ R11, DR
  FEQ R15, R11
  FEQ R0, 0.0
  FEQ R1, 1.5
  FEQ R4, -2.25
  FEQ R10, 3.14159
  FEQ CR, 100.0
  FEQ SR, 0.001
  FEQ DR, 0
  FEQ BP, 1
  FEQ SP, -3
  FEQ R11, 100
  FEQ R15, 'A'
  FEQ R0, true
  FNE R0, R1
  FNE R1, CR
  FNE R4, BP
  FNE R10, R15
  FNE CR, R4
  FNE SR, SR
  FNE DR, SP
  FNE BP, R0
  FNE SP, R10
  FNE R11, DR
  FNE R15, R11
  FNE R0, 0.0
  FNE R1, 1.5
  FNE R4, -2.25
  FNE R10, 3.14159
  FNE CR, 100.0
  FNE SR, 0.001
  FNE DR, 0
  FNE BP, 1
  FNE SP, -3
  FNE R11, 100
  FNE R15, 'A'
  FNE R0, true
  FGT R0, R1
  FGT R1, CR
  FGT R4, BP
  FGT R10, R15
  FGT CR, R4
  FGT SR, SR
  FGT DR, SP
  FGT BP, R0
  FGT SP, R10
  FGT R11, DR
  FGT R15, R11
  FGT R0, 0.0
  FGT R1, 1.5
  FGT R4, -2.25
  FGT R10, 3.14159
  FGT CR, 100.0
  FGT SR, 0.001
  FGT DR, 0
  FGT BP, 1
  FGT SP, -3
  FGT R11, 100
  FGT R15, 'A'
  FGT R0, true
  FGE R0, R1
  FGE R1, CR
  FGE R4, BP
  FGE R10, R15
  FGE CR, R4
  FGE SR, SR
  FGE DR, SP
  FGE BP, R0
  FGE SP, R10
  FGE R11, DR
  FGE R15, R11
  FGE R0, 0.0
  FGE R1, 1.5
  FGE R4, -2.25
  FGE R10, 3.14159
  FGE CR, 100.0
  FGE SR, 0.001
  FGE DR, 0
  FGE BP, 1
  FGE SP, -3
  FGE R11, 100
  FGE R15, 'A'
  FGE R0, true
  FLT R0, R1
  FLT R1, CR
  FLT R4, BP
  FLT R10, R15
  FLT CR, R4
  FLT SR, SR
  FLT DR, SP
  FLT BP, R0
  FLT SP, R10
  FLT R11, DR
  FLT R15, R11
  FLT R0, 0.0
  FLT R1, 1.5
  FLT R4, -2.25
  FLT R10, 3.14159
  FLT CR, 100.0
  FLT SR, 0.001
  FLT DR, 0
  FLT BP, 1
  FLT SP, -3
  FLT R11, 100
  FLT R15, 'A'
  FLT R0, true
  FLE R0, R1
  FLE R1, CR
  FLE R4, BP
  FLE R10, R15
  FLE CR, R4
  FLE SR, SR
  FLE DR, SP
  FLE BP, R0
  FLE SP, R10
  FLE R11, DR
  FLE R15, R11
  FLE R0, 0.0
  FLE R1, 1.5
  FLE R4, -2.25
  FLE R10, 3.14159
  FLE CR, 100.0
  FLE SR, 0.001
  FLE DR, 0
  FLE BP, 1
  FLE SP, -3
  FLE R11, 100
  FLE R15, 'A'
  FLE R0, true
  FADD R0, R1
  FADD R1, CR
  FADD R4, BP
  FADD R10, R15
  FADD CR, R4
  FADD SR, SR
  FADD DR, SP
  FADD BP, R0
  FADD SP, R10
  FADD R11, DR
  FADD R15, R11
  FADD R0, 0.0
  FADD R1, 1.5
  FADD R4, -2.25
  FADD R10, 3.14159
  FADD CR, 100.0
  FADD SR, 0.001
  FADD DR, 0
  FADD BP, 1
  FADD SP, -3
  FADD R11, 100
  FADD R15, 'A'
  FADD R0, true
  FSUB R0, R1
  FSUB R1, CR
  FSUB R4, BP
  FSUB R10, R15
  FSUB CR, R4
  FSUB SR, SR
  FSUB DR, SP
  FSUB BP, R0
  FSUB SP, R10
  FSUB R11, DR
  FSUB R15, R11
  FSUB R0, 0.0
  FSUB R1, 1.5
  FSUB R4, -2.25
  FSUB R10, 3.14159
  FSUB CR, 100.0
  FSUB SR, 0.001
  FSUB DR, 0
  FSUB BP, 1
  FSUB SP, -3
  FSUB R11, 100
  FSUB R15, 'A'
  FSUB R0, true
  FMUL R0, R1
  FMUL R1, CR
  FMUL R4, BP
  FMUL R10, R15
  FMUL CR, R4
  FMUL SR, SR
  FMUL DR, SP
  FMUL BP, R0
  FMUL SP, R10
  FMUL R11, DR
  FMUL R15, R11
  FMUL R0, 0.0
  FMUL R1, 1.5
  FMUL R4, -2.25
  FMUL R10, 3.14159
  FMUL CR, 100.0
  FMUL SR, 0.001
  FMUL DR, 0
  FMUL BP, 1
  FMUL SP, -3
  FMUL R11, 100
  FMUL R15, 'A'
  FMUL R0, true
  FDIV R0, R1
  FDIV R1, CR
  FDIV R4, BP
  FDIV R10, R15
  FDIV CR, R4
  FDIV SR, SR
  FDIV DR, SP
  FDIV BP, R0
  FDIV SP, R10
  FDIV R11, DR
  FDIV R15, R11
  FDIV R0, 0.0
  FDIV R1, 1.5
  FDIV R4, -2.25
  FDIV R10, 3.14159
  FDIV CR, 100.0
  FDIV SR, 0.001
  FDIV DR, 0
  FDIV BP, 1
  FDIV SP, -3
  FDIV R11, 100
  FDIV R15, 'A'
  FDIV R0, true
  FMOD R0, R1
  FMOD R1, CR
  FMOD R4, BP
  FMOD R10, R15
  FMOD CR, R4
  FMOD SR, SR
  FMOD DR, SP
  FMOD BP, R0
  FMOD SP, R10
  FMOD R11, DR
  FMOD R15, R11
  FMOD R0, 0.0
  FMOD R1, 1.5
  FMOD R4, -2.25
  FMOD R10, 3.14159
  FMOD CR, 100.0
  FMOD SR, 0.001
  FMOD DR, 0
  FMOD BP, 1
  FMOD SP, -3
  FMOD R11, 100
  FMOD R15, 'A'
  FMOD R0, true
  FMIN R0, R1
  FMIN R1, CR
  FMIN R4, BP
  FMIN R10, R15
  FMIN CR, R4
  FMIN SR, SR
  FMIN DR, SP
  FMIN BP, R0
  FMIN SP, R10
  FMIN R11, DR
  FMIN R15, R11
  FMIN R0, 0.0
  FMIN R1, 1.5
  FMIN R4, -2.25
  FMIN R10, 3.14159
  FMIN CR, 100.0
  FMIN SR, 0.001
  FMIN DR, 0
  FMIN BP, 1
  FMIN SP, -3
  FMIN R11, 100
  FMIN R15, 'A'
  FMIN R0, true
  FMAX R0, R1
  FMAX R1, CR
  FMAX R4, BP
  FMAX R10, R15
  FMAX CR, R4
  FMAX SR, SR
  FMAX DR, SP
  FMAX BP, R0
  FMAX SP, R10
  FMAX R11, DR
  FMAX R15, R11
  FMAX R0, 0.0
  FMAX R1, 1.5
  FMAX R4, -2.25
  FMAX R10, 3.14159
  FMAX CR, 100.0
  FMAX SR, 0.001
  FMAX DR, 0
  FMAX BP, 1
  FMAX SP, -3
  FMAX R11, 100
  FMAX R15, 'A'
  FMAX R0, true
; ---- two registers only
  ATAN2 R0, R1
  ATAN2 R1, CR
  ATAN2 R4, BP
  ATAN2 R10, R15
  ATAN2 CR, R4
  ATAN2 SR, SR
  ATAN2 DR, SP
  ATAN2 BP, R0
  ATAN2 SP, R10
  ATAN2 R11, DR
  ATAN2 R15, R11
  POW R0, R1
  POW R1, CR
  POW R4, BP
  POW R10, R15
  POW CR, R4
  POW SR, SR
  POW DR, SP
  POW BP, R0
  POW SP, R10
  POW R11, DR
  POW R15, R11
; ---- jumps
  JMP R0
  JMP R1
  JMP R4
  JMP R10
  JMP CR
  JMP 0
  JMP 1234
  JMP 0x20000000
  JMP -1
  JMP _start
  JMP _later
  CALL R0
  CALL R1
  CALL R4
  CALL R10
  CALL CR
  CALL 0
  CALL 1234
  CALL 0x20000000
  CALL -1
  CALL _start
  CALL _later
  JT R0, R4
  JT R1, R10
  JT R4, CR
  JT R10, SR
  JT CR, R0
  JT SR, R1
  JT R3, 0
  JT R3, 77
  JT R3, 0x20000010
  JT R3, -1
  JT R2, _start
  JT R2, _later
  JF R0, R4
  JF R1, R10
  JF R4, CR
  JF R10, SR
  JF CR, R0
  JF SR, R1
  JF R3, 0
  JF R3, 77
  JF R3, 0x20000010
  JF R3, -1
  JF R2, _start
  JF R2, _later
; ---- MOV in all eight addressing modes
  MOV R0, 0
  MOV R0, 1
  MOV R0, 7
  MOV R0, 255
  MOV R0, 65536
  MOV R0, 0x10
  MOV R0, 0.0
  MOV R0, 1.5
  MOV R0, _start
  MOV R0, _later
  MOV R0, R10
  MOV R0, [R1]
  MOV R0, [0x1000]
  MOV R0, [_start]
  MOV R0, [_later]
  MOV R0, [R4+0]
  MOV R0, [R4+1]
  MOV R0, [R4+16]
  MOV R0, [R4-4]
  MOV R0, [R4-1]
  MOV R0, [R4+65535]
  MOV [0x2000], R0
  MOV [_later], R0
  MOV [R1], R0
  MOV [CR+0], R0
  MOV [CR+2], R0
  MOV [CR-8], R0
  MOV R1, 0
  MOV R1, 1
  MOV R1, 7
  MOV R1, 255
  MOV R1, 65536
  MOV R1, 0x10
  MOV R1, 0.0
  MOV R1, 1.5
  MOV R1, _start
  MOV R1, _later
  MOV R1, CR
  MOV R1, [R4]
  MOV R1, [0x1000]
  MOV R1, [_start]
  MOV R1, [_later]
  MOV R1, [R10+0]
  MOV R1, [R10+1]
  MOV R1, [R10+16]
  MOV R1, [R10-4]
  MOV R1, [R10-1]
  MOV R1, [R10+65535]
  MOV [0x2000], R1
  MOV [_later], R1
  MOV [R4], R1
  MOV [SR+0], R1
  MOV [SR+2], R1
  MOV [SR-8], R1
  MOV R4, 0
  MOV R4, 1
  MOV R4, 7
  MOV R4, 255
  MOV R4, 65536
  MOV R4, 0x10
  MOV R4, 0.0
  MOV R4, 1.5
  MOV R4, _start
  MOV R4, _later
  MOV R4, SR
  MOV R4, [R10]
  MOV R4, [0x1000]
  MOV R4, [_start]
  MOV R4, [_later]
  MOV R4, [CR+0]
  MOV R4, [CR+1]
  MOV R4, [CR+16]
  MOV R4, [CR-4]
  MOV R4, [CR-1]
  MOV R4, [CR+65535]
  MOV [0x2000], R4
  MOV [_later], R4
  MOV [R10], R4
  MOV [DR+0], R4
  MOV [DR+2], R4
  MOV [DR-8], R4
  MOV R10, 0
  MOV R10, 1
  MOV R10, 7
  MOV R10, 255
  MOV R10, 65536
  MOV R10, 0x10
  MOV R10, 0.0
  MOV R10, 1.5
  MOV R10, _start
  MOV R10, _later
  MOV R10, DR
  MOV R10, [CR]
  MOV R10, [0x1000]
  MOV R10, [_start]
  MOV R10, [_later]
  MOV R10, [SR+0]
  MOV R10, [SR+1]
  MOV R10, [SR+16]
  MOV R10, [SR-4]
  MOV R10, [SR-1]
  MOV R10, [SR+65535]
  MOV [0x2000], R10
  MOV [_later], R10
  MOV [CR], R10
  MOV [BP+0], R10
  MOV [BP+2], R10
  MOV [BP-8], R10
  MOV CR, 0
  MOV CR, 1
  MOV CR, 7
  MOV CR, 255
  MOV CR, 65536
  MOV CR, 0x10
  MOV CR, 0.0
  MOV CR, 1.5
  MOV CR, _start
  MOV CR, _later
  MOV CR, BP
  MOV CR, [SR]
  MOV CR, [0x1000]
  MOV CR, [_start]
  MOV CR, [_later]
  MOV CR, [DR+0]
  MOV CR, [DR+1]
  MOV CR, [DR+16]
  MOV CR, [DR-4]
  MOV CR, [DR-1]
  MOV CR, [DR+65535]
  MOV [0x2000], CR
  MOV [_later], CR
  MOV [SR], CR
  MOV [SP+0], CR
  MOV [SP+2], CR
  MOV [SP-8], CR
  MOV SR, 0
  MOV SR, 1
  MOV SR, 7
  MOV SR, 255
  MOV SR, 65536
  MOV SR, 0x10
  MOV SR, 0.0
  MOV SR, 1.5
  MOV SR, _start
  MOV SR, _later
  MOV SR, SP
  MOV SR, [DR]
  MOV SR, [0x1000]
  MOV SR, [_start]
  MOV SR, [_later]
  MOV SR, [BP+0]
  MOV SR, [BP+1]
  MOV SR, [BP+16]
  MOV SR, [BP-4]
  MOV SR, [BP-1]
  MOV SR, [BP+65535]
  MOV [0x2000], SR
  MOV [_later], SR
  MOV [DR], SR
  MOV [R11+0], SR
  MOV [R11+2], SR
  MOV [R11-8], SR
; ---- LEA
  LEA R0, [R1]
  LEA R0, [R4+8]
  LEA R0, [R10-8]
  LEA R1, [R4]
  LEA R1, [R10+8]
  LEA R1, [CR-8]
  LEA R4, [R10]
  LEA R4, [CR+8]
  LEA R4, [SR-8]
  LEA R10, [CR]
  LEA R10, [SR+8]
  LEA R10, [DR-8]
  LEA CR, [SR]
  LEA CR, [DR+8]
  LEA CR, [BP-8]
  LEA SR, [DR]
  LEA SR, [BP+8]
  LEA SR, [SP-8]
  LEA DR, [BP]
  LEA DR, [SP+8]
  LEA DR, [R11-8]
  LEA BP, [SP]
  LEA BP, [R11+8]
  LEA BP, [R15-8]
  LEA SP, [R11]
  LEA SP, [R15+8]
  LEA SP, [R0-8]
  LEA R11, [R15]
  LEA R11, [R0+8]
  LEA R11, [R1-8]
  LEA R15, [R0]
  LEA R15, [R1+8]
  LEA R15, [R4-8]
; ---- IN / OUT with every port
  IN R1, TIM_CurrentDate
  OUT TIM_CurrentDate, R2
  OUT TIM_CurrentDate, 5
  OUT TIM_CurrentDate, -1
  OUT TIM_CurrentDate, 0.5
  IN R1, TIM_CurrentTime
  OUT TIM_CurrentTime, R2
  OUT TIM_CurrentTime, 5
  OUT TIM_CurrentTime, -1
  OUT TIM_CurrentTime, 0.5
  IN R1, TIM_FrameCounter
  OUT TIM_FrameCounter, R2
  OUT TIM_FrameCounter, 5
  OUT TIM_FrameCounter, -1
  OUT TIM_FrameCounter, 0.5
  IN R1, TIM_CycleCounter
  OUT TIM_CycleCounter, R2
  OUT TIM_CycleCounter, 5
  OUT TIM_CycleCounter, -1
  OUT TIM_CycleCounter, 0.5
  IN R1, RNG_CurrentValue
  OUT RNG_CurrentValue, R2
  OUT RNG_CurrentValue, 5
  OUT RNG_CurrentValue, -1
  OUT RNG_CurrentValue, 0.5
  IN R1, GPU_Command
  OUT GPU_Command, R2
  OUT GPU_Command, 5
  OUT GPU_Command, -1
  OUT GPU_Command, 0.5
  IN R1, GPU_RemainingPixels
  OUT GPU_RemainingPixels, R2
  OUT GPU_RemainingPixels, 5
  OUT GPU_RemainingPixels, -1
  OUT GPU_RemainingPixels, 0.5
  IN R1, GPU_ClearColor
  OUT GPU_ClearColor, R2
  OUT GPU_ClearColor, 5
  OUT GPU_ClearColor, -1
  OUT GPU_ClearColor, 0.5
  IN R1, GPU_MultiplyColor
  OUT GPU_MultiplyColor, R2
  OUT GPU_MultiplyColor, 5
  OUT GPU_MultiplyColor, -1
  OUT GPU_MultiplyColor, 0.5
  IN R1, GPU_ActiveBlending
  OUT GPU_ActiveBlending, R2
  OUT GPU_ActiveBlending, 5
  OUT GPU_ActiveBlending, -1
  OUT GPU_ActiveBlending, 0.5
  IN R1, GPU_SelectedTexture
  OUT GPU_SelectedTexture, R2
  OUT GPU_SelectedTexture, 5
  OUT GPU_SelectedTexture, -1
  OUT GPU_SelectedTexture, 0.5
  IN R1, GPU_SelectedRegion
  OUT GPU_SelectedRegion, R2
  OUT GPU_SelectedRegion, 5
  OUT GPU_SelectedRegion, -1
  OUT GPU_SelectedRegion, 0.5
  IN R1, GPU_DrawingPointX
  OUT GPU_DrawingPointX, R2
  OUT GPU_DrawingPointX, 5
  OUT GPU_DrawingPointX, -1
  OUT GPU_DrawingPointX, 0.5
  IN R1, GPU_DrawingPointY
  OUT GPU_DrawingPointY, R2
  OUT GPU_DrawingPointY, 5
  OUT GPU_DrawingPointY, -1
  OUT GPU_DrawingPointY, 0.5
  IN R1, GPU_DrawingScaleX
  OUT GPU_DrawingScaleX, R2
  OUT GPU_DrawingScaleX, 5
  OUT GPU_DrawingScaleX, -1
  OUT GPU_DrawingScaleX, 0.5
  IN R1, GPU_DrawingScaleY
  OUT GPU_DrawingScaleY, R2
  OUT GPU_DrawingScaleY, 5
  OUT GPU_DrawingScaleY, -1
  OUT GPU_DrawingScaleY, 0.5
  IN R1, GPU_DrawingAngle
  OUT GPU_DrawingAngle, R2
  OUT GPU_DrawingAngle, 5
  OUT GPU_DrawingAngle, -1
  OUT GPU_DrawingAngle, 0.5
  IN R1, GPU_RegionMinX
  OUT GPU_RegionMinX, R2
  OUT GPU_RegionMinX, 5
  OUT GPU_RegionMinX, -1
  OUT GPU_RegionMinX, 0.5
  IN R1, GPU_RegionMinY
  OUT GPU_RegionMinY, R2
  OUT GPU_RegionMinY, 5
  OUT GPU_RegionMinY, -1
  OUT GPU_RegionMinY, 0.5
  IN R1, GPU_RegionMaxX
  OUT GPU_RegionMaxX, R2
  OUT GPU_RegionMaxX, 5
  OUT GPU_RegionMaxX, -1
  OUT GPU_RegionMaxX, 0.5
  IN R1, GPU_RegionMaxY
  OUT GPU_RegionMaxY, R2
  OUT GPU_RegionMaxY, 5
  OUT GPU_RegionMaxY, -1
  OUT GPU_RegionMaxY, 0.5
  IN R1, GPU_RegionHotspotX
  OUT GPU_RegionHotspotX, R2
  OUT GPU_RegionHotspotX, 5
  OUT GPU_RegionHotspotX, -1
  OUT GPU_RegionHotspotX, 0.5
  IN R1, GPU_RegionHotspotY
  OUT GPU_RegionHotspotY, R2
  OUT GPU_RegionHotspotY, 5
  OUT GPU_RegionHotspotY, -1
  OUT GPU_RegionHotspotY, 0.5
  IN R1, SPU_Command
  OUT SPU_Command, R2
  OUT SPU_Command, 5
  OUT SPU_Command, -1
  OUT SPU_Command, 0.5
  IN R1, SPU_GlobalVolume
  OUT SPU_GlobalVolume, R2
  OUT SPU_GlobalVolume, 5
  OUT SPU_GlobalVolume, -1
  OUT SPU_GlobalVolume, 0.5
  IN R1, SPU_SelectedSound
  OUT SPU_SelectedSound, R2
  OUT SPU_SelectedSound, 5
  OUT SPU_SelectedSound, -1
  OUT SPU_SelectedSound, 0.5
  IN R1, SPU_SelectedChannel
  OUT SPU_SelectedChannel, R2
  OUT SPU_SelectedChannel, 5
  OUT SPU_SelectedChannel, -1
  OUT SPU_SelectedChannel, 0.5
  IN R1, SPU_SoundLength
  OUT SPU_SoundLength, R2
  OUT SPU_SoundLength, 5
  OUT SPU_SoundLength, -1
  OUT SPU_SoundLength, 0.5
  IN R1, SPU_SoundPlayWithLoop
  OUT SPU_SoundPlayWithLoop, R2
  OUT SPU_SoundPlayWithLoop, 5
  OUT SPU_SoundPlayWithLoop, -1
  OUT SPU_SoundPlayWithLoop, 0.5
  IN R1, SPU_SoundLoopStart
  OUT SPU_SoundLoopStart, R2
  OUT SPU_SoundLoopStart, 5
  OUT SPU_SoundLoopStart, -1
  OUT SPU_SoundLoopStart, 0.5
  IN R1, SPU_SoundLoopEnd
  OUT SPU_SoundLoopEnd, R2
  OUT SPU_SoundLoopEnd, 5
  OUT SPU_SoundLoopEnd, -1
  OUT SPU_SoundLoopEnd, 0.5
  IN R1, SPU_ChannelState
  OUT SPU_ChannelState, R2
  OUT SPU_ChannelState, 5
  OUT SPU_ChannelState, -1
  OUT SPU_ChannelState, 0.5
  IN R1, SPU_ChannelAssignedSound
  OUT SPU_ChannelAssignedSound, R2
  OUT SPU_ChannelAssignedSound, 5
  OUT SPU_ChannelAssignedSound, -1
  OUT SPU_ChannelAssignedSound, 0.5
  IN R1, SPU_ChannelVolume
  OUT SPU_ChannelVolume, R2
  OUT SPU_ChannelVolume, 5
  OUT SPU_ChannelVolume, -1
  OUT SPU_ChannelVolume, 0.5
  IN R1, SPU_ChannelSpeed
  OUT SPU_ChannelSpeed, R2
  OUT SPU_ChannelSpeed, 5
  OUT SPU_ChannelSpeed, -1
  OUT SPU_ChannelSpeed, 0.5
  IN R1, SPU_ChannelLoopEnabled
  OUT SPU_ChannelLoopEnabled, R2
  OUT SPU_ChannelLoopEnabled, 5
  OUT SPU_ChannelLoopEnabled, -1
  OUT SPU_ChannelLoopEnabled, 0.5
  IN R1, SPU_ChannelPosition
  OUT SPU_ChannelPosition, R2
  OUT SPU_ChannelPosition, 5
  OUT SPU_ChannelPosition, -1
  OUT SPU_ChannelPosition, 0.5
  IN R1, INP_SelectedGamepad
  OUT INP_SelectedGamepad, R2
  OUT INP_SelectedGamepad, 5
  OUT INP_SelectedGamepad, -1
  OUT INP_SelectedGamepad, 0.5
  IN R1, INP_GamepadConnected
  OUT INP_GamepadConnected, R2
  OUT INP_GamepadConnected, 5
  OUT INP_GamepadConnected, -1
  OUT INP_GamepadConnected, 0.5
  IN R1, INP_GamepadLeft
  OUT INP_GamepadLeft, R2
  OUT INP_GamepadLeft, 5
  OUT INP_GamepadLeft, -1
  OUT INP_GamepadLeft, 0.5
  IN R1, INP_GamepadRight
  OUT INP_GamepadRight, R2
  OUT INP_GamepadRight, 5
  OUT INP_GamepadRight, -1
  OUT INP_GamepadRight, 0.5
  IN R1, INP_GamepadUp
  OUT INP_GamepadUp, R2
  OUT INP_GamepadUp, 5
  OUT INP_GamepadUp, -1
  OUT INP_GamepadUp, 0.5
  IN R1, INP_GamepadDown
  OUT INP_GamepadDown, R2
  OUT INP_GamepadDown, 5
  OUT INP_GamepadDown, -1
  OUT INP_GamepadDown, 0.5
  IN R1, INP_GamepadButtonStart
  OUT INP_GamepadButtonStart, R2
  OUT INP_GamepadButtonStart, 5
  OUT INP_GamepadButtonStart, -1
  OUT INP_GamepadButtonStart, 0.5
  IN R1, INP_GamepadButtonA
  OUT INP_GamepadButtonA, R2
  OUT INP_GamepadButtonA, 5
  OUT INP_GamepadButtonA, -1
  OUT INP_GamepadButtonA, 0.5
  IN R1, INP_GamepadButtonB
  OUT INP_GamepadButtonB, R2
  OUT INP_GamepadButtonB, 5
  OUT INP_GamepadButtonB, -1
  OUT INP_GamepadButtonB, 0.5
  IN R1, INP_GamepadButtonX
  OUT INP_GamepadButtonX, R2
  OUT INP_GamepadButtonX, 5
  OUT INP_GamepadButtonX, -1
  OUT INP_GamepadButtonX, 0.5
  IN R1, INP_GamepadButtonY
  OUT INP_GamepadButtonY, R2
  OUT INP_GamepadButtonY, 5
  OUT INP_GamepadButtonY, -1
  OUT INP_GamepadButtonY, 0.5
  IN R1, INP_GamepadButtonL
  OUT INP_GamepadButtonL, R2
  OUT INP_GamepadButtonL, 5
  OUT INP_GamepadButtonL, -1
  OUT INP_GamepadButtonL, 0.5
  IN R1, INP_GamepadButtonR
  OUT INP_GamepadButtonR, R2
  OUT INP_GamepadButtonR, 5
  OUT INP_GamepadButtonR, -1
  OUT INP_GamepadButtonR, 0.5
  IN R1, CAR_Connected
  OUT CAR_Connected, R2
  OUT CAR_Connected, 5
  OUT CAR_Connected, -1
  OUT CAR_Connected, 0.5
  IN R1, CAR_ProgramROMSize
  OUT CAR_ProgramROMSize, R2
  OUT CAR_ProgramROMSize, 5
  OUT CAR_ProgramROMSize, -1
  OUT CAR_ProgramROMSize, 0.5
  IN R1, CAR_NumberOfTextures
  OUT CAR_NumberOfTextures, R2
  OUT CAR_NumberOfTextures, 5
  OUT CAR_NumberOfTextures, -1
  OUT CAR_NumberOfTextures, 0.5
  IN R1, CAR_NumberOfSounds
  OUT CAR_NumberOfSounds, R2
  OUT CAR_NumberOfSounds, 5
  OUT CAR_NumberOfSounds, -1
  OUT CAR_NumberOfSounds, 0.5
  IN R1, MEM_Connected
  OUT MEM_Connected, R2
  OUT MEM_Connected, 5
  OUT MEM_Connected, -1
  OUT MEM_Connected, 0.5
  OUT GPU_Command, GPUCommand_ClearScreen
  OUT GPU_Command, GPUCommand_DrawRegion
  OUT GPU_Command, GPUCommand_DrawRegionZoomed
  OUT GPU_Command, GPUCommand_DrawRegionRotated
  OUT GPU_Command, GPUCommand_DrawRegionRotozoomed
  OUT GPU_Command, GPUBlendingMode_Alpha
  OUT GPU_Command, GPUBlendingMode_Add
  OUT GPU_Command, GPUBlendingMode_Subtract
  OUT GPU_Command, SPUCommand_PlaySelectedChannel
  OUT GPU_Command, SPUCommand_PauseSelectedChannel
  OUT GPU_Command, SPUCommand_StopSelectedChannel
  OUT GPU_Command, SPUCommand_PauseAllChannels
  OUT GPU_Command, SPUCommand_ResumeAllChannels
  OUT GPU_Command, SPUCommand_StopAllChannels
  OUT GPU_Command, SPUChannelState_Stopped
  OUT GPU_Command, SPUChannelState_Playing
_later:
  HLT
