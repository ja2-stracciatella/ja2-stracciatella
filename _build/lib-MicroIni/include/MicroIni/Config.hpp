#ifndef MICROINI_CONFIG_HPP
#define MICROINI_CONFIG_HPP

// import / export macro
#if defined(_WIN32) || defined(__WIN32__)
    #ifndef MICROINI_STATIC
        #ifdef MICROINI_EXPORTS
            #define MICROINI_API __declspec(dllexport)
        #else
            #define MICROINI_API __declspec(dllimport)
        #endif
    #else
        #define MICROINI_API
    #endif
#else
    #define MICROINI_API
#endif

#endif // MICROINI_CONFIG_HPP
