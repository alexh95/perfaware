package tech.alexh95.perfaware

import java.io.File

@OptIn(ExperimentalUnsignedTypes::class)
fun disassemble8086(file: File): String {
    return disassemble8086(file.readBytes().toUByteArray())
}

@OptIn(ExperimentalUnsignedTypes::class)
fun disassemble8086(bytes: UByteArray): String {
    val result = StringBuilder()
    result.append("bits 16\n")

    var byteIndex = 0
    while (byteIndex < bytes.size) {
        result.append("\n")

        val firstByte = bytes[byteIndex]

        val opcode = Opcode.decodeOperation(firstByte)
        val directionFlag = decodeDirectionFlag(firstByte)
        val wordFlag = decodeWordFlag(firstByte)

        val secondByte = bytes[byteIndex + 1]

        val mode = Mode.decodeMode(secondByte)
        val registerField = Register.decodeRegisterField(secondByte, wordFlag, false)
        val rmField = Register.decodeRegisterField(secondByte, wordFlag, true)

        when (opcode) {
            Opcode.MOV -> {
                if (directionFlag) {
                    result.append("$opcode $registerField, $rmField")
                } else {
                    result.append("$opcode $rmField, $registerField")
                }
            }
        }

        byteIndex += 2
    }

    return result.toString().lowercase()
}

enum class Opcode(val code: UByte, val mask: UByte) {
    MOV(0b1000_1000u, 0b1111_1100u);

    companion object {
        fun decodeOperation(byte: UByte): Opcode {
            Opcode.values().forEach {
                if (byte and it.mask == it.code) {
                    return it
                }
            }
            throw RuntimeException("Invalid Opcode")
        }
    }
}

fun decodeDirectionFlag(byte: UByte): Boolean {
    return byte and 0b0000_0010u > 0u
}

fun decodeWordFlag(byte: UByte): Boolean {
    return byte and 0b0000_0001u > 0u
}

enum class Mode(val code: UByte, val mask: UByte, val displacement: Int) {
    MEMORY_MODE(0b0000_0000u, 0b1100_0000u, 0),
    MEMORY_MODE_8(0b0100_0000u, 0b1100_0000u, 1),
    MEMORY_MODE_16(0b1000_0000u, 0b1100_0000u, 2),
    REGISTER_MODE(0b1100_0000u, 0b1100_0000u, 0);

    companion object {
        fun decodeMode(byte: UByte): Mode {
            Mode.values().forEach {
                if (byte and it.mask == it.code) {
                    return it
                }
            }
            throw RuntimeException("Invalid mode code")
        }
    }
}

enum class Register {
    AL, CL, DL, BL,
    AH, CH, DH, BH,
    AX, CX, DX, BX,
    SP, BP, SI, DI;

    companion object {
        fun decodeRegisterField(byte: UByte, wordFlag: Boolean, shifted: Boolean): Register {
            val field: Int = if (shifted) {
                (byte and 0b00_000_111u).toInt()
            } else {
                (byte and 0b00_111_000u).toInt() shr 3
            }
            return when (field) {
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