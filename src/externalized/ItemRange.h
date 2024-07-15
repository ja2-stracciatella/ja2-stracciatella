#pragma once

#include <vector>
#include "ItemModel.h"

class ItemRange {
	using Iterator = std::vector<ItemModel const *>::const_iterator;

	public:
		ItemRange(Iterator begin, Iterator end) noexcept : m_begin(begin), m_end(end) {};
		Iterator begin() const noexcept {
			return m_begin;
		};
		Iterator end() const noexcept {
			return m_end;
		};

	private:
		Iterator m_begin;
		Iterator m_end;
};
