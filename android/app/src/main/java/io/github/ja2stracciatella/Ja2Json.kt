package io.github.ja2stracciatella

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

@Serializable
data class Ja2Json(
    @SerialName("game_dir")
    val vanillaGameDir: String? = null,
    @SerialName("resversion")
    val vanillaGameVersion: VanillaVersion? = null,
    @SerialName("save_game_dir")
    val saveGameDir: String? = null,
    @SerialName("res")
    val resolution: Resolution? = null,
    @SerialName("scaling")
    val scalingQuality: ScalingQuality? = null,
    @SerialName("debug")
    val debug: Boolean? = null
)
