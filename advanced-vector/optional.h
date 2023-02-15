#include <stdexcept>
#include <utility>
#include <algorithm>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception
{
public:
    using exception::exception;

    virtual const char* what() const noexcept override
    {
        return "Bad optional access";
    }
};

template <typename T>
class Optional
{
public:
    Optional() = default;
    Optional(const T& value);
    Optional(T&& value);
    Optional(const Optional& other);
    Optional(Optional&& other);

    Optional& operator=(const T& value);
    Optional& operator=(T&& rhs);
    Optional& operator=(const Optional& rhs);
    Optional& operator=(Optional&& rhs);

    ~Optional();

    bool HasValue() const
    {
        return is_initialized_;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*()&;
    const T& operator*() const&;
    T* operator->();
    const T* operator->() const;
    T&& operator*()&&;

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value()&;
    const T& Value() const&;    
    T&& Value() &&;

    void Reset();

    template <typename...Args>
    void Emplace(Args&&...args);

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
    T* ptr_value_ = nullptr;
};

template <typename T>
Optional<T>::~Optional()
{
    if (this->HasValue())
    {
        ptr_value_->~T();
    }
}

template <typename T>
Optional<T>::Optional(const T& value)
{
    ptr_value_ = new(&data_) T(value);
    is_initialized_ = true;
}

template <typename T>
Optional<T>::Optional(T&& value)
{
    ptr_value_ = new (&data_) T(std::move(value));
    is_initialized_ = true;
}

template <typename T>
Optional<T>::Optional(const Optional& other)
{

    if (HasValue() && !other.HasValue())
    {
        Reset();
    }
    else if (!this->HasValue() && other.HasValue())
    {
        ptr_value_ = new(&data_[0]) T(other.Value());
        is_initialized_ = true;
    }
    else if (HasValue() && other.HasValue())
    {
        *ptr_value_ = other.Value();
        is_initialized_ = true;
    }
}

template<typename T>
Optional<T>::Optional(Optional&& other)
{
    if (HasValue() && !other.HasValue())
    {
        Reset();
    }
    else if (!this->HasValue() && other.HasValue())
    {
        ptr_value_ = new (&data_) T(std::move(other.Value()));
        is_initialized_ = true;
    }
    else if (HasValue() && other.HasValue())
    {
        *ptr_value_ = std::move(other.Value());
        is_initialized_ = true;
    }
}

template<typename T>
Optional<T>& Optional<T>::operator=(const T& value)
{
    if (this->HasValue())
    {
        *ptr_value_ = value;
    }
    else
    {
        ptr_value_ = new (&data_) T(value);
        is_initialized_ = true;
    }
    return *this;
}

template<typename T>
Optional<T>& Optional<T>::operator=(T&& value)
{
    if (this->HasValue())
    {
        *ptr_value_ = std::move(value);
    }
    else
    {
        ptr_value_ = new (&data_) T(std::move(value));
        is_initialized_ = true;
    }
    return *this;
}

template<typename T>
Optional<T>& Optional<T>::operator=(const Optional& value)
{
    if (this->HasValue() && value.HasValue())
    {
        *this = std::move(value.Value());
        is_initialized_ = true;
    }
    else if (!this->HasValue() && value.HasValue())
    {
        ptr_value_ = new (&data_) T(std::move(value.Value()));
        is_initialized_ = true;
    }
    else if (this->HasValue() && !value.HasValue())
    {
        this->Reset();
        is_initialized_ = value.is_initialized_;
    }
    return *this;
}

template<typename T>
Optional<T>& Optional<T>::operator=(Optional&& value)
{
    if (this->HasValue() && value.HasValue())
    {
        *this = std::move(value.Value());
        is_initialized_ = std::move(value.is_initialized_);
    }
    else if (!this->HasValue() && value.HasValue())
    {
        ptr_value_ = new (&data_) T(std::move(value.Value()));
        is_initialized_ = std::move(value.is_initialized_);;
    }
    else if (this->HasValue() && !value.HasValue())
    {
        this->Reset();
    }
    return *this;
}

template<typename T>
T& Optional<T>::operator*()&
{
    return *ptr_value_;
}

template<typename T>
const T& Optional<T>::operator*() const&
{
    return *ptr_value_;
}

template <typename T>
T&& Optional<T>::operator*()&&
{
    return std::move(*ptr_value_);
}

template<typename T>
T* Optional<T>::operator->()
{
    return &*ptr_value_;
}

template<typename T>
const T* Optional<T>::operator->() const
{
    return &*ptr_value_;
}

template <typename T>
T& Optional<T>::Value()&
{
    if (this->HasValue())
    {
        return *ptr_value_;
    }
    throw BadOptionalAccess{};
}

template <typename T>
T&& Optional<T>::Value()&&
{
    return std::move(*ptr_value_);
}

template <typename T>
const T& Optional<T>::Value() const&
{
    if (this->HasValue())
    {
        return *ptr_value_;
    }
    throw BadOptionalAccess{};
}

template<typename T>
void Optional<T>::Reset()
{
    ptr_value_->~T();
    is_initialized_ = false;
}

template<typename T>
template<typename...Args>
void Optional<T>::Emplace(Args&&...args)
{
    ptr_value_ = new (&data_) T(std::forward<Args>(args)...);
    is_initialized_ = true;
}
