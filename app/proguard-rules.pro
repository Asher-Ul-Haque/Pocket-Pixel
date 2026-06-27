# Keep your main activity
-keep class just.somebody.templates.MainActivity { *; }

# Keep your application class (if you have one)
-keep class just.somebody.templates.**Application { *; }
# Keep the specific class accessed by FindClass in JNI
-keep class just.somebody.templates.appModule.network.NetworkService { *; }

# Keep the GameBoy class (which JNI_OnLoad uses to find methods)
-keep class just.somebody.templates.domain.GameBoy { *; }

# Keep the AppModule interface and any implementers
-keep class just.somebody.templates.appModule.AppModuleInterface { *; }

# Keep all native methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep members accessed via JNI
-keepclassmembers class just.somebody.templates.domain.GameBoy {
    static <methods>;
}
# Keep Kotlin metadata
-keep class kotlin.Metadata { *; }

# Needed for reflection
-keepclassmembers class ** {
    @kotlinx.serialization.Serializable *;
}
-keepclassmembers class ** {
    @androidx.room.* <methods>;
}

# Keep Kotlinx serialization classes
-keep class kotlinx.serialization.** { *; }
-dontwarn kotlinx.serialization.**

# Keep Room
-keep class androidx.room.** { *; }
-dontwarn androidx.room.**

# Keep Ktor (for HTTP + WebSocket)
-keep class io.ktor.** { *; }
-dontwarn io.ktor.**

# Keep Socket.IO client
-keep class io.socket.** { *; }
-dontwarn io.socket.**

# Prevent Compose classes from being stripped
-keep class androidx.compose.** { *; }
-dontwarn androidx.compose.**

# Optional: keep your logger
-keep class just.somebody.templates.appModule.ForgeLogger { *; }

##############################
# ✅ Android-specific
##############################

# Keep classes with entry points
-keepclassmembers class * {
    public <init>(android.content.Context);
}

# Keep all custom views
-keep public class * extends android.view.View {
    public <init>(android.content.Context);
    public <init>(android.content.Context, android.util.AttributeSet);
}

# Keep data classes (for JSON, DB, etc.)
-keepclassmembers class * {
    <fields>;
    <methods>;
}