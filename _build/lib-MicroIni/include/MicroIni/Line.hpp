#ifndef MICROINI_LINE_HPP
#define MICROINI_LINE_HPP

#include <MicroIni/Value.hpp>

namespace MicroIni
{

/// A simple line.
class MICROINI_API Line : public std::pair<std::string, Value>
{
    public:
        /// Constructor.
        Line(const std::string& name, const Value& value);

        /// Test whether the line is a blank line or not.
        bool isBlank() const;

        /// Test whether the line is a comment line or not.
        bool isComment() const;

        /// Test whether the line is a property or not.
        bool isProperty() const;

        /// Get line's name.
        const std::string& getName() const;

        /// Set line's name.
        void setName(const std::string& name);

        /// Get line's Value.
        const Value& getValue() const;

        /// Set line's Value.
        void setValue(const Value& value);
};

/// Make a blank Line.
MICROINI_API Line Blank();

/// Make a comment Line.
MICROINI_API Line Comment(const std::string& comment);

/// Make a property Line.
MICROINI_API Line Property(const std::string& name, const Value& value);

} // namespace MicroIni

#endif // MICROINI_LINE_HPP
