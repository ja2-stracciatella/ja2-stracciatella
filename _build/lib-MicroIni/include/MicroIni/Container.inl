template <typename pair>
typename Container<pair>::const_iterator Container<pair>::find(const Key& key, unsigned int n) const
{
    for(typename Container<pair>::const_iterator i = Container<pair>::begin(); i != Container<pair>::end(); ++i)
        if(i->first == key && !n--)
            return i;
    return Container<pair>::end();
}

template <typename pair>
typename Container<pair>::iterator Container<pair>::find(const Key& key, unsigned int n)
{
    for(typename Container<pair>::iterator i = Container<pair>::begin(); i != Container<pair>::end(); ++i)
        if(i->first == key && !n--)
            return i;
    return Container<pair>::end();
}

template <typename pair>
void Container<pair>::erase(const Key& key)
{
    typename Container<pair>::iterator i = Container<pair>::begin();
    while(i != Container<pair>::end())
        if(i->first == key)
            i = erase(i);
        else
            ++i;
}

template <typename pair>
bool Container<pair>::has(const Key& key, unsigned int n) const
{
    return find(key, n) != Container<pair>::end();
}

template <typename pair>
typename Container<pair>::T& Container<pair>::at(const Key& key, unsigned int n)
{
    typename Container<pair>::iterator result = find(key, n);
    if(result != Container<pair>::end())
        return result->second;
    else
    {
        Container<pair>::push_back(pair(key, T()));
        return Container<pair>::back().second;
    }
}

template <typename pair>
typename Container<pair>::T& Container<pair>::operator[](const Key& key)
{
    return at(key);
}

template <typename pair>
Container<pair>& Container<pair>::operator+=(const pair& element)
{
    Container<pair>::push_back(element);
    return *this;
}
