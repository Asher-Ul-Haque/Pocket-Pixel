package just.somebody.templates.presentation.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import coil.compose.AsyncImage
import just.somebody.templates.R
import just.somebody.templates.data.entities.AchievementEntity
import just.somebody.templates.presentation.viewModels.AchievementViewModel
import just.somebody.templates.presentation.widgets.CustomButton
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.DeviceSizePreviews
import java.text.SimpleDateFormat
import java.util.*

@Composable
fun AchievementScreen(VIEW_MODEL: AchievementViewModel) {
    val settings by VIEW_MODEL.settings.collectAsState()
    val achievements by VIEW_MODEL.achievements.collectAsState()

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(GameBoyColors.DarkGreen)
            .padding(16.dp)
    ) {
        if (settings.raToken.isEmpty()) {
            LoginContent(ON_LOGIN = { user, token -> VIEW_MODEL.login(user, token) })
        } else {
            ProfileContent(
                USERNAME = settings.raUsername,
                ACHIEVEMENTS = achievements,
                ON_LOGOUT = { VIEW_MODEL.logout() }
            )
        }
    }
}

@Composable
fun LoginContent(ON_LOGIN: (String, String) -> Unit) {
    var username by remember { mutableStateOf("") }
    var token by remember { mutableStateOf("") }

    Column(
        modifier = Modifier.fillMaxSize(),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Icon(
            painter = painterResource(R.drawable.trophy),
            contentDescription = null,
            tint = GameBoyColors.LightGreen,
            modifier = Modifier.size(64.dp)
        )
        Spacer(Modifier.height(16.dp))
        CustomText("RetroAchievements", FONT_SIZE = 24, COLOR = GameBoyColors.LightGreen)
        Spacer(Modifier.height(32.dp))

        RAInput(LABEL = "Username", VALUE = username, ON_VALUE_CHANGE = { username = it })
        Spacer(Modifier.height(16.dp))
        RAInput(LABEL = "Password", VALUE = token, ON_VALUE_CHANGE = { token = it }, IS_PASSWORD = true)
        
        Spacer(Modifier.height(32.dp))
        CustomButton(
            ON_CLICK = { if (username.isNotEmpty() && token.isNotEmpty()) ON_LOGIN(username, token) },
            MODIFIER = Modifier.fillMaxWidth()
        ) {
            CustomText("Login", FONT_SIZE = 18)
        }
        
        Spacer(Modifier.height(16.dp))
        CustomText(
            "Logging in will generate a secure session token",
            FONT_SIZE = 12,
            COLOR = GameBoyColors.MediumGreen,
            MODIFIER = Modifier.padding(horizontal = 16.dp)
        )
    }
}

@Composable
fun ProfileContent(
    USERNAME: String,
    ACHIEVEMENTS: List<AchievementEntity>,
    ON_LOGOUT: () -> Unit
) {
    Column(modifier = Modifier.fillMaxSize()) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column {
                CustomText("User: $USERNAME", FONT_SIZE = 18, COLOR = GameBoyColors.LightGreen)
                CustomText("${ACHIEVEMENTS.size} Achievements", FONT_SIZE = 14, COLOR = GameBoyColors.MediumGreen)
            }
        }

        Spacer(Modifier.height(16.dp))
        HorizontalDivider(color = GameBoyColors.MediumGreen)
        Spacer(Modifier.height(16.dp))

        Box(modifier = Modifier.weight(1f)) {
            LazyColumn(
                modifier = Modifier.fillMaxSize(),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                items(ACHIEVEMENTS) { achievement ->
                    AchievementItem(achievement)
                }
            }
        }

        Spacer(Modifier.height(16.dp))
        CustomButton(
            ON_CLICK = ON_LOGOUT,
            MODIFIER = Modifier.fillMaxWidth(),
            COLOR = GameBoyColors.Error
        ) {
            Row(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.Start
            ) {
                Icon(painterResource(R.drawable.trash), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
                Spacer(Modifier.width(12.dp))
                CustomText("Logout", FONT_SIZE = 14)
            }
        }
    }
}

@Composable
fun AchievementItem(ACHIEVEMENT: AchievementEntity) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .border(2.dp, GameBoyColors.Green)
            .background(GameBoyColors.MediumGreen.copy(alpha = 0.2f))
            .padding(8.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        AsyncImage(
            model = ACHIEVEMENT.badgeUrl,
            contentDescription = null,
            modifier = Modifier
                .size(48.dp)
                .background(GameBoyColors.DarkGreen)
                .border(1.dp, GameBoyColors.Green),
            placeholder = painterResource(R.drawable.trophy)
        )
        Spacer(Modifier.width(12.dp))
        Column(modifier = Modifier.weight(1f)) {
            CustomText(ACHIEVEMENT.title, FONT_SIZE = 14, COLOR = GameBoyColors.LightGreen)
            CustomText(ACHIEVEMENT.description, FONT_SIZE = 10, COLOR = GameBoyColors.Green)
            
            val date = SimpleDateFormat("yyyy-MM-dd", Locale.getDefault()).format(Date(ACHIEVEMENT.unlockDate))
            CustomText("Unlocked: $date", FONT_SIZE = 9, COLOR = GameBoyColors.MediumGreen)
        }
        Column(horizontalAlignment = Alignment.End) {
            CustomText("${ACHIEVEMENT.points}", FONT_SIZE = 14, COLOR = GameBoyColors.LightGreen)
            if (ACHIEVEMENT.isHardcore) {
                CustomText("HC", FONT_SIZE = 10, COLOR = Color.Red)
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun RAInput(
    LABEL: String,
    VALUE: String,
    ON_VALUE_CHANGE: (String) -> Unit,
    IS_PASSWORD: Boolean = false
) {
    Column(modifier = Modifier.fillMaxWidth()) {
        CustomText(LABEL, FONT_SIZE = 14, COLOR = GameBoyColors.LightGreen)
        TextField(
            value = VALUE,
            onValueChange = ON_VALUE_CHANGE,
            modifier = Modifier
                .fillMaxWidth()
                .padding(top = 4.dp)
                .background(GameBoyColors.Green),
            colors = TextFieldDefaults.colors(
                focusedTextColor = GameBoyColors.DarkGreen,
                unfocusedTextColor = GameBoyColors.DarkGreen,
                focusedContainerColor = Color.Transparent,
                unfocusedContainerColor = Color.Transparent,
                cursorColor = GameBoyColors.DarkGreen,
                focusedIndicatorColor = Color.Transparent,
                unfocusedIndicatorColor = Color.Transparent
            ),
            singleLine = true,
            visualTransformation = if (IS_PASSWORD) androidx.compose.ui.text.input.PasswordVisualTransformation() else androidx.compose.ui.text.input.VisualTransformation.None,
            shape = RectangleShape
        )
    }
}

@DeviceSizePreviews
@Composable
fun AchievementScreenProfilePreview() {
    val mockAchievements = listOf(
        AchievementEntity(1, 1, "First Steps", "Started your first game", 5, "", System.currentTimeMillis(), false),
        AchievementEntity(2, 1, "Hardcore Master", "Beat the first boss in HC", 25, "", System.currentTimeMillis(), true)
    )
    Box(modifier = Modifier.fillMaxSize().background(GameBoyColors.DarkGreen)) {
        ProfileContent(
            USERNAME = "Gamer123",
            ACHIEVEMENTS = mockAchievements,
            ON_LOGOUT = {}
        )
    }
}

@DeviceSizePreviews
@Composable
fun AchievementScreenLoginPreview() {
    Box(modifier = Modifier.fillMaxSize().background(GameBoyColors.DarkGreen)) {
        LoginContent(ON_LOGIN = { _, _ -> })
    }
}
