import java.util.Properties
import java.io.FileInputStream
import java.net.URI

val properties = Properties().apply {
  val file = rootProject.file("keystore.properties")
  if (file.exists()) {
    load(FileInputStream(file))
  }
}

plugins {
  alias(libs.plugins.android.application)
  alias(libs.plugins.kotlin.android)
  alias(libs.plugins.kotlin.compose)
  id("com.google.devtools.ksp") // Apply KSP plugin
  kotlin("plugin.serialization") version "2.2.0"
  id("org.jetbrains.dokka") version "1.9.20" // <-- 1. ADDED DOKKA PLUGIN
}

android {
  namespace = "just.somebody.templates"
  compileSdk = 35

  signingConfigs {
    create("release") {
      storeFile = file(properties["RELEASE_STORE_FILE"] as String)
      storePassword = properties["RELEASE_STORE_PASSWORD"] as String
      keyAlias = properties["RELEASE_KEY_ALIAS"] as String
      keyPassword = properties["RELEASE_KEY_PASSWORD"] as String
    }
  }

  defaultConfig {
    applicationId = "just.somebody.templates"
    minSdk = 24
    targetSdk = 35
    versionCode = 14
    versionName = "2.1.2"
    androidResources {
      localeFilters.add("en") // Keep only English
    }
    testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    externalNativeBuild {
      cmake {
        cppFlags += "-O3"
        arguments += listOf(
          "-DANDROID_STL=c++_shared",
          "-DCMAKE_BUILD_TYPE=Release",
          "-DLOG_WARNING_ENABLED=0",
          "-DLOG_INFO_ENABLED=0",
          "-DLOG_DEBUG_ENABLED=0",
          "-DLOG_TRACE_ENABLED=0",
          "-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON"
                           )
      }
    }
  }

  buildTypes {
    release {
      signingConfig = signingConfigs.getByName("release")
      isMinifyEnabled = true
      isShrinkResources = true
      proguardFiles(
        getDefaultProguardFile("proguard-android-optimize.txt"),
        "proguard-rules.pro"
                   )
    }
  }
  compileOptions {
    sourceCompatibility = JavaVersion.VERSION_11
    targetCompatibility = JavaVersion.VERSION_11
  }
  kotlinOptions {
    jvmTarget = "11"
  }
  buildFeatures {
    compose = true
    prefab = true
  }
  externalNativeBuild {
    cmake {
      path = file("src/main/cpp/CMakeLists.txt")
      version = "3.22.1"
    }
  }
}

dependencies {
  implementation(libs.androidx.core.ktx)
  implementation(libs.androidx.lifecycle.runtime.ktx)
  implementation(libs.androidx.activity.compose)
  implementation(platform(libs.androidx.compose.bom))
  implementation(libs.androidx.ui)
  implementation(libs.androidx.ui.graphics)
  implementation(libs.androidx.ui.tooling.preview)
  implementation(libs.androidx.material3)
  implementation(libs.androidx.lifecycle.viewmodel.compose)
  implementation(libs.androidx.datastore)
  implementation("io.socket:socket.io-client:2.1.0") {
    exclude(group = "org.json", module = "json") // prevent version conflicts
  }

  implementation(libs.kotlinx.serialization.json)
  implementation(libs.kotlinx.collections.immutable)
  implementation(libs.kotlinx.serialization.json.v132)

  implementation(libs.androidx.core.splashscreen)
  implementation(libs.androidx.navigation.compose)
  implementation(libs.androidx.lifecycle.runtime.compose)
  implementation(libs.androidx.documentfile)

  // Room dependencies
  implementation(libs.androidx.room.runtime)
  ksp(libs.androidx.room.compiler)

  implementation(libs.androidx.sqlite.bundled)

  // Ktor & Coil
  implementation(libs.coil.compose)
  implementation(libs.ktor.client.android)
  implementation(libs.ktor.client.core)
  implementation(libs.ktor.client.content.negotiation)
  implementation(libs.ktor.client.logging)
  implementation(libs.ktor.serialization.kotlinx.json)
// Or the latest version
  // Testing
  testImplementation(libs.junit)
  androidTestImplementation(libs.androidx.junit)
  androidTestImplementation(libs.androidx.espresso.core)
  androidTestImplementation(platform(libs.androidx.compose.bom))
  androidTestImplementation(libs.androidx.ui.test.junit4)
  debugImplementation(libs.androidx.ui.tooling)
  debugImplementation(libs.androidx.ui.test.manifest)
}

// <-- 2. ADDED DOKKA CONFIGURATION TASK
tasks.dokkaHtml {
  dokkaSourceSets {
    configureEach {
      skipDeprecated.set(true)

      // Link to standard Android documentation so your Android classes are clickable
      externalDocumentationLink {
        url.set(URI.create("https://developer.android.com/reference/kotlin/").toURL())
      }
    }
  }
}