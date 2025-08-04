# Keep your main activity
-keep class just.somebody.templates.MainActivity { *; }

# Keep your application class (if you have one)
-keep class just.somebody.templates.**Application { *; }

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