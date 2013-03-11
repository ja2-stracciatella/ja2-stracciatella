#include <MicroIni/MicroIni.hpp>

int main()
{
    // Create the File object
    MicroIni::File file;

    // Load a file
    if(!file.load("input.ini"))
        return 1;

    // Clear the anonymous section (lines before the first section)
    file[""].clear();

    // Access a property value
    std::cout << file["message"]["hello"] << std::endl;

    // Change the property value
    file["message"]["hello"] = "Hello you!";

    // Add a blank line
    file["message"].push_back(MicroIni::Blank());

    // Create a section and properties
    // Any type convertible by a std::stringstream is assignable
    file["numbers"]["an integer"] = 10;
    file["numbers"]["a float"] = 0.8f;

    // Add a comment
    file["numbers"].push_back(MicroIni::Comment("# this is my comment"));
    
    // Save the file
    if(!file.save("output.ini"))
        return 1;

    return 0;
}
