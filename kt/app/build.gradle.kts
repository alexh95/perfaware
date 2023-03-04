import org.jetbrains.kotlin.gradle.internal.ensureParentDirsCreated

plugins {
    id("org.jetbrains.kotlin.jvm") version "1.8.10"
    application
}

repositories {
    mavenCentral()
}

dependencies {
    testImplementation("org.jetbrains.kotlin:kotlin-test-junit5")
    testImplementation("org.junit.jupiter:junit-jupiter-engine:5.9.1")
}

application {
    mainClass.set("tech.alexh95.perfaware.AppKt")
}

tasks.withType<Jar> {
    manifest {
        attributes("Main-Class" to application.mainClass)
    }
    archiveBaseName.set("disasm")
    duplicatesStrategy = DuplicatesStrategy.EXCLUDE
    from(configurations.runtimeClasspath.get().map {
        if (it.isDirectory) it else zipTree(it)
    })
}

tasks.named<Test>("test") {
    dependsOn("buildAsmTestFiles")
    useJUnitPlatform()
}

//val nasmUrl: String = if (org.gradle.internal.os.OperatingSystem.current().isWindows) {
//    "https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/win64/nasm-2.16.01-win64.zip"
//} else {
//    "https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/linux/nasm-2.16.01-0.fc36.x86_64.rpm"
//}
val nasmUrl = "https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/win64/nasm-2.16.01-win64.zip"
val nasmFileName = nasmUrl.substring(nasmUrl.lastIndexOf("/") + 1, nasmUrl.length)

// TODO(alex): make this cache so it does not download it every build
tasks.register<DefaultTask>("downloadNasm") {
    group = "verification"

    doLast {
        val destFile = layout.buildDirectory.file("tmp/nasm/$nasmFileName").get().asFile
        destFile.ensureParentDirsCreated()
        ant.invokeMethod("get", mapOf("src" to nasmUrl, "dest" to destFile))
        copy {
            from(zipTree(destFile))
            into(layout.buildDirectory.dir("resources"))
        }
    }
}

tasks.register<DefaultTask>("buildAsmTestFiles") {
    group = "verification"
    dependsOn("downloadNasm")
    // NOTE(alex): this is needed because the resources/test folder gets deleted by GitHub actions
    mustRunAfter("processTestResources")

    doLast {
        val nasmPath = layout.buildDirectory.file("resources/nasm-2.16.01/nasm.exe").get().asFile.path
        val outDirPath = layout.buildDirectory.dir("resources/test/assembly").get().asFile.path

        layout.projectDirectory.dir("src/test/resources/assembly").asFileTree.files.forEach {
            val outputName = it.name.substring(0, it.name.indexOf('.'))
            val outputPath = outDirPath + File.separator + outputName
            File(outputPath).ensureParentDirsCreated()
            exec {
                commandLine(nasmPath, it.path, "-o $outputPath")
            }
        }
    }
}
