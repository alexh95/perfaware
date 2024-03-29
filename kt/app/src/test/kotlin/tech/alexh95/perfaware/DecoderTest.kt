package tech.alexh95.perfaware

import org.junit.jupiter.api.DynamicTest
import org.junit.jupiter.api.TestFactory
import java.io.File
import kotlin.test.assertEquals
import kotlin.test.assertTrue

class DecoderTest {

    companion object {
        const val PROCESS_TIMEOUT = 2000
        val assemblyFilePath: String = Thread.currentThread().contextClassLoader.getResource("assembly")!!.file

        fun getTestFiles(): Map<String, Pair<File, File>> {
            return File(assemblyFilePath).listFiles()!!
                .filter { it.name.startsWith("listing") && !it.name.endsWith(".txt") }
                .groupBy { if (it.name.contains('.')) it.name.substring(0, it.name.indexOf('.')) else it.name  }
                .map { Pair(it.key, Pair(it.value[0], it.value[1])) }.toMap()
        }
    }

    @TestFactory
    fun testDisassemblyMatchesSource(): List<DynamicTest> {
        return getTestFiles().map { (listing, testFiles) ->
            DynamicTest.dynamicTest("Test file: $listing") {
                checkDisassembledMatchesSource(testFiles.first, testFiles.second)
            }
        }
    }

    private fun checkDisassembledMatchesSource(assembledFile: File, sourceFile: File) {
        println("Testing file: ${assembledFile.name}")
        println("debug print dir: $assemblyFilePath")
        File(assemblyFilePath).listFiles()!!.forEach { println(it.name) }
        val disassembledCode = disassemble8086(assembledFile)
        val disassembledFileName = "disassembled_${sourceFile.name}"
        File("$assemblyFilePath/$disassembledFileName").writeText(disassembledCode)

        val reassembledFileName = "reassembled_${assembledFile.name}"
        val process = ProcessBuilder("$assemblyFilePath/nasm.exe", disassembledFileName, "-o $reassembledFileName")
            .directory(File(assemblyFilePath))
            .start()
        assertTimeout(process)

        val expectedBinary = assembledFile.readBytes()
        val actualBinary = File("$assemblyFilePath/$reassembledFileName").readBytes()
        assertByteArrayEquals(expectedBinary, actualBinary)
    }

    private fun assertTimeout(process: Process) {
        val startTime = System.currentTimeMillis()
        while (process.isAlive) {
            val elapsedTime = System.currentTimeMillis() - startTime;
            assertTrue(elapsedTime <= PROCESS_TIMEOUT)
        }
    }

    private fun assertByteArrayEquals(expected: ByteArray, actual: ByteArray) {
        assertEquals(expected.size, actual.size)
        for (index in expected.indices) {
            assertEquals(expected[index], actual[index])
        }
    }

}
