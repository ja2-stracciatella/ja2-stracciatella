#include "ContentManager.h"
#include "DefaultContentManager.h"
#include "EncryptedString.h"
#include "IEDT.h"
#include "Json.h"
#include "ModPackContentManager.h"
#include "SGPFile.h"
#include <charconv>
#include <memory>
#include <numeric>
#include <optional>
#include <string_view>
#include <vector>
#include "string_theory/string"

namespace {

class ClassicEDT final : public IEDT
{
	std::unique_ptr<SGPFile> underlyingFile;
	std::vector<IEDT::column_list::value_type> columns;
	unsigned rowLength;

public:
	ClassicEDT(ContentManager const& cm,
	           std::string_view const filename,
	           IEDT::column_list columnsList) :
		underlyingFile{ cm.openGameResForReading(filename) },
		columns{ columnsList },
		rowLength{ std::accumulate(columns.begin(), columns.end(), 0U) }
	{
	}

	ST::string at(unsigned const row, unsigned const column) const override
	{
		return LoadEncryptedString(underlyingFile.get(),
			row * rowLength + std::accumulate
				(columns.begin(), columns.begin() + column, 0U),
			columns.at(column));
	}

	~ClassicEDT() override = default;
};


class JsonEDT final : public IEDT
{
	template<std::size_t TN>
	static char * conv(char (&buf)[TN], unsigned const value)
	{
		auto const convResult{ std::to_chars(buf, buf + TN, value) };
		// to_chars does not NUL terminate its result, fix that here.
		*convResult.ptr = 0;
		return buf;
	}

	std::optional<ST::string> attemptToGet(unsigned const row, unsigned const column) const
	{
		char buf[64];
		if (jsonObject.has(conv(buf, row)))
		{
			auto const rowObject{ jsonObject.GetValue(buf).toObject() };
			if (rowObject.has(conv(buf, column)))
			{
				return rowObject.GetString(buf);
			}
		}

		return std::nullopt;
	}

	ClassicEDT backupEDT;
	JsonObject jsonObject;

public:
	JsonEDT(ContentManager const& cm,
	        std::string_view const filename,
	        IEDT::column_list columnsList)
		: backupEDT{ cm, filename, columnsList }
	{
		auto const jsonFilename{ ST::string{filename} + ".json" };
		if (cm.doesGameResExists(jsonFilename))
		{
			std::unique_ptr<SGPFile> jsonfile{ cm.openGameResForReading(jsonFilename) };
			jsonObject = JsonValue::deserialize(jsonfile->readStringToEnd()).toObject();
		}
	}

	ST::string at(unsigned const row, unsigned const column) const override
	{
		auto const result{ attemptToGet(row, column) };
		// Not using result.value_or here because that would make us
		// always call backupEDT.at().
		return result ? *result : backupEDT.at(row, column);
	}

	~JsonEDT() override = default;
};
} // anonymous namespace


IEDT::uptr DefaultContentManager::openEDT(std::string_view const filename,
	IEDT::column_list columns) const
{
	return IEDT::uptr{ new ClassicEDT(*this, filename, columns) };
}


IEDT::uptr ModPackContentManager::openEDT(std::string_view const filename,
	IEDT::column_list columns) const
{
	return IEDT::uptr{ new JsonEDT(*this, filename, columns) };
}
