template <typename T>
Value::Value(const T& value)
{
    std::ostringstream oss;
    oss << value;
    *this = oss.str();
}

template <typename T>
Value::operator T() const
{
    std::istringstream iss(*this);
    T t;
    iss >> t;
    return t;
}
