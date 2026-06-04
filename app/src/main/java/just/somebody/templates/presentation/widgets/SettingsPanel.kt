package just.somebody.templates.presentation.widgets

import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.material3.*
import androidx.compose.material3.TabRowDefaults.tabIndicatorOffset
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import coil.compose.AsyncImage
import coil.request.ImageRequest
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.domain.GameBoy
import just.somebody.templates.domain.models.PRESET_PALETTES
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.ui.theme.GameBoyColors
import java.util.Date
import android.text.format.DateFormat
import androidx.compose.foundation.layout.Arrangement
import androidx.core.graphics.toColorInt

/**
 * Main persistent control panel overlay containing in-game runtime options.
 *
 * Implements a structured tabbed modal bottom layout to distribute modifications smoothly
 * across localized hardware variables including independent sound channel volumes, color space palettes,
 * active fragment shader filters, save/load slot configurations, and systemic emulation modifiers.
 *
 * @param GAME_BOY Core underlying native backend engine interface state instance.
 * @param EMULATOR The host view state coordinator tracking running session profiles.
 * @param ON_CLOSE Lifecycle management callback triggered to collapse this sheet panel view.
 * @param MODIFIER [Modifier] used to establish operational canvas spacing geometry rules.
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsPanel(
  GAME_BOY: GameBoy,
  EMULATOR: EmulatorViewModel,
  ON_CLOSE: () -> Unit,
  MODIFIER: Modifier = Modifier)
{
  var settingsPage  by remember { mutableIntStateOf(0) }
  val settings      by EMULATOR.settings.collectAsState()

  ModalBottomSheet(
    onDismissRequest    = ON_CLOSE,
    containerColor      = GameBoyColors.DarkGreen,
    shape               = RectangleShape,
    dragHandle          = { BottomSheetDefaults.DragHandle(color = GameBoyColors.Green) }
                  )
  {
    Column(
      modifier = Modifier
        .padding(horizontal = 16.dp, vertical = 8.dp)
        .fillMaxWidth(),
      horizontalAlignment = Alignment.CenterHorizontally)
    {
      CustomText(stringResource(R.string.game_settings), FONT_SIZE = 20)
      Spacer(modifier = Modifier.height(4.dp))
      HorizontalDivider(
        thickness   = 1.dp,
        color       = GameBoyColors.MediumGreen,
        modifier    = Modifier.fillMaxWidth(0.4f))
      Spacer(modifier = Modifier.height(8.dp))

      TabRow(
        selectedTabIndex    = settingsPage,
        containerColor      = Color.Transparent,
        contentColor        = GameBoyColors.LightGreen,
        divider             = {},
        indicator           =
          { tabPositions ->
            TabRowDefaults.SecondaryIndicator(
              Modifier.tabIndicatorOffset(tabPositions[settingsPage]),
              color = GameBoyColors.Green)
          }
            )
      {
        val tabs = listOf(
          R.string.audio  to R.drawable.speaker,
          R.string.visual to R.drawable.paint_blob,
          R.string.states to R.drawable.gameboy,
          R.string.misc   to R.drawable.settings
        )
        tabs.forEachIndexed { index, (titleRes, iconRes) ->
          Tab(
            selected = settingsPage == index,
            onClick = { settingsPage = index }
          ) {
            Column(
              horizontalAlignment = Alignment.CenterHorizontally,
              modifier = Modifier.padding(6.dp)
            ) {
              Icon(
                painter = painterResource(iconRes),
                contentDescription = null,
                modifier = Modifier.size(20.dp),
                tint = if (settingsPage == index) GameBoyColors.LightGreen else GameBoyColors.MediumGreen
              )
              CustomText(
                stringResource(titleRes),
                FONT_SIZE = 10,
                MODIFIER = Modifier,
                COLOR = if (settingsPage == index) GameBoyColors.LightGreen else GameBoyColors.MediumGreen
              )
            }
          }
        }
      }

      Spacer(modifier = Modifier.height(12.dp))

      Box(modifier = Modifier.height(280.dp))
      {
        LazyColumn(
          modifier              = Modifier.fillMaxSize(),
          horizontalAlignment   = Alignment.CenterHorizontally,
          verticalArrangement   = Arrangement.Top)
        {
          item()
          {
            when (settingsPage)
            {
              0 -> AudioSettingsSection(settings.channelVolume) { vol, ch -> EMULATOR.setVolume(vol, ch) }
              1 -> VisualSettingsSection(
                PALETTE_INDEX    = settings.paletteIndex,
                SHADER_INDEX     = settings.shaderIndex,
                ON_PALETTE_SELECT = { EMULATOR.setPaletteIndex(it) },
                ON_SHADER_SELECT  = { EMULATOR.setShaderIndex(it) })
              2 -> SaveStateSection(EMULATOR)
              3 -> MiscSettingsSection(EMULATOR)
            }
          }
        }
      }

      Spacer(modifier = Modifier.height(8.dp))
    }
  }
}

/**
 * Dedicated sound configuration dashboard pane block.
 *
 * Connects standard audio voice registries directly into hardware mixers via descriptive text slider blocks.
 *
 * @param VOLUMES Collection list mapping current amplification levels across all accessible hardware voices.
 * @param ON_VOLUME_CHANGE Event stream processor shifting designated voice registries by calculated volume parameters.
 */
@Composable
private fun AudioSettingsSection(
  VOLUMES           : List<Float>,
  ON_VOLUME_CHANGE  : (Float, Int) -> Unit)
{
  val labels = listOf(
    stringResource(R.string.pulse1),
    stringResource(R.string.pulse2),
    stringResource(R.string.wave),
    stringResource(R.string.noise))
  Column(verticalArrangement = Arrangement.spacedBy(8.dp))
  {
    labels.forEachIndexed()
    { index, label ->
      Column()
      {
        CustomText(label, FONT_SIZE = 12, MODIFIER = Modifier.padding(bottom = 2.dp))
        RetroSlider(
          VALUE             = VOLUMES[index],
          ON_VALUE_CHANGE   = { ON_VOLUME_CHANGE(it, index) },
          MODIFIER          = Modifier.fillMaxWidth())
      }
    }
  }
}

/**
 * Graphic layout configuration sub-component controlling color mapping systems and display textures.
 *
 * Serves up selection rows to re-theme native lookup indexes or dynamically bind structural post-processing
 * fragmentation effects directly on top of the running OpenGL rendering surface context.
 *
 * @param PALETTE_INDEX Numerical offset pointer targeting the currently activated hardware color scheme model.
 * @param SHADER_INDEX Numerical offset pointer targeting the current active graphics post-processing calculation.
 * @param ON_PALETTE_SELECT State update pipeline redirecting selected structural layout indices back to storage variables.
 * @param ON_SHADER_SELECT State update pipeline redirecting chosen texture compilation formats to processing units.
 */
@Composable
private fun VisualSettingsSection(
  PALETTE_INDEX     : Int,
  SHADER_INDEX      : Int,
  ON_PALETTE_SELECT : (Int) -> Unit,
  ON_SHADER_SELECT  : (Int) -> Unit)
{
  val shaders = listOf(
    stringResource(R.string.shader_sharp),
    stringResource(R.string.shader_crt),
    stringResource(R.string.shader_lcd),
    stringResource(R.string.shader_chromatic),
    stringResource(R.string.shader_default))

  Column()
  {
    Row(
      verticalAlignment = Alignment.CenterVertically,
      modifier          = Modifier.fillMaxWidth())
    {
      CustomText(stringResource(R.string.palette), FONT_SIZE = 16, MODIFIER = Modifier.padding(end = 6.dp))
      HorizontalDivider(thickness = 1.dp, color = GameBoyColors.MediumGreen, modifier = Modifier.weight(1f))
    }

    Spacer(modifier = Modifier.height(4.dp))

    Box(modifier = Modifier.height(110.dp))
    {
      LazyColumn()
      {
        itemsIndexed(PRESET_PALETTES)
        { index, palette ->
          Row(
            modifier = Modifier
              .fillMaxWidth()
              .clickable { ON_PALETTE_SELECT(index) }
              .background(
                if (index == PALETTE_INDEX) GameBoyColors.MediumGreen
                else                        Color.Transparent)
              .padding(6.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment     = Alignment.CenterVertically)
          {
            CustomText(palette.name, FONT_SIZE = 12, MODIFIER = Modifier.weight(1f))
            PalettePreview(palette.colors)
          }
        }
      }
    }

    Spacer(modifier = Modifier.height(8.dp))
    HorizontalDivider(thickness = 1.5.dp, color = GameBoyColors.Green, modifier = Modifier.fillMaxWidth())
    Spacer(modifier = Modifier.height(8.dp))

    Row(
      verticalAlignment = Alignment.CenterVertically,
      modifier          = Modifier.fillMaxWidth())
    {
      CustomText(stringResource(R.string.shader), FONT_SIZE = 16, MODIFIER = Modifier.padding(end = 6.dp))
      HorizontalDivider(thickness = 1.dp, color = GameBoyColors.MediumGreen, modifier = Modifier.weight(1f))
    }

    Spacer(modifier = Modifier.height(4.dp))

    Box(modifier = Modifier.height(110.dp))
    {
      LazyColumn()
      {
        itemsIndexed(shaders)
        { index, shader ->
          CustomText(
            shader,
            FONT_SIZE = 12,
            MODIFIER  = Modifier
              .fillMaxWidth()
              .clickable { ON_SHADER_SELECT(index) }
              .background(
                if (index == SHADER_INDEX)  GameBoyColors.MediumGreen
                else                        Color.Transparent)
              .padding(6.dp))
        }
      }
    }
  }
}

/**
 * Horizontal layout indicator bar that maps out a color preview track.
 *
 * Parses an array of string hashes to render miniature blocks showing the hex shades of an emulator palette.
 *
 * @param COLORS Array list containing hexadecimal color string representations to plot.
 */
@Composable
private fun PalettePreview(COLORS: List<String>)
{
  Row(horizontalArrangement = Arrangement.spacedBy(2.dp))
  {
    COLORS.forEach()
    { colorHex ->
      Box(
        modifier = Modifier
          .size(14.dp)
          .background(Color(colorHex.toColorInt())))
    }
  }
}

/**
 * Interactive serialization interface block managing real-time system save states.
 *
 * Generates an array index map tracking snapshot points. Reads linked storage files to display
 * visual screen capture blocks and calendar date identifiers alongside the write and load triggers.
 *
 * @param EMULATOR The host view model tracking background memory streams.
 */
@Composable
private fun SaveStateSection(EMULATOR: EmulatorViewModel)
{
  val game          by EMULATOR.currentGame.collectAsState()
  val gameId        = game?.id ?: return
  val states        by App.appModule.saveStateManager.getSaveStatesForGame(gameId).collectAsState(initial = emptyList())
  var selectedSlot  by remember { mutableIntStateOf(-1) }

  Column(horizontalAlignment = Alignment.CenterHorizontally)
  {
    LazyRow(
      horizontalArrangement = Arrangement.SpaceEvenly,
      contentPadding        = PaddingValues(horizontal = 16.dp),
      modifier              = Modifier.fillMaxWidth())
    {
      items(5)
      { index ->
        val slot            = index + 1
        val state           = states.find { it.slot == slot }
        val screenshotFile  = App.appModule.saveStateManager.getScreenshotFile(gameId, slot)

        Column(
          horizontalAlignment = Alignment.CenterHorizontally,
          modifier            = Modifier
            .width(100.dp)
            .clickable { selectedSlot = slot }
            .border(
              if (selectedSlot == slot) 2.dp
              else                      1.dp,
              if (selectedSlot == slot) GameBoyColors.Green
              else                      GameBoyColors.MediumGreen,
              RectangleShape)
            .background(
              if (selectedSlot == slot) GameBoyColors.MediumGreen
              else                      Color.Transparent)
            .padding(2.dp))
        {
          Box(
            modifier = Modifier
              .fillMaxWidth()
              .aspectRatio(160f / 144f)
              .background(GameBoyColors.DarkGreen),
            contentAlignment = Alignment.Center)
          {
            if (screenshotFile.exists())
            {
              AsyncImage(
                model               = ImageRequest.Builder(LocalContext.current)
                  .data(screenshotFile)
                  .memoryCacheKey("${screenshotFile.absolutePath}_${state?.timestamp ?: 0}")
                  .build(),
                contentDescription  = null,
                modifier            = Modifier.fillMaxSize(),
                contentScale        = ContentScale.Fit)
            }
            else
            { CustomText(stringResource(R.string.empty), FONT_SIZE = 8) }
          }
          if (state != null)
          {
            val date    = Date(state.timestamp)
            val timeStr = DateFormat.format("MM/dd HH:mm", date).toString()
            CustomText(stringResource(R.string.slot_format, slot, timeStr), FONT_SIZE = 9, COLOR = GameBoyColors.Green)
          }
          else
          { CustomText(stringResource(R.string.slot_only, slot), FONT_SIZE = 10, COLOR = GameBoyColors.LightGreen) }
        }
      }
    }

    Spacer(modifier = Modifier.height(12.dp))

    Row(
      horizontalArrangement = Arrangement.spacedBy(12.dp),
      modifier              = Modifier.fillMaxWidth())
    {
      CustomButton(
        ON_CLICK  = { if (selectedSlot != -1) EMULATOR.saveState(selectedSlot) },
        MODIFIER  = Modifier.weight(1f),
        COLOR     =
          if (selectedSlot != -1) GameBoyColors.MediumGreen
          else                    GameBoyColors.DarkGreen)
      { CustomText(stringResource(R.string.save), FONT_SIZE = 14) }

      CustomButton(
        ON_CLICK  = { if (selectedSlot != -1) EMULATOR.loadState(selectedSlot) },
        MODIFIER  = Modifier.weight(1f),
        COLOR     =
          if (selectedSlot != -1 && states.any { it.slot == selectedSlot }) GameBoyColors.MediumGreen
          else                                                              GameBoyColors.DarkGreen)
      { CustomText(stringResource(R.string.load), FONT_SIZE = 14) }
    }
  }
}

/**
 * Compilation layout block serving standalone auxiliary system operational modifiers.
 *
 * Bundles click utilities mapping clock rate multipliers (Fast Forward toggling), application viewport configurations
 * (Immersive full-screen overrides), game collection bookmarks, and safe persistent battery SRAM memory flushes.
 *
 * @param EMULATOR The host view model parsing background system adjustment operations.
 */
@Composable
private fun MiscSettingsSection(EMULATOR: EmulatorViewModel)
{
  val fastForward by EMULATOR.fastForward.collectAsState()
  val game        by EMULATOR.currentGame.collectAsState()
  
  val infiniteTransition = rememberInfiniteTransition(label = "ff_blink")
  val ffAlpha by infiniteTransition.animateFloat(
    initialValue = 1f,
    targetValue = 0.4f,
    animationSpec = infiniteRepeatable(
      animation = tween(500, easing = LinearEasing),
      repeatMode = RepeatMode.Reverse
    ),
    label = "ff_alpha"
  )

  Column(verticalArrangement = Arrangement.spacedBy(8.dp))
  {
    CustomButton(
      ON_CLICK = { EMULATOR.toggleFastForward() },
      MODIFIER = Modifier.fillMaxWidth())
    {
      Row(
        modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.Start)
      {
        Icon(
          painter = painterResource(R.drawable.fastforward),
          contentDescription = null,
          tint = (if (fastForward) GameBoyColors.Green else GameBoyColors.DarkGreen).copy(alpha = if (fastForward) ffAlpha else 1f),
          modifier = Modifier.size(24.dp))
        
        Spacer(modifier = Modifier.width(12.dp))
        
        CustomText(
          if (fastForward) stringResource(R.string.speed_2x)
          else             stringResource(R.string.speed_1x),
          FONT_SIZE = 14, MODIFIER = Modifier,
          COLOR = GameBoyColors.LightGreen)
      }
    }

    val settings by EMULATOR.settings.collectAsState()
    CustomButton(
      ON_CLICK = { EMULATOR.toggleImmersiveMode() },
      MODIFIER = Modifier.fillMaxWidth())
    {
      Row(
        modifier = Modifier.fillMaxWidth().padding(horizontal = 8.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.Start) {
        Icon(painterResource(R.drawable.gameboy), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
        Spacer(Modifier.width(12.dp))
        CustomText(
          if (settings.isImmersiveModeEnabled)  stringResource(R.string.immersive_on)
          else                                  stringResource(R.string.immersive_off),
          FONT_SIZE = 14, MODIFIER = Modifier)
      }
    }

    CustomButton(
      ON_CLICK = { EMULATOR.takeScreenshot() },
      MODIFIER = Modifier.fillMaxWidth())
    {
      Row(
        modifier = Modifier.fillMaxWidth().padding(horizontal = 8.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.Start) {
        Icon(painterResource(R.drawable.camera), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
        Spacer(Modifier.width(12.dp))
        CustomText(stringResource(R.string.SCREENSHOT), FONT_SIZE = 14, MODIFIER = Modifier)
      }
    }

    CustomButton(
      ON_CLICK = { EMULATOR.openScreenshots() },
      MODIFIER = Modifier.fillMaxWidth())
    {
      Row(
        modifier = Modifier.fillMaxWidth().padding(horizontal = 8.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.Start) {
        Icon(painterResource(R.drawable.camera), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
        Spacer(Modifier.width(12.dp))
        CustomText(stringResource(R.string.VIEW_SCREENSHOTS), FONT_SIZE = 14, MODIFIER = Modifier)
      }
    }

    if (game != null)
    {
      CustomButton(
        ON_CLICK = { EMULATOR.toggleFavorite() },
        MODIFIER = Modifier.fillMaxWidth())
      {
        Row(
          modifier = Modifier.fillMaxWidth().padding(horizontal = 8.dp),
          verticalAlignment = Alignment.CenterVertically,
          horizontalArrangement = Arrangement.Start) {
          Icon(painterResource(R.drawable.heart), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
          Spacer(Modifier.width(12.dp))
          CustomText(
            if (game!!.isFavorite)  stringResource(R.string.REMOVE_FAV)
            else                    stringResource(R.string.ADD_FAV),
            FONT_SIZE = 14, MODIFIER = Modifier)
        }
      }
    }

    CustomButton(
      ON_CLICK  = { App.appModule.gameBoy.deleteRamFile() },
      MODIFIER  = Modifier.fillMaxWidth(),
      COLOR     = GameBoyColors.Error)
    { 
      Row(
        modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.Start) {
        Icon(painterResource(R.drawable.trash), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
        Spacer(Modifier.width(12.dp))
        CustomText(stringResource(R.string.DELTE_SAV), FONT_SIZE = 14, MODIFIER = Modifier) 
      }
    }
  }
}
