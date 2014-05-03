#ifndef MICROINI_VALUE_HPP
#define MICROINI_VALUE_HPP

#include <MicroIni/Config.hpp>
#include <string>
#include <sstream>

namespace MicroIni
{

/// \brief A property value.
///
/// It is stored as a string and can be easily converted.
class MICROINI_API Value : public std::string
{
    public:
        /// Constructor.
        Value(const std::string& value = std::string());

        /// \brief Template constructor.
        ///
        /// Convert the input to a string with a stringstream.
        template <typename T>
        Value(const T& value);

        /// \brief Template cast operator.
        ///
        /// Convert the value to the desired type with a stringstream.
        template <typename T>
        operator T() const;
};

#include <MicroIni/Value.inl>

} // namespace MicroIni

#endif // MICROINI_VALUE_HPP
