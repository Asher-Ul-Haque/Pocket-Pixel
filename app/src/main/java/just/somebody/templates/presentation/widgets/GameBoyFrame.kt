package just.somebody.templates.presentation.widgets // Updated package name

import android.content.Context
import android.opengl.GLSurfaceView
import just.somebody.templates.App
import just.somebody.templates.domain.GameBoy
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * A custom [GLSurfaceView] that renders the Game Boy emulator's display output using OpenGL ES.
 *
 * This component acts as the hardware graphics bridge between the Android UI environment and
 * the native execution layer. It relies on a synchronized frame state loop to process pixel maps.
 *
 * @param CONTEXT The execution context of the host view hierarchy.
 */
class GameBoyFrame(CONTEXT: Context) : GLSurfaceView(CONTEXT)
{
  private var gameBoy: GameBoy = App.appModule.gameBoy

  init
  {
    setEGLContextClientVersion(2)

    // - - - Set the custom renderer for this GLSurfaceView.
    setRenderer(GameBoyRenderer())

    // - - - Set the render mode to RENDERMODE_WHEN_DIRTY (only rederaw when explicity asked)
    renderMode = RENDERMODE_WHEN_DIRTY

    GameBoy.setGLSurfaceView(this)
  }

  /**
   * Dedicated hardware renderer implementation connecting OpenGL callbacks to native surface registers.
   */
  inner class GameBoyRenderer : Renderer
  {
    /**
     * Triggered once when the OpenGL ES execution layer is established.
     * Sets up hardware shaders, matrix transformations, and graphic textures.
     */
    override fun onSurfaceCreated(GL : GL10?, CONFIG : EGLConfig?)
    { gameBoy.nativeOnSurfaceCreated() }

    /**
     * Triggered when screen dimensions update, such as layout resizing or screen orientation shifts.
     * Updates viewport mapping boundaries to preserve correct pixel geometry.
     */
    override fun onSurfaceChanged(GL : GL10?, WIDTH : Int, HEIGHT : Int)
    { gameBoy.nativeOnSurfaceChanged(WIDTH, HEIGHT) }

    /**
     * Executed on demand whenever a frame buffer synchronization requests a refresh pass.
     * Flushes the decoded VRAM matrix straight to the GPU display panel.
     */
    override fun onDrawFrame(GL : GL10?)
    { gameBoy.nativeOnDrawFrame() }
  }
}