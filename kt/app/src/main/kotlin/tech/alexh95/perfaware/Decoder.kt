package tech.alexh95.perfaware

import tech.alexh95.perfaware.Opcode.*
import java.io.File

class Bytes constructor(private val array: ByteArray, private var index: Int, private var startIndex: Int = 0) {
    fun hasNextByte(): Boolean = index < array.size
    fun nextByte(): Int = (array[index++].toUByte()).toInt()
    fun nextWord(): Int {
        val data0 = nextByte()
        val data1 = nextByte()
        return (data1 shl 8) or data0
    }
    fun nextData(word: Boolean): Int = if (word) nextWord() else nextByte()
    fun peekByte(): Int = (array[index].toUByte()).toInt()
    fun instructionStartAddress(): Int = startIndex
    fun advanceStartAddress() { startIndex = index }
}

typealias Instruction = Pair<Int?, String>
fun Instruction.baseAddress(): Int? = this.first
fun Instruction.line(): String = this.second
typealias InstructionList = ArrayList<Instruction>

fun InstructionList.push(bytes: Bytes, instruction: String) {
    this += Pair(bytes.instructionStartAddress(), instruction)
}

enum class Opcode(val code: Int, val mask: Int = 0b11111111, val codeByte1: Int = 0, val maskByte1: Int = 0) {
    // SEGMENT OVERRIDE PREFIX
    S_OVERRIDE( 0b00100110, 0b11100111),
//    ES(         0b00100110),
//    CS(         0b00101110),
//    SS(         0b00110110),
//    DS(         0b00111110),
    // DATA TRANSFER
    MOV_RM_R(   0b10001000, 0b11111100),
    MOV_IM_RM(  0b11000110, 0b11111110, 0b00000000, 0b00111000),
    MOV_IM_R(   0b10110000, 0b11110000),
    MOV_M_A(    0b10100000, 0b11111110),
    MOV_A_M(    0b10100010, 0b11111110),
//    MOV_RM_SR(  0b10001110),
//    MOV_SR_RM(  0b10001100),
    PUSH_RM(    0b11111111, 0b11111111, 0b00110000, 0b00111000),
    PUSH_R(     0b01010000, 0b11111000),
    PUSH_SR(    0b00000110, 0b11100111),
    POP_RM(     0b10001111, 0b11111111, 0b00000000, 0b00111000),
    POP_R(      0b01011000, 0b11111000),
    POP_SR(     0b00000111, 0b11100111),
    XCHG_RM_R(  0b10000110, 0b11111110),
    XCHG_R_A(   0b10010000, 0b11111000),
    IN_F(       0b11100100, 0b11111110),
    IN_V(       0b11101100, 0b11111110),
    OUT_F(      0b11100110, 0b11111110),
    OUT_V(      0b11101110, 0b11111110),
    XLAT(       0b11010111),
    LEA(        0b10001101),
    LDS(        0b11000101),
    LES(        0b11000100),
    LAHF(       0b10011111),
    SAHF(       0b10011110),
    PUSHF(      0b10011100),
    POPF(       0b10011101),
    // ARITHMETIC & LOGIC
    OP_RM_R(    0b00000000, 0b11000100),
    OP_IM_RM(   0b10000000, 0b11111100),
    OP_IM_A(    0b00000100, 0b11000110),
    OP_GR2_RM( 0b11111110, 0b11111110),
    INC_R(      0b01000000, 0b11111000),
    DEC_R(      0b01001000, 0b11111000),
    OP_GR1_RM(  0b11110110, 0b11111110),
    AAM(        0b11010100),
    AAD(        0b11010101),
    AAA(        0b00110111),
    DAA(        0b00100111),
    AAS(        0b00111111),
    DAS(        0b00101111),
    CBW(        0b10011000),
    CWD(        0b10011001),
    OP_SHIFT_RM(0b11010000, 0b11111100),
    AND_RM_R(   0b00100000, 0b11111100),
    AND_A(      0b00100100, 0b11111110),
    TEST_RM_R(  0b10000100, 0b11111100),
    TEST_A(     0b10101000, 0b11111110),
    // STRING MANIPULATION
    REP(        0b11110010, 0b11111110),
    MOVS(       0b10100100, 0b11111110),
    CMPS(       0b10100110, 0b11111110),
    SCAS(       0b10101110, 0b11111110),
    LODS(       0b10101100, 0b11111110),
    STOS(       0b10101010, 0b11111110),
    // CONTROL TRANSFER
    RET_S(      0b11000011),
    RET_SI(     0b11000010),
    INT_T(      0b11001101),
    INT_3(      0b11001100),
    INTO(       0b11001110),
    IRET(       0b11001111),
    JE(         0b01110100),
    JL(         0b01111100),
    JLE(        0b01111110),
    JB(         0b01110010),
    JBE(        0b01110110),
    JP(         0b01111010),
    JO(         0b01110000),
    JS(         0b01111000),
    JNE(        0b01110101),
    JNL(        0b01111101),
    JNLE(       0b01111111),
    JNB(        0b01110011),
    JNBE(       0b01110111),
    JNP(        0b01111011),
    JNO(        0b01110001),
    JNS(        0b01111001),
    LOOP(       0b11100010),
    LOOPZ(      0b11100001),
    LOOPNZ(     0b11100000),
    JCXZ(       0b11100011),
    // PROCESSOR CONTROL
    CLC(        0b11111000),
    CMC(        0b11110101),
    STC(        0b11111001),
    CLD(        0b11111100),
    STD(        0b11111101),
    CLI(        0b11111010),
    STI(        0b11111011),
    HLT(        0b11110100),
    WAIT(       0b10011011),
    LOCK(       0b11110000),
    ;

    companion object {
        fun decodeOperation(byte: Int, bytes: Bytes): Opcode {
            values().forEach {
                if (byte and it.mask == it.code) {
                    if (it.maskByte1 == 0 || bytes.peekByte() and it.maskByte1 == it.codeByte1) return it
                }
            }
            throw RuntimeException("Invalid Opcode ${byte.toString(2)}")
        }
    }

    override fun toString(): String = name.lowercase()
}

fun disassemble8086(byteArray: ByteArray): String {
    val result: InstructionList = arrayListOf()
    result += Pair(null, "bits 16")
    var previousRepZero = false
    var previousRep = false
    var previousLock = false
    var previousSr: SegmentRegister? = null
    var currentSr: SegmentRegister? = null
    val bytes = Bytes(byteArray, 0)
    while (bytes.hasNextByte()) {
        bytes.advanceStartAddress()
        val byte0 = bytes.nextByte()
        val opcode = Opcode.decodeOperation(byte0, bytes)
        when (opcode) {
            S_OVERRIDE -> { currentSr = SegmentRegister.decodeSR(byte0) }
            // DATA TRANSFER
            MOV_RM_R -> decodeInstructionRMToRFull(result, byte0, bytes, "mov")
            MOV_IM_RM -> decodeIMToRM(result, byte0, bytes)
            MOV_IM_R -> decodeMovImToR(result, byte0, bytes)
            MOV_M_A -> result.push(bytes, "mov ax, [${bytes.nextWord()}]")
            MOV_A_M -> result.push(bytes, "mov [${bytes.nextWord()}], ax")
            /*MOV_RM_SR -> {
                val byte1 = bytes.nextByte()
                val mode = Mode.decodeMode(byte1)
                val sr = SegmentRegister.decodeSR(byte1)
                val rm = decodeRMExpression(byte1, bytes, true)
            }*/
            PUSH_RM -> decodeStackRM(result, bytes, "push")
            PUSH_R -> decodeStackR(result, byte0, bytes, "push")
            PUSH_SR -> decodeStackSR(result, byte0, bytes, "push")
            POP_RM -> decodeStackRM(result, bytes, "pop")
            POP_R -> decodeStackR(result, byte0, bytes, "pop")
            POP_SR -> decodeStackSR(result, byte0, bytes, "pop")
            XCHG_RM_R -> decodeXchgRMToR(result, byte0, bytes)
            XCHG_R_A -> decodeXcghRToA(result, byte0, bytes)
            IN_F -> decodeIO(result, byte0, bytes, direction = true, variable = false)
            IN_V -> decodeIO(result, byte0, bytes, direction = true, variable = true)
            OUT_F -> decodeIO(result, byte0, bytes, direction = false, variable = false)
            OUT_V -> decodeIO(result, byte0, bytes, direction = false, variable = true)
            LEA, LDS, LES -> decodeLoad(result, bytes, opcode.toString())
            XLAT, LAHF, SAHF, PUSHF, POPF -> result.push(bytes, "$opcode")
            // ARITHMETIC & LOGIC
            OP_RM_R -> decodeArithmeticLogicRMToR(result, byte0, bytes)
            OP_IM_RM -> decodeArithmeticLogicIMToRM(result, byte0, bytes)
            OP_IM_A -> decodeArithmeticLogicIMToA(result, byte0, bytes)
            OP_GR2_RM -> decodeGr2(result, byte0, bytes)
            INC_R -> decodeIncDecR(result, byte0, bytes, "inc")
            DEC_R -> decodeIncDecR(result, byte0, bytes, "dec")
            OP_GR1_RM -> decodeOpGr1(result, byte0, bytes)
            AAM, AAD -> decodeAsciiAdjust(result, bytes, opcode)
            AAA, DAA, AAS, DAS, CBW, CWD -> result.push(bytes, "$opcode")
            OP_SHIFT_RM -> decodeOpShift(result, byte0, bytes)
            AND_RM_R -> decodeInstructionRMToRFull(result, byte0, bytes, "and")
            AND_A -> decodeOpA(result, byte0, bytes, "and")
            TEST_RM_R -> decodeInstructionRMToRFull(result, byte0, bytes, "test")
            TEST_A -> decodeOpA(result, byte0, bytes, "test")
            // STRING MANIPULATION
            REP -> { previousRepZero = decodeWordFlag(byte0) }
            MOVS, CMPS, SCAS, LODS, STOS -> decodeStringOp(result, byte0, bytes, opcode)
            // CONTROL TRANSFER
            JE, JL, JLE, JB, JBE, JP, JO, JS, JNE, JNL, JNLE, JNB, JNBE, JNP, JNO, JNS, LOOP, LOOPZ, LOOPNZ, JCXZ -> decodeJump(result, bytes, opcode)
            RET_S -> result.push(bytes, "ret")
            RET_SI -> {
                val immediate = decodeDataSigned(bytes, true)
                result.push(bytes, "ret $immediate")
            }
            INT_T -> {
                val immediate = bytes.nextByte()
                result.push(bytes, "int $immediate")
            }
            INT_3 -> result.push(bytes, "int 3")
            INTO, IRET -> result.push(bytes, "$opcode")
            // PROCESSOR CONTROL
            CLC, CMC, STC, CLD, STD, CLI, STI, HLT, WAIT -> result.push(bytes, "$opcode")
            LOCK -> {}
        }
        // TODO(alex): simplify this
        if (previousLock && currentSr == null) {
            previousLock = false
            val last = result[result.size - 1]
            result[result.size - 1] = Pair(last.baseAddress()!! - 1, "lock ${last.line()}")
        }
        if (previousRep) {
            previousRep = false
            val last = result[result.size - 1]
            val repOp = if (previousRepZero) "rep" else "repne"
            result[result.size - 1] = Pair(last.baseAddress()!! - 1, "$repOp ${last.line()}")
        }
        if (previousSr != null) {
            val last = result[result.size - 1]
            result[result.size - 1] = Pair(last.baseAddress()!! - 1, last.line().replace("[", "$previousSr:["))
            previousSr = null
        }
        if (opcode == REP) {
            previousRep = true
        }
        if (opcode == LOCK) {
            previousLock = true
        }
        if (opcode == S_OVERRIDE) {
            previousSr = currentSr
        }
        currentSr = null
    }

    return result.joinToString("\n") { it.line() }
}

fun decodeStringOp(result: InstructionList, byte0: Int, bytes: Bytes, opcode: Opcode) {
    val wordFlag = decodeWordFlag(byte0)
    val suffix = if (wordFlag) "w" else "b"
    result.push(bytes, "$opcode$suffix")
}

fun disassemble8086(file: File): String = disassemble8086(file.readBytes())

private fun decodeAsciiAdjust(result: InstructionList, bytes: Bytes, opcode: Opcode) {
    bytes.nextByte()
    result.push(bytes, "$opcode")
}

private fun decodeJump(result: InstructionList, bytes: Bytes, opcode: Opcode) {
    val jumpOffset = decodeDataSigned(bytes, false) + 2
    val jumpLocation = "\$${if (jumpOffset >= 0) "+" else ""}$jumpOffset"
    result.push(bytes, "$opcode $jumpLocation")
}

enum class ShiftOperation {
    ROL, ROR, RCL, RCR, SHL, SHR, SAR;

    companion object {
        private fun decodeShiftOperationOpcode(byte: Int) = (byte and 0b00111000) shr 3
        fun decodeShiftOperation(byte: Int): ShiftOperation {
            return when (decodeShiftOperationOpcode(byte)) {
                0 -> ROL
                1 -> ROR
                2 -> RCL
                3 -> RCR
                4 -> SHL
                5 -> SHR
                7 -> SAR
                else -> throw RuntimeException("Invalid shift op code")
            }
        }
    }

    override fun toString(): String = name.lowercase()
}

private fun decodeOpShift(result: InstructionList, byte0: Int, bytes: Bytes) {
    val useCReg = (byte0 and 0b00000010) shr 1 == 1
    val wordFlag = decodeWordFlag(byte0)
    val byte1 = bytes.nextByte()
    val op = ShiftOperation.decodeShiftOperation(byte1).toString()
    val rm = decodeRMExpression(byte1, bytes, wordFlag)
    val src = if (useCReg) "cl" else "1"
    result.push(bytes, "$op $rm, $src")
}

enum class Group1Operation {
    TEST, NOT, NEG, MUL, IMUL, DIV, IDIV;

    companion object {
        private fun decodeGroup1OperationOpcode(byte: Int) = (byte and 0b00111000) shr 3
        fun decodeGroup1Operation(byte: Int): Group1Operation {
            return when (decodeGroup1OperationOpcode(byte)) {
                0 -> TEST
                2 -> NOT
                3 -> NEG
                4 -> MUL
                5 -> IMUL
                6 -> DIV
                7 -> IDIV
                else -> throw RuntimeException("Invalid mul opcode")
            }
        }
    }

    override fun toString(): String = name.lowercase()
}

private fun decodeOpGr1(result: InstructionList, byte0: Int, bytes: Bytes) {
    val wordFlag = decodeWordFlag(byte0)
    val byte1 = bytes.nextByte()
    val op = Group1Operation.decodeGroup1Operation(byte1)
    val rm = decodeRMExpression(byte1, bytes, wordFlag)
    if (op == Group1Operation.TEST) {
        val immediate = bytes.nextByte()
        result.push(bytes, "$op $rm, $immediate")
    } else {
        result.push(bytes, "$op $rm")
    }
}

enum class Group2Operation {
    INC, DEC, CALL, CALL_I, JMP, JMP_I, PUSH;

    companion object {
        private fun decodeGroup2OperationOpcode(byte: Int) = (byte and 0b00111000) shr 3
        fun decodeGroup2Operation(byte: Int): Group2Operation {
            return when (Group2Operation.decodeGroup2OperationOpcode(byte)) {
                0 -> INC
                1 -> DEC
                2 -> CALL
                3 -> CALL_I
                4 -> JMP
                5 -> JMP_I
                6 -> PUSH
                else -> throw RuntimeException("Invalid mul opcode")
            }
        }
    }

    override fun toString(): String = name.lowercase()
}

private fun decodeGr2(result: InstructionList, byte0: Int, bytes: Bytes) {
    val wordFlag = decodeWordFlag(byte0)
    val byte1 = bytes.nextByte()
    val op = Group2Operation.decodeGroup2Operation(byte1)
    val useType = op != Group2Operation.CALL && op != Group2Operation.JMP
    val rm = decodeRMExpression(byte1, bytes, wordFlag, useType)
    result.push(bytes, "$op $rm")
}

private fun decodeIncDecR(result: InstructionList, byte0: Int, bytes: Bytes, op: String) {
    val reg = Register.decodeRMFieldToRegister(byte0, true)
    result.push(bytes, "$op $reg")
}

private fun decodeIO(result: InstructionList, byte0: Int, bytes: Bytes, direction: Boolean, variable: Boolean) {
    val op = if (direction) "in" else "out"
    val wordFlag = decodeWordFlag(byte0)
    val reg = if (wordFlag) Register.AX else Register.AL
    val port = if (variable) Register.DX else bytes.nextByte()
    val dst = if (direction) reg else port
    val src = if (direction) port else reg
    result.push(bytes, "$op $dst, $src")
}

private fun decodeStackRM(result: InstructionList, bytes: Bytes, op: String) {
    val byte1 = bytes.nextByte()
    val rm = decodeRMExpression(byte1, bytes, true)
    result.push(bytes, "$op $rm")
}

private fun decodeStackR(result: InstructionList, byte0: Int, bytes:Bytes, op: String) {
    val reg = Register.decodeRMFieldToRegister(byte0, true)
    result.push(bytes, "$op $reg")
}

private fun decodeStackSR(result: InstructionList, byte0: Int, bytes: Bytes, op: String) {
    val sr = SegmentRegister.decodeSR(byte0)
    result.push(bytes, "$op $sr")
}

private enum class SegmentRegister {
    ES, CS, SS, DS;

    companion object {
        private fun decodeSReg(byte: Int): Int = (byte and 0b00011000) shr 3
        fun decodeSR(byte: Int): SegmentRegister {
            return when (decodeSReg(byte)) {
                0 -> ES
                1 -> CS
                2 -> SS
                3 -> DS
                else -> throw RuntimeException("Invalid SR code")
            }
        }
    }

    override fun toString(): String = name.lowercase()
}

private enum class ArithmeticLogicOperation {
    ADD, OR, ADC, SBB, AND, SUB, XOR, CMP;

    companion object {
        private fun decodeArithmeticLogicOp(byte: Int) = (byte and 0b00111000) shr 3
        fun decodeOperation(byte: Int): ArithmeticLogicOperation {
            return when (decodeArithmeticLogicOp(byte)) {
                0 -> ADD
                1 -> OR
                2 -> ADC
                3 -> SBB
                4 -> AND
                5 -> SUB
                6 -> XOR
                7 -> CMP
                else -> throw RuntimeException("Invalid operation code")
            }
        }
    }

    override fun toString(): String = name.lowercase()
}

private fun decodeLoad(result: InstructionList, bytes: Bytes, op: String) {
    decodeInstructionRMToR(result, bytes, directionFlag = true, wordFlag = true, op)
}

private fun decodeXchgRMToR(result: InstructionList, byte0: Int, bytes: Bytes) {
    val wordFlag = decodeWordFlag(byte0)
    decodeInstructionRMToR(result, bytes, true, wordFlag, "xchg")
}

private fun decodeXcghRToA(result: InstructionList, byte0: Int, bytes: Bytes) {
    val reg = Register.decodeRMFieldToRegister(byte0, true)
    result.push(bytes, "xchg ax, $reg")
}

private fun decodeRMToR(bytes: Bytes, wordFlag: Boolean): Pair<String, String> {
    val byte1 = bytes.nextByte()
    val mode = Mode.decodeMode(byte1)
    val register = Register.decodeRegisterFieldToRegister(byte1, wordFlag).toString()
    val rm = when (mode) {
        Mode.REGISTER_MODE -> Register.decodeRMFieldToRegister(byte1, wordFlag).toString()
        Mode.MEMORY_MODE -> if (decodeRMField(byte1) == 6) "[${bytes.nextWord()}]" else decodeEffectiveAddress(byte1, "")
        else -> decodeEffectiveAddress(byte1, decodeDataSignExtended(bytes, mode))
    }
    return Pair(register, rm)
}

private fun decodeMovImToR(result: InstructionList, byte0: Int, bytes: Bytes) {
    val wordFlag = decodeWordFlagImmediateToRegister(byte0)
    val registerField = Register.decodeRMFieldToRegister(byte0, wordFlag)
    val immediate = bytes.nextData(wordFlag)
    result.push(bytes, "mov $registerField, $immediate")
}

private fun decodeArithmeticLogicRMToR(result: InstructionList, byte0: Int, bytes: Bytes) {
    val op = ArithmeticLogicOperation.decodeOperation(byte0).toString()
    decodeInstructionRMToRFull(result, byte0, bytes, op)
}

private fun decodeInstructionRMToRFull(result: InstructionList, byte0: Int, bytes: Bytes, op: String) {
    val directionFlag = decodeDirectionFlag(byte0)
    val wordFlag = decodeWordFlag(byte0)
    decodeInstructionRMToR(result, bytes, directionFlag, wordFlag, op)
}

private fun decodeInstructionRMToR(result: InstructionList, bytes: Bytes, directionFlag: Boolean, wordFlag: Boolean, op: String) {
    val (register, rm) = decodeRMToR(bytes, wordFlag)

    val dst = if (directionFlag) register else rm
    val src = if (directionFlag) rm else register
    result.push(bytes, "$op $dst, $src")
}

private fun decodeIMToRM(result: InstructionList, byte0: Int, bytes: Bytes) {
    val wordFlag = decodeWordFlag(byte0)

    val byte1 = bytes.nextByte()
    val mode = Mode.decodeMode(byte1)
    val offsetEncoding = decodeDataSignExtended(bytes, mode)
    val rm = decodeEffectiveAddress(byte1, offsetEncoding)
    val immediate = bytes.nextData(wordFlag)
    val type = if (wordFlag) "word" else "byte"
    val immediateEncoding = "$type $immediate"

    result.push(bytes, "mov $rm, $immediateEncoding")
}

private fun decodeRMExpression(byte1: Int, bytes: Bytes, wordFlag: Boolean, useType: Boolean = true): String {
    val mode = Mode.decodeMode(byte1)
    return if (mode != Mode.REGISTER_MODE) {
        val offsetEncoding = decodeDataSignExtended(bytes, mode)
        val expression = if (mode == Mode.MEMORY_MODE && decodeRMField(byte1) == 6) "[${bytes.nextWord()}]" else decodeEffectiveAddress(byte1, offsetEncoding)
        if (useType) {
            val type = if (wordFlag) "word" else "byte"
            "$type $expression"
        } else {
            expression
        }
    } else Register.decodeRMFieldToRegister(byte1, wordFlag).toString()
}

private fun decodeArithmeticLogicIMToRM(result: InstructionList, byte0: Int, bytes: Bytes) {
    val signExtendFlag = decodeSignExtended(byte0)
    val wordFlag = decodeWordFlag(byte0)

    val byte1 = bytes.nextByte()
    val op = ArithmeticLogicOperation.decodeOperation(byte1).toString()

    val rm = decodeRMExpression(byte1, bytes, wordFlag)
    var immediate = bytes.nextData(!signExtendFlag && wordFlag)
    if (signExtendFlag && wordFlag && immediate > 127) {
        immediate = 65536 - immediate
    }

    result.push(bytes, "$op $rm, $immediate")
}

private fun decodeOpA(result: InstructionList, byte0: Int, bytes: Bytes, op: String) {
    val wordFlag = decodeWordFlag(byte0)
    val register = if (wordFlag) Register.AX else Register.AL
    val immediate = bytes.nextData(wordFlag)
    result.push(bytes, "$op $register, $immediate")
}

private fun decodeArithmeticLogicIMToA(result: InstructionList, byte0: Int, bytes: Bytes) {
    val op = ArithmeticLogicOperation.decodeOperation(byte0).toString()
    decodeOpA(result, byte0, bytes, op)
}

private fun decodeDataSigned(bytes: Bytes, wide: Boolean): Int {
    return if (wide) {
        val data = bytes.nextWord()
        data.toShort().toInt()
    } else {
        val data = bytes.nextByte()
        data.toByte().toInt()
    }
}

private fun decodeDataSignExtended(bytes: Bytes, mode: Mode): String {
    return if (mode == Mode.MEMORY_MODE_16) {
        val data = bytes.nextWord()
        if (data > 32767) " - ${65536 - data}" else if (data != 0) " + $data" else ""
    } else if (mode == Mode.MEMORY_MODE_8) {
        val data = bytes.nextByte()
        if (data > 127) " - ${256 - data}" else if (data != 0) " + $data" else ""
    } else if (mode == Mode.MEMORY_MODE) ""
    else throw RuntimeException("Invalid mode")
}

fun decodeEffectiveAddress(byte: Int, expression: String): String {
    return when (decodeRMField(byte)) {
        0 -> "[bx + si$expression]"
        1 -> "[bx + di$expression]"
        2 -> "[bp + si$expression]"
        3 -> "[bp + di$expression]"
        4 -> "[si$expression]"
        5 -> "[di$expression]"
        6 -> "[bp$expression]"
        7 -> "[bx$expression]"
        else -> throw RuntimeException("Invalid R/M code")
    }
}

fun decodeDirectionFlag(byte: Int): Boolean = byte and 0b00000010 > 0

fun decodeSignExtended(byte: Int): Boolean = byte and 0b00000010 > 0

fun decodeWordFlag(byte: Int): Boolean = byte and 0b00000001 > 0

fun decodeWordFlagImmediateToRegister(byte: Int): Boolean = byte and 0b00001000 > 0

enum class Mode {
    MEMORY_MODE,
    MEMORY_MODE_8,
    MEMORY_MODE_16,
    REGISTER_MODE;

    companion object {
        fun decodeMode(byte: Int): Mode {
            return when (byte and 0b11000000) {
                0b00000000 -> MEMORY_MODE
                0b01000000 -> MEMORY_MODE_8
                0b10000000 -> MEMORY_MODE_16
                0b11000000 -> REGISTER_MODE
                else -> throw RuntimeException("Invalid mode code")
            }
        }
    }
}

fun decodeRegisterField(byte: Int): Int = (byte and 0b00111000) shr 3

fun decodeRMField(byte: Int): Int = byte and 0b00000111

enum class Register {
    AL, CL, DL, BL,
    AH, CH, DH, BH,
    AX, CX, DX, BX,
    SP, BP, SI, DI;

    companion object {
        fun decodeRegisterFieldToRegister(byte: Int, wordFlag: Boolean): Register = indexToRegister(decodeRegisterField(byte), wordFlag)
        fun decodeRMFieldToRegister(byte: Int, wordFlag: Boolean): Register = indexToRegister(decodeRMField(byte), wordFlag)
        private fun indexToRegister(index: Int, wordFlag: Boolean): Register {
            return when (index) {
                0 -> if (wordFlag) AX else AL
                1 -> if (wordFlag) CX else CL
                2 -> if (wordFlag) DX else DL
                3 -> if (wordFlag) BX else BL
                4 -> if (wordFlag) SP else AH
                5 -> if (wordFlag) BP else CH
                6 -> if (wordFlag) SI else DH
                7 -> if (wordFlag) DI else BH
                else -> throw RuntimeException("Invalid register code")
            }
        }
    }

    override fun toString(): String = name.lowercase()
}