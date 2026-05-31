package just.somebody.templates.presentation.widgets

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
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import coil.compose.AsyncImage
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.domain.GameBoy
import just.somebody.templates.domain.models.PRESET_PALETTES
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.ui.theme.GameBoyColors
import java.util.Date
import android.text.format.DateFormat
import androidx.core.graphics.toColorInt

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsPanel(
    GAME_BOY: GameBoy,
    EMULATOR: EmulatorViewModel,
    ON_CLOSE: () -> Unit,
    MODIFIER: Modifier = Modifier
) {
    var settingsPage by remember { mutableIntStateOf(0) }
    val settings by EMULATOR.settings.collectAsState()

    ModalBottomSheet(
        onDismissRequest = ON_CLOSE,
        containerColor = GameBoyColors.DarkGreen,
        shape = RectangleShape,
        dragHandle = { BottomSheetDefaults.DragHandle(color = GameBoyColors.Green) }
    ) {
        Column(
            modifier = Modifier
                .padding(horizontal = 16.dp, vertical = 8.dp)
                .fillMaxWidth(),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            CustomText("Game Settings", FONT_SIZE = 20)
            Spacer(modifier = Modifier.height(4.dp))
            HorizontalDivider(thickness = 1.dp, color = GameBoyColors.MediumGreen, modifier = Modifier.fillMaxWidth(0.4f))
            Spacer(modifier = Modifier.height(8.dp))

            TabRow(
                selectedTabIndex = settingsPage,
                containerColor = Color.Transparent,
                contentColor = GameBoyColors.LightGreen,
                divider = {},
                indicator = { tabPositions ->
                    TabRowDefaults.SecondaryIndicator(
                        Modifier.tabIndicatorOffset(tabPositions[settingsPage]),
                        color = GameBoyColors.Green
                    )
                }
            ) {
                val tabs = listOf("Audio", "Visual", "States", "Misc")
                tabs.forEachIndexed { index, title ->
                    Tab(selected = settingsPage == index, onClick = { settingsPage = index }) {
                        CustomText(title, FONT_SIZE = 12, MODIFIER = Modifier.padding(6.dp))
                    }
                }
            }

            Spacer(modifier = Modifier.height(12.dp))

            Box(modifier = Modifier.height(280.dp)) {
                LazyColumn(
                    modifier = Modifier.fillMaxSize(),
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.Top
                ) {
                    item {
                        when (settingsPage) {
                            0 -> AudioSettingsSection(settings.channelVolume) { vol, ch -> EMULATOR.setVolume(vol, ch) }
                            1 -> VisualSettingsSection(
                                paletteIndex = settings.paletteIndex,
                                shaderIndex = settings.shaderIndex,
                                onPaletteSelect = { EMULATOR.setPaletteIndex(it) },
                                onShaderSelect = { EMULATOR.setShaderIndex(it) }
                            )
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

@Composable
private fun AudioSettingsSection(
    volumes: List<Float>,
    onVolumeChange: (Float, Int) -> Unit
) {
    val labels = listOf("CH1", "CH2", "Wave", "Noise")
    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
        labels.forEachIndexed { index, label ->
            Column {
                CustomText(label, FONT_SIZE = 12, MODIFIER = Modifier.padding(bottom = 2.dp))
                RetroSlider(
                    VALUE = volumes[index + 1],
                    ON_VALUE_CHANGE = { onVolumeChange(it, index + 1) },
                    MODIFIER = Modifier.fillMaxWidth()
                )
            }
        }
    }
}

@Composable
private fun VisualSettingsSection(
    paletteIndex: Int,
    shaderIndex: Int,
    onPaletteSelect: (Int) -> Unit,
    onShaderSelect: (Int) -> Unit
) {
    val shaders = listOf("Sharp Retro", "CRT", "LCD", "Chromatic Aberration", "Default")
    
    Column {
        Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.fillMaxWidth()) {
            CustomText("Palette", FONT_SIZE = 16, MODIFIER = Modifier.padding(end = 6.dp))
            HorizontalDivider(thickness = 1.dp, color = GameBoyColors.MediumGreen, modifier = Modifier.weight(1f))
        }
        
        Spacer(modifier = Modifier.height(4.dp))
        
        Box(modifier = Modifier.height(110.dp)) {
            LazyColumn {
                itemsIndexed(PRESET_PALETTES) { index, palette ->
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clickable { onPaletteSelect(index) }
                            .background(if (index == paletteIndex) GameBoyColors.MediumGreen else Color.Transparent)
                            .padding(6.dp),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        CustomText(palette.name, FONT_SIZE = 12, MODIFIER = Modifier.weight(1f))
                        PalettePreview(palette.colors)
                    }
                }
            }
        }

        Spacer(modifier = Modifier.height(8.dp))
        HorizontalDivider(thickness = 1.5.dp, color = GameBoyColors.Green, modifier = Modifier.fillMaxWidth())
        Spacer(modifier = Modifier.height(8.dp))
        
        Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.fillMaxWidth()) {
            CustomText("Shader", FONT_SIZE = 16, MODIFIER = Modifier.padding(end = 6.dp))
            HorizontalDivider(thickness = 1.dp, color = GameBoyColors.MediumGreen, modifier = Modifier.weight(1f))
        }

        Spacer(modifier = Modifier.height(4.dp))

        Box(modifier = Modifier.height(110.dp)) {
            LazyColumn {
                itemsIndexed(shaders) { index, shader ->
                    CustomText(
                        shader,
                        FONT_SIZE = 12,
                        MODIFIER = Modifier
                            .fillMaxWidth()
                            .clickable { onShaderSelect(index) }
                            .background(if (index == shaderIndex) GameBoyColors.MediumGreen else Color.Transparent)
                            .padding(6.dp)
                    )
                }
            }
        }
    }
}

@Composable
private fun PalettePreview(colors: List<String>) {
    Row(horizontalArrangement = Arrangement.spacedBy(2.dp)) {
        colors.forEach { colorHex ->
            Box(
                modifier = Modifier
                    .size(14.dp)
                    .background(Color(colorHex.toColorInt()))
            )
        }
    }
}

@Composable
private fun SaveStateSection(emulator: EmulatorViewModel) {
    val game by emulator.currentGame.collectAsState()
    val gameId = game?.id ?: return
    val states by App.appModule.saveStateManager.getSaveStatesForGame(gameId).collectAsState(initial = emptyList())
    var selectedSlot by remember { mutableIntStateOf(-1) }
    
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        CustomText("Save Slots", FONT_SIZE = 16)
        Spacer(modifier = Modifier.height(4.dp))
        
        LazyRow(
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            contentPadding = PaddingValues(horizontal = 16.dp),
            modifier = Modifier.fillMaxWidth()
        ) {
            items(5) { index ->
                val slot = index + 1
                val state = states.find { it.slot == slot }
                val screenshotFile = App.appModule.saveStateManager.getScreenshotFile(gameId, slot)
                
                Column(
                    horizontalAlignment = Alignment.CenterHorizontally,
                    modifier = Modifier
                        .width(100.dp)
                        .clickable { selectedSlot = slot }
                        .border(
                            if (selectedSlot == slot) 2.dp else 1.dp,
                            if (selectedSlot == slot) GameBoyColors.Green else GameBoyColors.MediumGreen,
                            RectangleShape
                        )
                        .background(if (selectedSlot == slot) GameBoyColors.MediumGreen else Color.Transparent)
                        .padding(2.dp)
                ) {
                    Box(
                        modifier = Modifier
                            .fillMaxWidth()
                            .aspectRatio(160f / 144f)
                            .background(Color.Black),
                        contentAlignment = Alignment.Center
                    ) {
                        if (screenshotFile.exists()) {
                            AsyncImage(
                                model = screenshotFile,
                                contentDescription = null,
                                modifier = Modifier.fillMaxSize(),
                                contentScale = ContentScale.Fit
                            )
                        } else {
                            CustomText("Empty", FONT_SIZE = 8)
                        }
                    }
                    if (state != null) {
                        val date = Date(state.timestamp)
                        val timeStr = DateFormat.format("MM/dd HH:mm", date).toString()
                        CustomText("Slot $slot - $timeStr", FONT_SIZE = 9, COLOR = GameBoyColors.Green)
                    } else {
                        CustomText("Slot $slot", FONT_SIZE = 10, COLOR = GameBoyColors.LightGreen)
                    }
                }
            }
        }

        Spacer(modifier = Modifier.height(12.dp))
        
        Row(horizontalArrangement = Arrangement.spacedBy(12.dp), modifier = Modifier.fillMaxWidth()) {
            CustomButton(
                ON_CLICK = { if (selectedSlot != -1) emulator.saveState(selectedSlot) },
                MODIFIER = Modifier.weight(1f),
                COLOR = if (selectedSlot != -1) GameBoyColors.MediumGreen else GameBoyColors.DarkGreen
            ) { CustomText("Save", FONT_SIZE = 14) }
            
            CustomButton(
                ON_CLICK = { if (selectedSlot != -1) emulator.loadState(selectedSlot) },
                MODIFIER = Modifier.weight(1f),
                COLOR = if (selectedSlot != -1 && states.any { it.slot == selectedSlot }) GameBoyColors.MediumGreen else GameBoyColors.DarkGreen
            ) { CustomText("Load", FONT_SIZE = 14) }
        }
    }
}

@Composable
private fun MiscSettingsSection(emulator: EmulatorViewModel) {
    val fastForward by emulator.fastForward.collectAsState()
    val game by emulator.currentGame.collectAsState()

    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
        CustomButton(
            ON_CLICK = { 
                emulator.toggleFastForward()
            },
            MODIFIER = Modifier.fillMaxWidth()
        ) {
            CustomText(if (fastForward) "Speed: 2x" else "Speed: 1x", FONT_SIZE = 14)
        }
        
        if (game != null) {
            CustomButton(
                ON_CLICK = { emulator.toggleFavorite() },
                MODIFIER = Modifier.fillMaxWidth()
            ) {
                CustomText(if (game!!.isFavorite) stringResource(R.string.REMOVE_FAV) else stringResource(R.string.ADD_FAV), FONT_SIZE = 14)
            }
        }

        CustomButton(
            ON_CLICK = { App.appModule.gameBoy.deleteRamFile() },
            MODIFIER = Modifier.fillMaxWidth(),
            COLOR = GameBoyColors.Error
        ) {
            CustomText(stringResource(R.string.DELTE_SAV), FONT_SIZE = 14)
        }
    }
}
