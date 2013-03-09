#pragma once

#include <stdexcept>


class LibraryFileNotFoundException : public std::runtime_error
{
public:
  LibraryFileNotFoundException(const std::string& what_arg)
    :std::runtime_error(what_arg)
  {
  }
};
