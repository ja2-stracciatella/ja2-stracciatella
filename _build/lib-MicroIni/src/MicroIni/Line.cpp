#include <MicroIni/Line.hpp>

namespace MicroIni
{

Line::Line(const std::string& name, const Value& value)
 : std::pair<std::string, Value>(name, value)
{}

bool Line::isBlank() const
{
    return getName().empty();
}

bool Line::isComment() const
{
    return getName() == ";";
}

bool Line::isProperty() const
{
    return (!isBlank()) && (!isComment());
}

const std::string& Line::getName() const
{
    return first;
}

void Line::setName(const std::string& name)
{
    first = name;
}

const Value& Line::getValue() const
{
    return second;
}

void Line::setValue(const Value& value)
{
    second = value;
}

Line Blank()
{
    return Line("", "");
}

Line Comment(const std::string& comment)
{
    return Line(";", comment);
}

Line Property(const std::string& name, const Value& value)
{
    return Line(name, value);
}

} // namespace MicroIni
