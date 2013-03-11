template <typename pair>
const typename Container<pair>::T* Container<pair>::find(const Key& key, unsigned int n) const
{
    for(typename vector::const_iterator i = vector::begin(); i != vector::end(); ++i)
        if(i->first == key && !n--)
            return &i->second;
    return 0;
}

template <typename pair>
bool Container<pair>::has(const Key& key, unsigned int n) const
{
    return find(key, n);
}

template <typename pair>
typename Container<pair>::T& Container<pair>::at(const Key& key, unsigned int n)
{
    if(T* result = const_cast<T*>(find(key, n)))
        return *result;
    else
    {
        vector::push_back(pair(key, T()));
        return vector::back().second;
    }
}

template <typename pair>
typename Container<pair>::T& Container<pair>::operator[](const Key& key)
{
    return at(key);
}
