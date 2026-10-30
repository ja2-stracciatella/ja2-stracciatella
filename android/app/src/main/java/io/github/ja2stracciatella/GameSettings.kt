package io.github.ja2stracciatella

import android.content.Context
import android.util.Log
import kotlinx.serialization.SerializationException
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonArray
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.JsonPrimitive
import kotlinx.serialization.json.buildJsonObject
import kotlinx.serialization.json.contentOrNull
import kotlinx.serialization.json.jsonArray
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.put
import java.io.File
import java.io.IOException

enum class GameSettingType {
    BOOLEAN,
    INTEGER,
    DECIMAL,
    TEXT
}

class GameSetting(
    val path: List<String>,
    val title: String,
    val type: GameSettingType,
    val defaultValue: JsonPrimitive
) {
    // JSON pointer (RFC 6901) of the setting, as used in the patch file
    val pointer: String = path.joinToString("") { "/" + it.replace("~", "~0").replace("/", "~1") }

    // Returns null if the input is not a valid value for the type of the setting
    fun parse(input: String): JsonPrimitive? {
        return when (type) {
            GameSettingType.BOOLEAN -> input.toBooleanStrictOrNull()?.let { JsonPrimitive(it) }
            GameSettingType.INTEGER -> input.trim().toLongOrNull()?.let { JsonPrimitive(it) }
            GameSettingType.DECIMAL -> input.trim().toDoubleOrNull()?.takeIf { it.isFinite() }
                ?.let { JsonPrimitive(it) }
            GameSettingType.TEXT -> JsonPrimitive(input)
        }
    }

    fun isSameValue(a: JsonPrimitive, b: JsonPrimitive): Boolean {
        if (type == GameSettingType.DECIMAL) {
            return a.content.toDoubleOrNull() == b.content.toDoubleOrNull()
        }
        return a.isString == b.isString && a.content == b.content
    }

    fun accepts(value: JsonPrimitive): Boolean {
        return value.isString == (type == GameSettingType.TEXT) && parse(value.content) != null
    }
}

class GameSettingsSection(
    val title: String?,
    val settings: List<GameSetting>
)

/**
 * The settings from game.json that can be changed in the launcher.
 *
 * The defaults are read from game.json in the APK assets. Changed values are written
 * as a JSON patch to game.patch.json in the data dir within stracciatella home, which
 * the game applies on top of game.json.
 */
class GameSettings private constructor(
    val sections: List<GameSettingsSection>,
    private val patchFile: File,
    private val overrides: MutableMap<String, JsonPrimitive>,
    // Patch operations that were not written by the launcher, kept as they are
    private val foreignOperations: List<JsonElement>,
    // Whether the patch file needs to be written
    private var dirty: Boolean
) {
    fun valueOf(setting: GameSetting): JsonPrimitive {
        return overrides[setting.pointer] ?: setting.defaultValue
    }

    fun isModified(setting: GameSetting): Boolean {
        return overrides.containsKey(setting.pointer)
    }

    fun set(setting: GameSetting, value: JsonPrimitive) {
        val current = valueOf(setting)
        if (setting.isSameValue(current, value)) {
            return
        }
        if (setting.isSameValue(setting.defaultValue, value)) {
            overrides.remove(setting.pointer)
        } else {
            overrides[setting.pointer] = value
        }
        dirty = true
    }

    fun reset(setting: GameSetting) {
        if (overrides.remove(setting.pointer) != null) {
            dirty = true
        }
    }

    fun resetAll() {
        if (overrides.isNotEmpty()) {
            overrides.clear()
            dirty = true
        }
    }

    fun save() {
        if (!dirty) {
            return
        }
        val operations = foreignOperations + sections.flatMap { it.settings }.mapNotNull { setting ->
            overrides[setting.pointer]?.let { value ->
                // add replaces existing members, but unlike replace does not fail if a mod's game.json lacks the member
                buildJsonObject {
                    put("op", "add")
                    put("path", setting.pointer)
                    put("value", value)
                }
            }
        }
        if (operations.isEmpty()) {
            if (patchFile.exists() && !patchFile.delete()) {
                throw IOException("Could not delete $patchFile")
            }
        } else {
            val parentDir = patchFile.parentFile
            if (parentDir?.exists() != true) {
                parentDir?.mkdirs()
            }
            patchFile.writeText(jsonFormat.encodeToString(JsonArray.serializer(), JsonArray(operations)))
        }
        dirty = false
    }

    companion object {
        private const val logTag = "GameSettings"
        private const val gameJsonAsset = "externalized/game.json"
        private const val patchFilename = ".ja2/data/game.patch.json"
        private val acronyms = setOf("ai", "ap", "cth", "fps", "gui", "imp")
        private val jsonFormat = Json {
            prettyPrint = true
        }

        fun load(context: Context): GameSettings {
            val text = context.assets.open(gameJsonAsset).bufferedReader().use { it.readText() }
            val gameJson = Json.parseToJsonElement(stripComments(text)).jsonObject
            val sections = buildSections(gameJson)
            val settingsByPointer = sections.flatMap { it.settings }.associateBy { it.pointer }

            val patchFile = File(context.filesDir, patchFilename)
            val overrides = mutableMapOf<String, JsonPrimitive>()
            val foreignOperations = mutableListOf<JsonElement>()
            var dirty = false
            if (patchFile.exists()) {
                try {
                    val operations = Json.parseToJsonElement(patchFile.readText()).jsonArray
                    for (operation in operations) {
                        val fields = operation as? JsonObject
                        val op = (fields?.get("op") as? JsonPrimitive)?.contentOrNull
                        val path = (fields?.get("path") as? JsonPrimitive)?.contentOrNull
                        val value = fields?.get("value") as? JsonPrimitive
                        val setting = path?.let { settingsByPointer[it] }
                        val isSetValue = (op == "add" || op == "replace") && path != null
                        if (isSetValue && setting != null && value != null && setting.accepts(value)) {
                            overrides[setting.pointer] = value
                        } else if (isSetValue && (setting != null || !exists(gameJson, path!!))) {
                            // The setting was removed, moved or changed its type in game.json since the value
                            // was saved. The value no longer applies, and could stop the game from starting.
                            Log.i(logTag, "Dropping outdated value for $path from $patchFile")
                            dirty = true
                        } else {
                            foreignOperations.add(operation)
                        }
                    }
                } catch (e: IllegalArgumentException) {
                    // SerializationException is an IllegalArgumentException, as is a patch that is not an array
                    // The game cannot start with an invalid patch, so replace it on the next save
                    Log.w(logTag, "Could not decode $patchFile, it will be replaced: ${e.message}")
                    overrides.clear()
                    foreignOperations.clear()
                    dirty = true
                }
            }
            return GameSettings(sections, patchFile, overrides, foreignOperations, dirty)
        }

        private fun buildSections(gameJson: JsonObject): List<GameSettingsSection> {
            val sections = mutableListOf<GameSettingsSection>()
            var topLevelSettings = mutableListOf<GameSetting>()
            for ((key, value) in gameJson) {
                if (value is JsonObject) {
                    if (topLevelSettings.isNotEmpty()) {
                        sections.add(GameSettingsSection(null, topLevelSettings))
                        topLevelSettings = mutableListOf()
                    }
                    val settings = mutableListOf<GameSetting>()
                    collectSettings(listOf(key), value, settings)
                    if (settings.isNotEmpty()) {
                        sections.add(GameSettingsSection(humanize(key), settings))
                    }
                } else {
                    createSetting(listOf(key), value)?.let { topLevelSettings.add(it) }
                }
            }
            if (topLevelSettings.isNotEmpty()) {
                sections.add(GameSettingsSection(null, topLevelSettings))
            }
            return sections
        }

        private fun collectSettings(path: List<String>, json: JsonObject, settings: MutableList<GameSetting>) {
            for ((key, value) in json) {
                if (value is JsonObject) {
                    collectSettings(path + key, value, settings)
                } else {
                    createSetting(path + key, value)?.let { settings.add(it) }
                }
            }
        }

        // Arrays and nulls are not supported, the launcher leaves them untouched
        private fun createSetting(path: List<String>, value: JsonElement): GameSetting? {
            if (value !is JsonPrimitive || value.contentOrNull == null) {
                return null
            }
            val content = value.content
            val type = when {
                value.isString -> GameSettingType.TEXT
                content == "true" || content == "false" -> GameSettingType.BOOLEAN
                content.toLongOrNull() != null -> GameSettingType.INTEGER
                content.toDoubleOrNull() != null -> GameSettingType.DECIMAL
                else -> return null
            }
            // Settings within an object are shown in a section titled after the object
            val titlePath = if (path.size > 1) path.drop(1) else path
            val title = titlePath.joinToString(" / ") { humanize(it) }
            return GameSetting(path, title, type, value)
        }

        // Whether the JSON pointer (RFC 6901) refers to a value within json
        private fun exists(json: JsonElement, pointer: String): Boolean {
            if (pointer.isEmpty()) {
                return true
            }
            if (!pointer.startsWith("/")) {
                return false
            }
            var current = json
            for (token in pointer.substring(1).split('/')) {
                val name = token.replace("~1", "/").replace("~0", "~")
                val parent = current
                current = when (parent) {
                    is JsonObject -> parent[name]
                    is JsonArray -> name.toIntOrNull()?.let { parent.getOrNull(it) }
                    else -> null
                } ?: return false
            }
            return true
        }

        private fun humanize(key: String): String {
            return key.split('_').filter { it.isNotEmpty() }.mapIndexed { index, word ->
                when {
                    word in acronyms -> word.uppercase()
                    index == 0 -> word.replaceFirstChar { it.uppercase() }
                    else -> word
                }
            }.joinToString(" ")
        }

        // game.json contains comments, which are not valid json
        private fun stripComments(text: String): String {
            val stripped = StringBuilder(text.length)
            var i = 0
            while (i < text.length) {
                val c = text[i]
                if (c == '/' && text.getOrNull(i + 1) == '/') {
                    val end = text.indexOf('\n', i)
                    i = if (end == -1) text.length else end
                } else if (c == '/' && text.getOrNull(i + 1) == '*') {
                    val end = text.indexOf("*/", i + 2)
                    if (end == -1) {
                        throw SerializationException("Unterminated comment")
                    }
                    i = end + 2
                } else if (c == '"') {
                    // Copy strings as they are, they may contain comment markers
                    val end = findStringEnd(text, i)
                    stripped.append(text, i, end + 1)
                    i = end + 1
                } else {
                    stripped.append(c)
                    i++
                }
            }
            return stripped.toString()
        }

        private fun findStringEnd(text: String, start: Int): Int {
            var i = start + 1
            while (i < text.length) {
                when (text[i]) {
                    '\\' -> i += 2
                    '"' -> return i
                    else -> i++
                }
            }
            throw SerializationException("Unterminated string")
        }
    }
}
