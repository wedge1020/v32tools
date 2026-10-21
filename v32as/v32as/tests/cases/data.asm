_start:
  MOV R0, _table
  MOV R1, [_table]
  CALL _func
  HLT
_func:
  RET
_table:
  integer 1, -2, 0x10, 65535
  float 1.5, -0.25, 3
  pointer _start, _func, _table
_msg:
  string "Hello\n\t\"q\"\\"
  string ""
  string "abc"
_file:
  datafile "data.dat"
_end:
  integer 7
