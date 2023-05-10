This file contains the instructions as presented in the manual.

Data transfer

$MOV
+100010 d w | mod reg r/m |
+1100001 w | mod 000 r/m |
+1011 w reg | data | data-w |
+1010000 w | addr-lo | addr-hi |
+1010001 w | addr-lo | addr-hi |
+100011 d 0 | mod 0 SR r/m |
 10001110 | mod 0 SR r/m |
 10001100 | mod 0 SR r/m |

$PUSH
+11111111 | mod 110 r/m |
+01010 reg |
+000 SR 1 1 0 |

$POP
+10001111 | mod 000 r/m |
+01011 reg |
+000 SR 111 |

$XCHG
+1000011 w | mod reg r/m |
+10010 reg |

$IN
+1110010 w | data |
+1110110 w |
