#pragma once


#include "GameRes.h"
#include "Json.h"
#include "SGPFile.h"
#include "stracciatella.h"
#include <cstdint>
#include <map>
#include <string_theory/st_format.h>
#include <string_theory/st_string.h>
#include <vector>

namespace TranslatableString {
	/*
	 * Interface for the loader class
	 */
	class Loader {
		public:
			virtual const std::vector<ST::string>& getJsonTranslations(const ST::string& file) = 0;
			virtual SGPFile* getBinaryFile(const ST::string& file) = 0;
	};

	/*
	 * Loader class that can feed the translatable string loading, caching data inbetween calls
	 * Note that it does not own its Vfs and SchemaManager instances and will not free them on delete
	 */
	class FileLoader : public Loader {
		public:
			FileLoader(Vfs* vfs, SchemaManager* schemaManager, GameVersion version) : m_vfs(vfs), m_schemaManager(schemaManager), m_version(version) {}
			~FileLoader() {
				for (auto file : m_openFiles) {
					DeleteSGPFile(file.second);
				}
			}

			const std::vector<ST::string>& getJsonTranslations(const ST::string& file) override;
			SGPFile* getBinaryFile(const ST::string& file) override;
		private:
			Vfs* m_vfs;
			SchemaManager* m_schemaManager;
			GameVersion m_version;

			std::map<ST::string, std::vector<ST::string>> m_loadedJson;
			std::map<ST::string, SGPFile*> m_openFiles;
	};

	namespace Unittests {
		/*
		 * Loader class for unit tests that always returns the same values
		 */
		class TestLoader : public Loader {
			public:
				TestLoader() {
					for (auto i = 0; i < 512; i++) {
						m_jsonTranslations.push_back(ST::format("Translation {}", i));
					}
				}

				const std::vector<ST::string>& getJsonTranslations(const ST::string& file) override {
					return m_jsonTranslations;
				};
				SGPFile* getBinaryFile(const ST::string& file) override {
					return nullptr;
				};
			private:
				std::vector<ST::string> m_jsonTranslations;
		};
	}

	/*
	 * A virtual class for all translatable string mutations
	 */
	class String {
		public:
			virtual ST::string resolve(Loader& loader) = 0;
			static std::unique_ptr<String> parse(JsonValue value);
	};

	/*
	 * An untranslated translatable string
	 */
	class Untranslated : public String {
		public:
			Untranslated(ST::string str) : m_str(str) {}

			ST::string resolve(Loader& loader) override;
		private:
			ST::string m_str;
	};

	/*
	 * A translatable string translated via JSON strings reference
	 */
	class Json : public String {
			public:
				Json(ST::string file, uint32_t index) : m_prefix(file), m_index(index) {}

				ST::string resolve(Loader& loader) override;
			private:
				ST::string m_prefix;
				uint32_t m_index;
	};

	/*
	 * A translatable string translated via a binary file
	 */
	class Binary : public String {
			public:
				Binary(ST::string file, uint32_t offset, uint32_t length) : m_file(file), m_offset(offset), m_length(length) {};

				ST::string resolve(Loader& loader) override;
			private:
				ST::string m_file;
				uint32_t m_offset;
				uint32_t m_length;
	};
}
