package tech.alexh95.perfaware

import java.io.File

fun main(args: Array<String>) {
    if (args.isEmpty()) {
        println("Usage: disasm <filename>")
        return
    }

    val inputFile = File(args[0])
    if (!inputFile.exists()) {
        println("Input file not found")
        return
    }

    val assemblyCode = disassemble8086(inputFile)
    println(assemblyCode)
}
