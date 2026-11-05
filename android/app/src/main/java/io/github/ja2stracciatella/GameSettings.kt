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
    val description: String?,
    val type: GameSettingType,
    val defaultValue: JsonPrimitive,
    private val minimum: Double?,
    private val maximum: Double?,
    private val pattern: Regex?
) {
    // JSON pointer (RFC 6901) of the setting, as used in the patch file
    val pointer: String = path.joinToString("") { "/" + it.replace("~", "~0").replace("/", "~1") }

    // Returns null if the input is not a valid value for the setting according to its schema
    fun parse(input: String): JsonPrimitive? {
        return when (type) {
            GameSettingType.BOOLEAN -> input.toBooleanStrictOrNull()?.let { JsonPrimitive(it) }
            GameSettingType.INTEGER -> input.trim().toLongOrNull()
                ?.takeIf { isInRange(it.toDouble()) }
                ?.let { JsonPrimitive(it) }
            GameSettingType.DECIMAL -> input.trim().toDoubleOrNull()
                ?.takeIf { it.isFinite() && isInRange(it) }
                ?.let { JsonPrimitive(it) }
            // Patterns in json schemas are not anchored
            GameSettingType.TEXT -> input.takeIf { pattern?.containsMatchIn(it) ?: true }
                ?.let { JsonPrimitive(it) }
        }
    }

    // A text describing the allowed values, or null if any value of the type is allowed.
    // Ranges that only reflect the storage type (e.g. int32) are not meaningful to players and are omitted.
    fun rangeText(): String? {
        if (minimum != null && maximum != null && Pair(minimum, maximum) in storageTypeRanges) {
            return null
        }
        return when {
            minimum != null && maximum != null -> "${formatLimit(minimum)} – ${formatLimit(maximum)}"
            minimum != null -> "≥ ${formatLimit(minimum)}"
            maximum != null -> "≤ ${formatLimit(maximum)}"
            else -> null
        }
    }

    private fun isInRange(value: Double): Boolean {
        return (minimum == null || value >= minimum) && (maximum == null || value <= maximum)
    }

    private fun formatLimit(limit: Double): String {
        return if (limit % 1.0 == 0.0) limit.toLong().toString() else limit.toString()
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

    private companion object {
        // The ranges of the integer types in the schema's types dir (int8.schema.yaml etc.)
        val storageTypeRanges = setOf(
            Pair(Byte.MIN_VALUE.toDouble(), Byte.MAX_VALUE.toDouble()),
            Pair(Short.MIN_VALUE.toDouble(), Short.MAX_VALUE.toDouble()),
            Pair(Int.MIN_VALUE.toDouble(), Int.MAX_VALUE.toDouble()),
            Pair(0.0, UByte.MAX_VALUE.toDouble()),
            Pair(0.0, UShort.MAX_VALUE.toDouble()),
            Pair(0.0, UInt.MAX_VALUE.toDouble())
        )
    }
}

class GameSettingsSection(
    val title: String?,
    val settings: List<GameSetting>
)

/**
 * The settings from game.json that can be changed in the launcher.
 *
 * The settings, their titles, descriptions and allowed values are taken from the json schema
 * of game.json, which is compiled into the game library. Settings that are not described there
 * are not shown. The defaults and the order of the settings are read from game.json in the APK assets,
 * as the schema has neither. Changed values are written
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
        private const val gameJsonSchemaPath = "game.json"
        private const val patchFilename = ".ja2/data/game.patch.json"
        private val jsonFormat = Json {
            prettyPrint = true
        }

        fun load(context: Context): GameSettings {
            val text = context.assets.open(gameJsonAsset).bufferedReader().use { it.readText() }
            val gameJson = Json.parseToJsonElement(stripComments(text)).jsonObject
            val schemaText = try {
                NativeSchemas.getSchemaForPath(gameJsonSchemaPath)
            } catch (e: LinkageError) {
                // The game library could not be loaded
                throw IOException("Could not read the schema of $gameJsonSchemaPath: ${e.message}", e)
            } ?: throw IOException("No schema for $gameJsonSchemaPath")
            val schema = Json.parseToJsonElement(schemaText).jsonObject
            val sections = buildSections(gameJson, schema)
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
                            // The setting was removed, moved or changed its type or allowed values since the value
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

        // The order of the properties is lost when the schema is compiled into the library, so follow game.json
        private fun buildSections(gameJson: JsonObject, schema: JsonObject): List<GameSettingsSection> {
            val sections = mutableListOf<GameSettingsSection>()
            var topLevelSettings = mutableListOf<GameSetting>()
            for ((key, value) in gameJson) {
                val propertySchema = propertySchema(schema, key) ?: continue
                if (value is JsonObject) {
                    if (topLevelSettings.isNotEmpty()) {
                        sections.add(GameSettingsSection(null, topLevelSettings))
                        topLevelSettings = mutableListOf()
                    }
                    val settings = mutableListOf<GameSetting>()
                    collectSettings(listOf(key), null, value, propertySchema, settings)
                    if (settings.isNotEmpty()) {
                        sections.add(GameSettingsSection(titleOf(key, propertySchema), settings))
                    }
                } else {
                    createSetting(listOf(key), null, value, propertySchema)?.let { topLevelSettings.add(it) }
                }
            }
            if (topLevelSettings.isNotEmpty()) {
                sections.add(GameSettingsSection(null, topLevelSettings))
            }
            return sections
        }

        // Objects below the section level are shown as a title prefix of their settings
        private fun collectSettings(
            path: List<String>,
            titlePrefix: String?,
            json: JsonObject,
            schema: JsonObject,
            settings: MutableList<GameSetting>
        ) {
            for ((key, value) in json) {
                val propertySchema = propertySchema(schema, key) ?: continue
                if (value is JsonObject) {
                    val title = titleOf(key, propertySchema)
                    collectSettings(
                        path + key,
                        titlePrefix?.let { "$it / $title" } ?: title,
                        value,
                        propertySchema,
                        settings
                    )
                } else {
                    createSetting(path + key, titlePrefix, value, propertySchema)?.let { settings.add(it) }
                }
            }
        }

        // Arrays, nulls and values that do not match the type in the schema are not supported,
        // the launcher leaves them untouched
        private fun createSetting(
            path: List<String>,
            titlePrefix: String?,
            value: JsonElement,
            schema: JsonObject
        ): GameSetting? {
            if (value !is JsonPrimitive || value.contentOrNull == null) {
                return null
            }
            val content = value.content
            val type = when (stringOf(schema, "type")) {
                "boolean" -> GameSettingType.BOOLEAN.takeIf { !value.isString && content.toBooleanStrictOrNull() != null }
                "integer" -> GameSettingType.INTEGER.takeIf { !value.isString && content.toLongOrNull() != null }
                "number" -> when {
                    value.isString -> null
                    // The game reads some numbers as integers, keep them integers
                    content.toLongOrNull() != null -> GameSettingType.INTEGER
                    content.toDoubleOrNull() != null -> GameSettingType.DECIMAL
                    else -> null
                }
                "string" -> GameSettingType.TEXT.takeIf { value.isString }
                else -> null
            } ?: return null
            val title = titleOf(path.last(), schema)
            return GameSetting(
                path,
                titlePrefix?.let { "$it / $title" } ?: title,
                stringOf(schema, "description")?.let { descriptionWithoutType(it) },
                type,
                value,
                (schema["minimum"] as? JsonPrimitive)?.content?.toDoubleOrNull(),
                (schema["maximum"] as? JsonPrimitive)?.content?.toDoubleOrNull(),
                stringOf(schema, "pattern")?.let { Regex(it) }
            )
        }

        // The schema build appends "Type: <title of the referenced type>" to descriptions (see build.rs),
        // which is meant for modders, not players
        private val typeLine = Regex("""\n+Type: [^\n]*\s*$""")

        private fun descriptionWithoutType(description: String): String? {
            return description.replace(typeLine, "").trim().ifEmpty { null }
        }

        private fun propertySchema(schema: JsonObject, key: String): JsonObject? {
            return (schema["properties"] as? JsonObject)?.get(key) as? JsonObject
        }

        private fun stringOf(schema: JsonObject, name: String): String? {
            return (schema[name] as? JsonPrimitive)?.takeIf { it.isString }?.content
        }

        private fun titleOf(key: String, schema: JsonObject): String {
            return stringOf(schema, "title") ?: key.replace('_', ' ').replaceFirstChar { it.uppercase() }
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
