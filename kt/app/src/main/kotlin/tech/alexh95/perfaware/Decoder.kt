package tech.alexh95.perfaware

import java.io.File

fun disassemble8086(file: File): String = disassemble8086(file.readBytes())

class Bytes constructor(private val array: ByteArray, private var index: Int) {
    fun hasNextByte(): Boolean = index < array.size
    fun nextByte(): Int = (array[index++].toUByte()).toInt()
    fun nextWord(): Int {
        val data0 = nextByte()
        val data1 = nextByte()
        return (data1 shl 8) or data0
    }
    fun nextData(word: Boolean): Int = if (word) nextWord() else nextByte()
}

fun disassemble8086(byteArray: ByteArray): String {
    val result = StringBuilder("BITS 16\n")

    val bytes = Bytes(byteArray, 0)
    while (bytes.hasNextByte()) {
        val byte0 = bytes.nextByte()
        when (Opcode.decodeOperation(byte0)) {
            Opcode.MOV_RM_R -> {
                val directionFlag = decodeDirectionFlag(byte0)
                val wordFlag = decodeWordFlag(byte0)

                val byte1 = bytes.nextByte()
                val mode = Mode.decodeMode(byte1)
                val register = Register.decodeRegisterFieldToRegister(byte1, wordFlag)
                val rm = when (mode) {
                    Mode.REGISTER_MODE -> Register.decodeRMFieldToRegister(byte1, wordFlag)
                    Mode.MEMORY_MODE -> if (decodeRMField(byte1) == 6) "[${bytes.nextWord()}]" else decodeEffectiveAddress(byte1, "")
                    else -> decodeEffectiveAddress(byte1, decodeSignExtended(bytes, mode))
                }

                val dst = if (directionFlag) register else rm
                val src = if (directionFlag) rm else register
                result.append("MOV $dst, $src\n")
            }
            Opcode.MOV_IM_RM -> {
                val wordFlag = decodeWordFlag(byte0)

                val byte1 = bytes.nextByte()
                val mode = Mode.decodeMode(byte1)
                val offsetEncoding = decodeSignExtended(bytes, mode)
                val rm = decodeEffectiveAddress(byte1, offsetEncoding)
                val immediate = bytes.nextData(wordFlag)
                val immediateType = if (wordFlag) "word" else "byte"
                val immediateEncoding = "$immediateType $immediate"

                result.append("MOV $rm, $immediateEncoding\n")
            }
            Opcode.MOV_IM_R -> {
                val wordFlag = decodeWordFlagImmediateToRegister(byte0)
                val registerField = Register.decodeRMFieldToRegister(byte0, wordFlag)
                val immediate = bytes.nextData(wordFlag)
                result.append("MOV $registerField, $immediate\n")
            }
            Opcode.MOV_M_A -> {
                result.append("MOV AX, [${bytes.nextWord()}]\n")
            }
            Opcode.MOV_A_M -> {
                result.append("MOV [${bytes.nextWord()}], AX\n")
            }
        }
    }

    return result.toString()
}

fun decodeSignExtended(bytes: Bytes, mode: Mode): String {
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
        0 -> "[BX + SI$expression]"
        1 -> "[BX + DI$expression]"
        2 -> "[BP + SI$expression]"
        3 -> "[BP + DI$expression]"
        4 -> "[SI$expression]"
        5 -> "[DI$expression]"
        6 -> "[BP$expression]"
        7 -> "[BX$expression]"
        else -> throw RuntimeException("Invalid R/M code")
    }
}

enum class Opcode(val code: Int, val mask: Int) {
    MOV_RM_R(0b1000_1000, 0b1111_1100),
    MOV_IM_RM(0b1100_0110, 0b1111_1110),
    MOV_IM_R(0b1011_0000, 0b1111_0000),
    MOV_M_A(0b1010_0000, 0b1111_1110),
    MOV_A_M(0b1010_0010, 0b1111_1110);

    companion object {
        fun decodeOperation(byte: Int): Opcode {
            Opcode.values().forEach {
                if (byte and it.mask == it.code) return it
            }
            throw RuntimeException("Invalid Opcode")
        }
    }
}

fun decodeDirectionFlag(byte: Int): Boolean = byte and 0b0000_0010 > 0

fun decodeWordFlag(byte: Int): Boolean = byte and 0b0000_0001 > 0

fun decodeWordFlagImmediateToRegister(byte: Int): Boolean = byte and 0b0000_1000 > 0

enum class Mode {
    MEMORY_MODE,
    MEMORY_MODE_8,
    MEMORY_MODE_16,
    REGISTER_MODE;

    companion object {
        fun decodeMode(byte: Int): Mode {
            return when (byte and 0b1100_0000) {
                0b0000_0000 -> MEMORY_MODE
                0b0100_0000 -> MEMORY_MODE_8
                0b1000_0000 -> MEMORY_MODE_16
                0b1100_0000 -> REGISTER_MODE
                else -> throw RuntimeException("Invalid mode code")
            }
        }
    }
}

fun decodeRegisterField(byte: Int): Int = (byte and 0b00_111_000) shr 3

fun decodeRMField(byte: Int): Int = byte and 0b00_000_111

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
}