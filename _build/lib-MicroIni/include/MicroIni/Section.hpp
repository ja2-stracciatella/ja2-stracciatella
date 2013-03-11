#ifndef MICROINI_SECTION_HPP
#define MICROINI_SECTION_HPP

#include <MicroIni/Properties.hpp>

namespace MicroIni
{

/// A section in a File.
class MICROINI_API Section : public std::pair<std::string, Properties>
{
    public:
        /// Constructor.
        Section(const std::string& name, const Properties& properties = Properties());

        /// Get section's name.
        const std::string& getName() const;

        /// Set section's name.
        void setName(const std::string& name);
        
        /// Get section's Properties.
        const Properties& getProperties() const;

        /// Access section's Properties.
        Properties& getProperties();
};

} // namespace MicroIni

#endif // MICROINI_SECTION_HPP
