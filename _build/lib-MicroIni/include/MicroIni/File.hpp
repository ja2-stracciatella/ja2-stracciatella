#ifndef MICROINI_FILE_HPP
#define MICROINI_FILE_HPP

#include <iostream>
#include <MicroIni/Section.hpp>

namespace MicroIni
{

/// Class for reading and writing INI files.
class MICROINI_API File : public Container<Section>
{
    public:
        /// Read from a stream.
        void read(std::istream& stream);

        /// \brief Read from a file.
        /// \return True if reading was successful, false otherwise.
        bool load(const std::string& file);

        /// Write to a stream.
        void write(std::ostream& stream, const std::string& delimiter = "=") const;

        /// \brief Write to a file.
        /// \return True if writing was successful, false otherwise.
        bool save(const std::string& file, const std::string& delimiter = "=") const;

    private:
        void parseLine(std::string line);
};

} // namespace MicroIni

#endif // MICROINI_FILE_HPP
