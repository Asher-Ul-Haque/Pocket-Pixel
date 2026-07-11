# Pocket-Pixel ProGuard Rules
# Optimized for size and performance while ensuring JNI and reflection stability.

# ---------------------------------------------------------------------------------
# 1. JNI & Native Bridge (libPocketPixel.so)
# ---------------------------------------------------------------------------------

# Keep the main entry point
-keep class just.somebody.templates.MainActivity { *; }

# Keep the Application class and its appModule field (accessed by JNI_OnLoad)
-keep class just.somebody.templates.App {
    public static just.somebody.templates.appModule.AppModuleInterface appModule;
    *;
}

# Keep classes and methods used by the C++ native bridge
-keep class just.somebody.templates.domain.GameBoy {
    # Keep all members to ensure @JvmStatic methods are preserved in the outer class
    *;
}

# Keep NetworkService for JNI-initiated HTTP requests (RetroAchievements)
-keep class just.somebody.templates.appModule.network.NetworkService {
    public void makeRaRequest(java.lang.String, java.lang.String, long);
    *;
}

# Keep the interface used by JNI to access the network service instance
-keep interface just.somebody.templates.appModule.AppModuleInterface {
    just.somebody.templates.appModule.network.NetworkService getNetworkService();
    *;
}

# Preserve all native method declarations
-keepclasseswithmembernames class * {
    native <methods>;
}

# ---------------------------------------------------------------------------------
# 2. Frameworks & Serialization
# ---------------------------------------------------------------------------------

# Kotlinx Serialization: Keep @Serializable members and support classes
-keepclassmembers class ** {
    @kotlinx.serialization.Serializable *;
}
-keep class kotlinx.serialization.** { *; }
-dontwarn kotlinx.serialization.**

# Room Database: Keep @Dao and @Entity related methods
-keepclassmembers class ** {
    @androidx.room.* <methods>;
}
-dontwarn androidx.room.**

# Ktor: Suppress warnings for various engines and features
-dontwarn io.ktor.**

# Socket.IO: Keep the library classes
-keep class io.socket.** { *; }
-dontwarn io.socket.**

# ---------------------------------------------------------------------------------
# 3. Android System & UI
# ---------------------------------------------------------------------------------

# Custom Views: Needed for XML layout inflation
-keep public class * extends android.view.View {
    public <init>(android.content.Context);
    public <init>(android.content.Context, android.util.AttributeSet);
}

# Reflection: Keep constructors used for dynamic instantiation (e.g., Workers, Fragments)
-keepclassmembers class * {
    public <init>(android.content.Context);
}

# ---------------------------------------------------------------------------------
# 4. Miscellaneous
# ---------------------------------------------------------------------------------

# Internal Logger
-keep class just.somebody.templates.appModule.ForgeLogger { *; }

# Suppress warnings from common libraries that provide their own rules
-dontwarn androidx.compose.**
-dontwarn kotlinx.coroutines.**
-dontwarn io.coil.**

# ---------------------------------------------------------------------------------
# Optimization Notes:
# Removed broad "keep members" rules that were preventing code shrinking.
# Fixed the Application class rule which was causing JNI_OnLoad failures.
# ---------------------------------------------------------------------------------
