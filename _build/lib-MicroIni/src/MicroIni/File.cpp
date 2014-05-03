#include <MicroIni/File.hpp>
#include <fstream>

namespace MicroIni
{

namespace
{

// remove trailing and leading spaces (or other characters)
std::string trim(std::string string, const std::string& toTrim = " \t\f\n\r\v")
{
    return string.erase(string.find_last_not_of(toTrim) + 1).erase(0, string.find_first_not_of(toTrim));
}

} // namespace

void File::read(std::istream& stream)
{
    // add anonymous section first
    if(empty())
        push_back(Section(""));

    std::string line;
    while(std::getline(stream, line))
        parseLine(line);
}

bool File::load(const std::string& file)
{
    std::ifstream ifs(file.c_str());
    if(!ifs)
        return false;
    else
    {
        clear();
        read(ifs);
        return true;
    }
}

void File::write(std::ostream& stream, const std::string& delimiter) const
{
    if(empty())
        return;

    const_iterator i = begin();
    if(i->getName().empty()) // if the first section is the anonymous section
        goto writeProperties; // do not write its name

    do // for each section
    {
        // write section's name
        stream << "[" << i->getName() << "]" << std::endl;

        // and write section's content
        writeProperties:
        for(Properties::const_iterator j = i->getProperties().begin(); j != i->getProperties().end(); ++j) // for each line
        {
            if(j->isProperty()) // property
            {
                stream << j->getName() << delimiter;
                if(j->getValue().size() != trim(j->getValue()).size()) // need quotes
                    stream << "\"" << j->getValue() << "\"";
                else
                    stream << j->getValue();
            }
            else if(j->isComment()) // comment line
                stream << j->getValue();
            stream << std::endl;
        }
    } while(++i != end());
}

bool File::save(const std::string& file, const std::string& delimiter) const
{
    std::ofstream ofs(file.c_str());
    if(!ofs)
        return false;
    else
    {
        write(ofs, delimiter);
        return true;
    }
}

void File::parseLine(std::string line)
{
    size_t pos;
    std::string string = trim(line);

    if(string.empty()) // blank line
        back().getProperties().push_back(Blank());

    else if(string[0] == ';' || string[0] == '#') // comment
        back().getProperties().push_back(Comment(line));

    else if(string[0] == '[' && (pos = string.find(']')) != std::string::npos) // section
        push_back(Section(trim(string.substr(1, pos - 1))));

    else if((pos = string.find_first_of("=:")) != std::string::npos) // property
        back().getProperties().push_back(Property(trim(string.substr(0, pos)), trim(trim(string.substr(pos + 1)), "\"")));
}

} // namespace MicroIni
