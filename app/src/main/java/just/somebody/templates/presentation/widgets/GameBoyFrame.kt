package just.somebody.templates.presentation.widgets // Updated package name

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import just.somebody.templates.App
import just.somebody.templates.domain.GameBoy
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * Custom GLSurfaceView to render the Game Boy emulator's output using OpenGL ES.
 * This view acts as the bridge between the Android UI and the native C++ OpenGL rendering.
 */
class GameBoyFrame : GLSurfaceView {

  // Reference to the GameBoy instance, used to call native JNI methods
  // Initialized directly from App.appModule.gameBoy as it's expected to be available.
  private var gameBoy: GameBoy = App.appModule.gameBoy

  /**
   * Constructor for programmatically creating the view.
   */
  constructor(context: Context) : super(context) {
    init()
  }

  /**
   * Constructor for inflating the view from XML (though we're using Compose here, it's good practice).
   */
  constructor(context: Context, attrs: AttributeSet) : super(context, attrs) {
    init()
  }

  /**
   * Common initialization logic for both constructors.
   */
  private fun init() {
    // Set the EGL context client version to 2 for OpenGL ES 2.0.
    // If you plan to use OpenGL ES 3.0+, set this to 3.
    setEGLContextClientVersion(2)

    // Set the custom renderer for this GLSurfaceView.
    setRenderer(GameBoyRenderer())

    // Set the render mode to RENDERMODE_WHEN_DIRTY.
    // This means the screen will only redraw when requestRender() is explicitly called.
    // This is crucial for performance in an emulator, as we only redraw when a new frame is ready.
    renderMode = RENDERMODE_WHEN_DIRTY

    // Register this GLSurfaceView instance with the GameBoy companion object
    // so that C++ can call requestRenderFromNative() on it.
    // This is moved here as 'gameBoy' is already initialized.
    GameBoy.setGLSurfaceView(this)
  }

  // setGameBoyInstance function removed as 'gameBoy' is initialized via DI.

  /**
   * Inner class implementing the GLSurfaceView.Renderer interface.
   * This interface defines the callbacks for OpenGL ES rendering events.
   */
  inner class GameBoyRenderer : Renderer {
    /**
     * Called once to create the OpenGL ES rendering context.
     * This is where you should perform your native OpenGL ES initialization.
     */
    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
      // Call the native C++ function to perform OpenGL ES setup (e.g., compile shaders, create textures).
      gameBoy.nativeOnSurfaceCreated() // No null-safe call needed now
    }

    /**
     * Called when the surface size changes (e.g., screen rotation, initial setup).
     * This is where you should update your native OpenGL ES viewport.
     */
    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
      // Call the native C++ function to update the OpenGL ES viewport.
      gameBoy.nativeOnSurfaceChanged(width, height) // No null-safe call needed now
    }

    /**
     * Called repeatedly to draw the current frame.
     * In RENDERMODE_WHEN_DIRTY mode, this is called only after requestRender().
     */
    override fun onDrawFrame(gl: GL10?) {
      // Call the native C++ function to perform the actual frame rendering.
      gameBoy.nativeOnDrawFrame() // No null-safe call needed now
    }
  }
}
