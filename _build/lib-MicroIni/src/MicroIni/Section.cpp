#include <MicroIni/Section.hpp>

namespace MicroIni
{

Section::Section(const std::string& name, const Properties& properties)
 : std::pair<std::string, Properties>(name, properties)
{}

const std::string& Section::getName() const
{
    return first;
}

void Section::setName(const std::string& name)
{
    first = name;
}

const Properties& Section::getProperties() const
{
    return second;
}

Properties& Section::getProperties()
{
    return second;
}

} // namespace MicroIni
