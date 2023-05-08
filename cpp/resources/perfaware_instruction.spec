This file contains the instructions as presented in the manual.

Data transfer

$MOV
+100010 d w | mod reg r/m
+110000 1 w | mod 000 r/m
+1011 w reg | data | data_w_1
+1010000 w | addr-lo | addr-hi
+101000 1 w | addr-lo | addr-hi
+100011d 0 | mod 0 SR r/m
 1 0 0 0 1 1 1 0 | mod 0 SR r/m
 1 0 0 0 1 1 0 0 | mod 0 SR r/m

$PUSH
+11111111 | mod 1 1 0 r/m
+01010 reg
+000 SR 1 1 0

$POP
+10001111 | mod 0 0 0 r/m
+01011 reg
+000 SR 01 1 1

$XCHG
+1000011 w | mod reg r/m
+10010 reg

$IN
+1110010 w | data-8
+111011 0 w
