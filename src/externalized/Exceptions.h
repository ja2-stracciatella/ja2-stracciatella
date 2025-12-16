#pragma once

#include <stdint.h>
#include <stdexcept>
#include <string_theory/string>

class IoException : public std::runtime_error
{
public:
	IoException(const ST::string &what_arg)
		: std::runtime_error(what_arg.to_std_string()) {}
};

class DataError: public std::runtime_error
{
public:
	DataError(const ST::string& what_arg)
		:std::runtime_error(what_arg.to_std_string()) {}
};

class NotFoundError : public DataError {
public:
	NotFoundError(const ST::string& what_arg) : DataError(what_arg) {}
};
