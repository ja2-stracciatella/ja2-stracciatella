#pragma once

#include <stdexcept>

class LibraryFileNotFoundException : public std::runtime_error
{
public:
	LibraryFileNotFoundException(const ST::string& what_arg)
		:std::runtime_error(what_arg.to_std_string()) {}
};

class DataError: public std::runtime_error
{
public:
	DataError(const ST::string& what_arg)
		:std::runtime_error(what_arg.to_std_string()) {}
};