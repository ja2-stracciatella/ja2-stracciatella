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

        /// \brief Look for the nth pair (0-based) with a certain key.
        /// \return An iterator to the pair, or end() if it was not found.
        typename Container<pair>::const_iterator find(const Key& key, unsigned int n = 0) const;

        /// \brief Look for the nth pair (0-based) with a certain key.
        /// \return An iterator to the pair, or end() if it was not found.
        typename Container<pair>::iterator find(const Key& key, unsigned int n = 0);

        /// Remove pairs with a certain name.
        void erase(const Key& key);

        /// Test whether the nth element (0-based) with a certain key exists.
        bool has(const Key& key, unsigned int n = 0) const;

        /// \brief Access the nth element (0-based) with a certain key.
        ///
        /// The element is created if it does not exist.
        T& at(const Key& key, unsigned int n = 0);

        /// \brief Access the first element with a certain key.
        ///
        /// \copydetails at
        T& operator[](const Key& key);

        /// \brief Add a new element at the end of the Container.
        ///
        /// Equivalent to `push_back`.
        Container<pair>& operator+=(const pair& element);

        using std::vector<pair>::erase;
        using std::vector<pair>::at;
        using std::vector<pair>::operator[];
};

#include <MicroIni/Container.inl>

} // namespace MicroIni

#endif // MICROINI_CONTAINER_HPP
