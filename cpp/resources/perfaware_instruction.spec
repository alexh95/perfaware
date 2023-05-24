This file contains the instructions as presented in the manual.

Data transfer

$MOV
Register/memory to/from register
+100010 d w | mod reg r/m |
Immediate to register/memory
+1100011 implicit:1 d w | mod 000 r/m | data | data-w |
Immediate to register
+1011 implicit:1 d w reg | data | data-w |
Memory to accumulator
+1010000 implicit:1 d w | implicit:0 mod implicit:0 reg implicit:6 r/m |
Accumulator to memory
+1010001 implicit:0 d w | implicit:0 mod implicit:0 reg implicit:6 r/m |

+100011 d 0 | mod 0 SR r/m |
 10001110 | mod 0 SR r/m |
 10001100 | mod 0 SR r/m |

$PUSH
Register/memory
+11111111 | mod 110 r/m |
Register
+01010 reg |
Segment register
+000 SR 110 |

$POP
Register/memory
+10001111 | mod 000 r/m |
Register
+01011 reg |
Segment register
+000 SR 111 |

$XCHG
Register/memory with register
+1000011 w | mod reg r/m |
Register with accumulator
+10010 reg |

$IN
Fixed port
+1110010 w | data |
Variable port
+1110110 w |

$OUT
Fixed port
+1110011 w | data |
Variable port
+1110111 w |

$XLAT
Translate byte to AL
+11010111 |

$LEA
Load EA to register
+10001101 | mod reg r/m |

$LDS
Load pointer to DS
+11000101 | mod reg r/m |

$LED
Load pointer to ES
+11000100 | mod reg r/m |

$LAHF
Load AH with flags
+10011111 |

$SAHF
Store AH into flags
+10011110 |

$PUSHF
Push flags
+10011100 |

$POPF
Pop flags
+10011101 |

$ADD
Reg/memory with register to either
+000000 d w | mod reg r/m |
Immediate to register/memory
+100000 implicit:1 d s w | mod 000 r/m | data | data-sw |
Immediate to accumulator
+0000010 implicit:1 d w implicit:0 reg | data | data-w |

$ADC
Reg/memory with register to either
+000100 d w | mod reg r/m |
Immediate to register/memory
+100000 implicit:1 d s w | mod 010 r/m | data | data-sw |
Immediate to accumulator
+0001010 implicit:1 d w implicit:0 reg | data | data-w |

$INC
Register/memory
+1111111 implicit:1 d w | mod 000 r/m |
Register
+01000 implicit:1 d reg |

$AAA
Adjust for add
+00110111 |

$DAA
Decimal adjist for add
+00100111 |

$SUB
Reg/memory with register to either
+001010 d w | mod reg r/m |
Immediate to register/memory
+100000 implicit:1 d s w | mod 101 r/m | data | data-sw |
Immediate to accumulator
+0010110 implicit:1 d w implicit:0 reg | data | data-w |

$SBB
Reg/memory with register to either
+000110 d w | mod reg r/m |
Immediate to register/memory
+100000 implicit:1 d s w | mod 011 r/m | data | data-sw |
Immediate to accumulator
+0001110 implicit:1 d w implicit:0 reg | data | data-w |

$DEC
Register/memory
+1111111 implicit:1 d w | mod 001 r/m |
Register
+01001 implicit:1 d reg |

$NEG
Change sign
+1111011 w | mod 011 r/m |

$CMP
Reg/memory with register to either
+001110 d w | mod reg r/m |
Immediate to register/memory
+100000 implicit:1 d s w | mod 111 r/m | data | data-sw |
Immediate to accumulator
+0011110 implicit:1 d w implicit:0 reg | data | data-w |

$AAS
ASCII adjust for subtract
+00111111 |

$DAS
Decimal adjust for subtract
+00101111 |
