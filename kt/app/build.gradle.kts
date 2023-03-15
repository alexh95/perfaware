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
    testImplementation("org.junit.jupiter:junit-jupiter-engine:5.9.2")
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
    dependsOn("downloadNasm")
    dependsOn("copyTestFiles")
    useJUnitPlatform()
}

// TODO(alex): make this cross platform
//val nasmUrl: String = if (org.gradle.internal.os.OperatingSystem.current().isWindows) {
//    "https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/win64/nasm-2.16.01-win64.zip"
//} else {
//    "https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/linux/nasm-2.16.01-0.fc36.x86_64.rpm"
//}
val nasmUrl = "https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/win64/nasm-2.16.01-win64.zip"
val nasmFileName = nasmUrl.substring(nasmUrl.lastIndexOf("/") + 1, nasmUrl.length)
val nasmFolderName = nasmFileName.substring(0, nasmFileName.lastIndexOf("-"))

tasks.register<DefaultTask>("downloadNasm") {
    group = "verification"

    mustRunAfter("processTestResources")

    doLast {
        val destFile = layout.buildDirectory.file("tmp/nasm/$nasmFileName").get().asFile
        if (!destFile.exists()) {
            destFile.ensureParentDirsCreated()
            println("Downloading $nasmFileName")
            ant.invokeMethod("get", mapOf("src" to nasmUrl, "dest" to destFile))
        }
        copy {
            from(zipTree(destFile))
            into(layout.buildDirectory)
        }
        copy {
            from(layout.buildDirectory.file("$nasmFolderName/nasm.exe"))
            into(layout.buildDirectory.dir("resources/test/assembly"))
        }
    }
}

tasks.register<Copy>("copyTestFiles") {
    group = "verification"

    mustRunAfter("processTestResources")

    from("../../computer_enhance/perfaware/part1")
    into(layout.buildDirectory.dir("resources/test/assembly"))
}

