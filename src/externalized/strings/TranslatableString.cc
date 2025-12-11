#include "TranslatableString.h"
#include "EncryptedString.h"
#include "Exceptions.h"
#include "Json.h"
#include "Localization.h"
#include "SGPFile.h"
#include <memory>
#include <vector>

namespace TranslatableString {
	const std::vector<ST::string>& Loader::getJsonTranslations(const ST::string& prefix) {
		auto found = m_loadedJson.find(prefix);
		if (found != m_loadedJson.end()) {
			return found->second;
		}

		ST::string fullName = ST::format("{}{}.json", prefix, L10n::GetSuffix(m_version, true));
		auto values = JsonValue::readFromFileWithSchema(m_vfs, m_schemaManager, fullName).toVec();

		std::vector<ST::string> strings;
		strings.reserve(values.size());
		for (const auto& value : values) {
			strings.push_back(value.toString());
		}

		m_loadedJson[prefix] = strings;
		return m_loadedJson[prefix];
	}

	SGPFile* Loader::getBinaryFile(const ST::string& filename) {
		 auto file =  SGPFile::openInVfs(m_vfs, filename);

		m_openFiles[filename] = std::move(file);

		return m_openFiles[filename];
	}

	std::unique_ptr<String> String::parse(JsonValue value) {
		if (value.isString()) {
			return std::make_unique<Untranslated>(value.toString());
		}
		auto obj = value.toObject();
		auto type = obj.GetString("type");
		if (type == "json") {
			return std::make_unique<Json>(obj.GetString("prefix"), obj.GetUInt("index"));
		}
		if (type == "binary") {
			return std::make_unique<Binary>(obj.GetString("file"), obj.GetUInt("offset"), obj.GetUInt("length"));
		}
		throw DataError(ST::format("Unknown translatable string type {}", type));
	}

	ST::string Untranslated::resolve(Loader& loader) {
		return m_str;
	}

	ST::string Json::resolve(Loader& loader) {
		auto translations = loader.getJsonTranslations(m_prefix);
		if (translations.size() <= m_index) {
			throw DataError(ST::format(
				"Translation file {} has only {} entries for the current language, while requested index was {}",
				m_prefix,
				translations.size(),
				m_index
			));
		}
		return translations[m_index];
	}

	ST::string Binary::resolve(Loader& loader) {
		auto file = loader.getBinaryFile(m_file);
		return LoadEncryptedString(file, m_offset, m_length);
	}
}
