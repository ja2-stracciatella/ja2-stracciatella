#ifndef MICROINI_CONTAINER_HPP
#define MICROINI_CONTAINER_HPP

#include <MicroIni/Config.hpp>
#include <vector>

namespace MicroIni
{

/// Kind of multimap that keeps the order of insertion.
template <typename pair>
class MICROINI_API Container : public std::vector<pair>
{
    public:
        /// Key type.
        typedef typename pair::first_type Key;

        /// Element type.
        typedef typename pair::second_type T;

        /// Look for the nth element (0-based) with a certain key.
        const T* find(const Key& key, unsigned int n = 0) const;

        /// Test whether the nth element (0-based) with a certain key exists.
        bool has(const Key& key, unsigned int n = 0) const;
        
        /// \brief Access the nth element (0-based) with a certain key.
        ///
        /// The element is created if it does not exists.
        T& at(const Key& key, unsigned int n = 0);

        /// \brief Access the first element with a certain key.
        ///
        /// The element is created if it does not exists.
        T& operator[](const Key& key);

        using std::vector<pair>::at;
        using std::vector<pair>::operator[];

    private:
        typedef std::vector<pair> vector;
};

#include <MicroIni/Container.inl>

} // namespace MicroIni

#endif // MICROINI_CONTAINER_HPP
