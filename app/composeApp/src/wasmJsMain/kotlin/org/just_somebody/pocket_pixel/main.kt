import androidx.compose.ui.ExperimentalComposeUiApi
import androidx.compose.ui.window.ComposeViewport
import kotlinx.browser.document
import org.just_somebody.pocket_pixel.App
import org.w3c.dom.Audio

@OptIn(ExperimentalComposeUiApi::class)
fun main()
{
    ComposeViewport(document.body!!) { App(); }
}