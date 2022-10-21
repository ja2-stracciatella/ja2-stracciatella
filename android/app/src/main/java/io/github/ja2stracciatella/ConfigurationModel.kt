package io.github.ja2stracciatella

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import kotlinx.serialization.KSerializer
import kotlinx.serialization.Serializable
import kotlinx.serialization.SerializationException
import kotlinx.serialization.descriptors.PrimitiveKind
import kotlinx.serialization.descriptors.PrimitiveSerialDescriptor
import kotlinx.serialization.descriptors.SerialDescriptor
import kotlinx.serialization.encoding.Decoder
import kotlinx.serialization.encoding.Encoder

enum class VanillaVersion(val value: String) {
    DUTCH("DUTCH"),
    ENGLISH("ENGLISH"),
    FRENCH("FRENCH"),
    GERMAN("GERMAN"),
    ITALIAN("ITALIAN"),
    POLISH("POLISH"),
    RUSSIAN("RUSSIAN"),
    RUSSIAN_GOLD("RUSSIAN_GOLD"),
    SIMPLIFIED_CHINESE("SIMPLIFIED_CHINESE");

    fun getLabel(): String {
        return when (this) {
            DUTCH -> "Dutch"
            ENGLISH -> "English"
            FRENCH -> "French"
            GERMAN -> "German"
            ITALIAN -> "Italian"
            POLISH -> "Polish"
            RUSSIAN -> "Russian"
            RUSSIAN_GOLD -> "Russian (Gold)"
            SIMPLIFIED_CHINESE -> "Simplified Chinese"
        }
    }
}

@Serializable(with = ResolutionSerializer::class)
class Resolution(
    val width: UInt,
    val height: UInt
) {
    companion object {
        val DEFAULT = Resolution(640u, 480u)
    }
}


object ResolutionSerializer : KSerializer<Resolution> {
    override val descriptor: SerialDescriptor = PrimitiveSerialDescriptor("Resolution", PrimitiveKind.STRING)

    override fun serialize(encoder: Encoder, value: Resolution) {
        val width = value.width.toString()
        val height = value.height.toString()
        encoder.encodeString("${width}x${height}")
    }

    override fun deserialize(decoder: Decoder): Resolution {
        val parts = decoder.decodeString().split("x")
        if (parts.size != 2) {
            throw SerializationException("must be in format 640x480")
        }
        val width: UInt
        val height: UInt
        try {
            width = parts[0].toUInt()
            height = parts[1].toUInt()
        } catch (e: NumberFormatException) {
            throw SerializationException("must be in format 640x480")
        }
        return Resolution(width, height)
    }
}

enum class ScalingQuality(val value: String) {
    LINEAR("LINEAR"),
    NEAR_PERFECT("NEAR_PERFECT"),
    PERFECT("PERFECT");

    fun getLabel(): String {
        return when (this) {
            LINEAR -> "Linear Interpolation"
            NEAR_PERFECT -> "Near perfect with oversampling"
            PERFECT -> "Pixel perfect centered"
        }
    }

    companion object {
        val DEFAULT = ScalingQuality.LINEAR
    }
}

class ConfigurationModel : ViewModel() {

    val vanillaGameDir = MutableLiveData<String?>()
    val vanillaGameVersion = MutableLiveData<VanillaVersion?>()
    val saveGameDir = MutableLiveData<String?>()
    val resolution = MutableLiveData(Resolution.DEFAULT)
    val scalingQuality = MutableLiveData(ScalingQuality.DEFAULT)

    fun setVanillaGameDir(vanillaGameDirSet: String?) {
        vanillaGameDir.value = vanillaGameDirSet
    }

    fun setVanillaGameVersion(version: VanillaVersion?) {
        vanillaGameVersion.value = version
    }

    fun setSaveGameDir(saveGameDirSet: String?) {
        saveGameDir.value = saveGameDirSet
    }

    fun setResolution(res: Resolution) {
        resolution.value = res
    }

    fun setScalingQuality(quality: ScalingQuality) {
        scalingQuality.value = quality
    }
}
