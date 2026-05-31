package just.somebody.templates.domain.models

import kotlinx.serialization.Serializable

@Serializable
data class Palette(
    val name: String,
    val colors: List<String> // Hex strings like "#d1cb95"
)

val PRESET_PALETTES = listOf(
    Palette("Default (Standard)", listOf("#d1cb95", "#40985e", "#1a644e", "#04373b")),
    Palette("Classic", listOf("#9aa13c", "#6c712a", "#4d511e", "#1f200c")),
    Palette("Fizzle", listOf("#cee5ff", "#c589dc", "#564991", "#1e182a")),
    Palette("Ice cream", listOf("#fff6d3", "#f9a875", "#eb6b6f", "#7c3f58")),
    Palette("Hollow", listOf("#fafbf6", "#c6b7be", "#565a75", "#0f0f1b")),
    Palette("Rustic", listOf("#edb4a1", "#a96868", "#764462", "#2c2137")),
    Palette("Mint", listOf("#c4f0c2", "#5ab9a8", "#1e606e", "#2d1b00")),
    Palette("SpaceHaze", listOf("#f8e3c4", "#cc3495", "#6b1fb1", "#0b0630")),
    Palette("Fiery Plague", listOf("#713141", "#512839", "#312137", "#1a2129")),
    Palette("Gold", listOf("#cfab51", "#9d654c", "#4d222c", "#210b1b")),
    Palette("Honey", listOf("#e9f5da", "#f0b695", "#877286", "#3e3a42")),
    Palette("Coral", listOf("#ffd0a4", "#f4949c", "#7c9aac", "#68518a")),
    Palette("Rabbit", listOf("#f1e0cd", "#ffa49a", "#da3467", "#35333f")),
    Palette("Caramel autumn", listOf("#fff4b8", "#ff8b40", "#a22fc9", "#290143")),
    Palette("Snow flake", listOf("#e7edeb", "#8ecece", "#62a1c7", "#3f6ecc")),
    Palette("Lemon and Lime", listOf("#fff37b", "#5fcc86", "#39809c", "#28375b")),
    Palette("Kirokaze", listOf("#e2f3e4", "#94e344", "#46878f", "#332c50")),
    Palette("Red is dead", listOf("#fffcfe", "#ff0015", "#860020", "#11070a"))
)
