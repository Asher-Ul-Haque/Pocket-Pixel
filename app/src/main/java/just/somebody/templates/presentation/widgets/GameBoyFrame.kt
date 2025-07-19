package just.somebody.templates.presentation.widgets // Updated package name

import android.content.Context
import android.opengl.GLSurfaceView
import just.somebody.templates.App
import just.somebody.templates.domain.GameBoy
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/*
 Custom GLSurfaceView to render the Game Boy emulator's output using OpenGL ES.
 This view acts as the bridge between the Android UI and the native C++ OpenGL rendering.
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

  // - - - Inner class implementing the GLSurfaceView.Renderer interface. Defines the callbacks for OpenGL ES rendering events.
  inner class GameBoyRenderer : Renderer
  {
    // - - - Called once to create the OpenGL ES rendering context.
    override fun onSurfaceCreated(GL : GL10?, CONFIG : EGLConfig?)
    { gameBoy.nativeOnSurfaceCreated() }

    // - - - update on a change like rotation
    override fun onSurfaceChanged(GL : GL10?, WIDTH : Int, HEIGHT : Int)
    { gameBoy.nativeOnSurfaceChanged(WIDTH, HEIGHT) }

    // - - - draw
    override fun onDrawFrame(GL : GL10?)
    { gameBoy.nativeOnDrawFrame() }
  }
}
