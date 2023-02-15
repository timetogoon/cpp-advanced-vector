#pragma once

#include <cassert>
#include <cstdlib>
#include <memory>
#include <utility>
#include <algorithm>
#include <iostream>

//--------------------Work with RawMemory-------------------------
template <typename T>
class RawMemory
{
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity);

    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory&) = delete;
    RawMemory(RawMemory&& other) noexcept;
    RawMemory& operator=(RawMemory&& rhs) noexcept;

    ~RawMemory();

    T* operator+(size_t offset) noexcept;

    const T* operator+(size_t offset) const noexcept;

    const T& operator[](size_t index) const noexcept;

    T& operator[](size_t index) noexcept;

    void Swap(RawMemory& other) noexcept;

    const T* GetAddress() const noexcept;

    T* GetAddress() noexcept;

    size_t Capacity() const;

private:
    void Init(RawMemory&& other);

    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n)
    {
        return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
    }

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept
    {
        operator delete(buf);
    }

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

//--------------------Work with RawMemory (Methods)-------------------------
template <typename T>
RawMemory<T>::RawMemory(size_t capacity)
    : buffer_(Allocate(capacity))
    , capacity_(capacity)
{
}

template <typename T>
RawMemory<T>::RawMemory(RawMemory&& other) noexcept
{
    Init(std::move(other));
}

template <typename T>
RawMemory<T>& RawMemory<T>::operator=(RawMemory&& rhs) noexcept
{
    Init(std::move(rhs));
    return *this;
}

template <typename T>
RawMemory<T>::~RawMemory()
{
    Deallocate(buffer_);
}

template <typename T>
T* RawMemory<T>::operator+(size_t offset) noexcept
{    
    return buffer_ + offset;
}

template <typename T>
const T& RawMemory<T>::operator[](size_t index) const noexcept
{
    return const_cast<RawMemory&>(*this)[index];
}

template <typename T>
T& RawMemory<T>::operator[](size_t index) noexcept
{    
    return buffer_[index];
}

template <typename T>
const T* RawMemory<T>::operator+(size_t offset) const noexcept
{
    return const_cast<RawMemory&>(*this) + offset;
}

template <typename T>
void RawMemory<T>::Init(RawMemory&& other)
{
    Deallocate(buffer_);
    buffer_ = std::exchange(other.buffer_, nullptr);
    capacity_ = std::exchange(other.capacity_, 0);
}

template <typename T>
void RawMemory<T>::Swap(RawMemory& other) noexcept
{
    std::swap(buffer_, other.buffer_);
    std::swap(capacity_, other.capacity_);
}

template <typename T>
const T* RawMemory<T>::GetAddress() const noexcept
{
    return buffer_;
}

template <typename T>
T* RawMemory<T>::GetAddress() noexcept
{
    return buffer_;
}

template <typename T>
size_t RawMemory<T>::Capacity() const
{
    return capacity_;
}

//--------------------Work with Vector-------------------------
template <typename T>
class Vector
{
public:
    Vector() = default;

    explicit Vector(size_t size);

    Vector(const Vector& other);

    Vector(Vector&& other) noexcept;

    Vector& operator=(const Vector& rhs);

    Vector& operator=(Vector&& rhs) noexcept;

    void Swap(Vector& other) noexcept;

    ~Vector();

    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
    
    size_t Size() const noexcept;

    size_t Capacity() const;

    const T& operator[](size_t index) const noexcept;

    T& operator[](size_t index) noexcept;
    
    void Resize(size_t new_size);
    void PushBack(const T& value);
    void PushBack(T&& value);
    void PopBack();

    template<typename... Args>
    T& EmplaceBack(Args&&... args);

    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args);

    iterator Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>);
    iterator Insert(const_iterator pos, const T& value);
    iterator Insert(const_iterator pos, T&& value);

    // Резервирования памяти под элементы вектора
    void Reserve(size_t new_capacity);

private:
    RawMemory<T> data_;
    size_t size_ = 0;

    // Вызывает деструкторы n объектов массива по адресу buf
    static void DestroyN(T* buf, size_t n) noexcept
    {
        for (size_t i = 0; i != n; ++i)
        {
            Destroy(buf + i);
        }
    }

    // Создаёт копию объекта elem в сырой памяти по адресу buf
    static void CopyConstruct(T* buf, const T& elem)
    {
        new (buf) T(elem);
    }

    // Вызывает деструктор объекта по адресу buf
    static void Destroy(T* buf) noexcept
    {
        buf->~T();
    }

    void SafeOp(T* from, size_t size, T* to);

    template <typename... Args>
    iterator EmplaceWithReallocate(const_iterator pos, Args&&...args);

    template <typename... Args>
    iterator EmplaceWithoutReallocate(const_iterator pos, Args&&...args);
};

//--------------------Work with Vector (Methods)-------------------------
//----------Constructors------------
template <typename T>
Vector<T>::Vector(size_t size)
    : data_(size)
    , size_(size)
{
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
}

template <typename T>
Vector<T>::Vector(const Vector& other)
    : data_(other.size_)
    , size_(other.size_)
{
    std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());
}

template <typename T>
Vector<T>::Vector(Vector&& other) noexcept
{
    data_ = std::move(other.data_);
    size_ = std::exchange(other.size_, 0);
}

//----------Destructor------------
template <typename T>
Vector<T>::~Vector()
{
    std::destroy_n(data_.GetAddress(), size_);
}

//--------------Begin/end iterators functions-------------------
template <typename T>
typename Vector<T>::iterator Vector<T>::begin() noexcept
{
    return data_.GetAddress();
}

template <typename T>
typename Vector<T>::iterator Vector<T>::end() noexcept
{
    return data_ + size_;
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::begin() const noexcept
{
    return data_.GetAddress();
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::end() const noexcept
{
    return data_ + size_;
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::cbegin() const noexcept
{
    return data_.GetAddress();
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::cend() const noexcept
{
    return data_ + size_;
}

//--------------------private field functions-----------------
template <typename T>
size_t Vector<T>::Size() const noexcept
{
    return size_;
}

template <typename T>
size_t Vector<T>::Capacity() const
{
    return data_.Capacity();
}

//-------------------Operators----------------
template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& rhs)
{
    if (this != &rhs) {
        if (rhs.size_ > data_.Capacity())
        {
            Vector rhs_copy(rhs);
            Swap(rhs_copy);
        }
        else
        {
            size_t copy_elem = rhs.size_ < size_ ? rhs.size_ : size_;
            auto end = std::copy_n(rhs.data_.GetAddress(), copy_elem, data_.GetAddress());
            if (rhs.size_ < size_)
            {
                std::destroy_n(end, size_ - rhs.size_);
            }
            else
            {
                std::uninitialized_copy_n(rhs.data_.GetAddress() + size_, rhs.size_ - size_, end);
            }
            size_ = rhs.size_;
        }
    }
    return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& rhs) noexcept
{
    Swap(rhs);
    return *this;
}

template <typename T>
const T& Vector<T>::operator[](size_t index) const noexcept
{
    return const_cast<Vector&>(*this)[index];
}

template <typename T>
T& Vector<T>::operator[](size_t index) noexcept
{
    return data_[index];
}

//----------------------Void functions--------------------
template <typename T>
void Vector<T>::Swap(Vector& other) noexcept
{
    data_.Swap(other.data_);
    std::swap(size_, other.size_);
}

template<typename T>
void Vector<T>::Resize(size_t new_size)
{
    if (new_size < size_)
    {
        std::destroy_n(data_.GetAddress(), size_ - new_size);

    }
    else
    {
        this->Reserve(new_size);
        std::uninitialized_value_construct_n(data_.GetAddress(), new_size - size_);
    }

    size_ = new_size;
}

template<typename T>
void Vector<T>::PushBack(const T& value)
{
    if (size_ == Capacity())
    {
        RawMemory<T>new_data(size_ == 0 ? 1 : size_ * 2);

        new (new_data + size_) T(value);

        SafeOp(data_.GetAddress(), size_, new_data.GetAddress());
        data_.Swap(new_data);
    }
    else
    {
        new (data_ + size_) T(value);
    }

    ++size_;
}

template<typename T>
void Vector<T>::PushBack(T&& value)
{
    EmplaceBack(std::move(value));
}

template<typename T>
void Vector<T>::SafeOp(T* from, size_t size, T* to) // Checks the type T for the possibility of constructing 
                                                    // a copy constructor or the presence of a nothrow move constructor
{
    if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
    {
        std::uninitialized_move_n(from, size, to);
    }
    else
    {
        std::uninitialized_copy_n(from, size, to);

    }
    std::destroy_n(from, size);
}

template<typename T>
void Vector<T>::PopBack()
{
    std::destroy_at(data_ + (size_ - 1));
    --size_;
}

template <typename T>
void Vector<T>::Reserve(size_t new_capacity)
{
    if (new_capacity <= data_.Capacity())
    {
        return;
    }

    RawMemory<T>new_data(new_capacity);

    SafeOp(data_.GetAddress(), size_, new_data.GetAddress());

    data_.Swap(new_data);
}

//-------------Functions that returns iterators-------------------
template<typename T>
template<typename... Args>
T& Vector<T>::EmplaceBack(Args&&... args)
{
    if (size_ == Capacity())
    {
        RawMemory<T>new_data(size_ == 0 ? 1 : size_ * 2);

        new (new_data + size_) T(std::forward<Args>(args)...);

        SafeOp(data_.GetAddress(), size_, new_data.GetAddress());

        data_.Swap(new_data);
    }
    else
    {
        new (data_ + size_) T(std::forward<Args>(args)...);
    }

    ++size_;

    return data_[size_ - 1];
}

template<typename T>
template<typename ...Args>
typename Vector<T>::iterator Vector<T>::Emplace(const_iterator pos, Args&& ...args)
{
    assert(pos >= begin() && pos <= end());

    if (pos == end()) {
        return &EmplaceBack(std::forward<Args>(args)...);
    }

    if (size_ == Capacity())
    {
        return EmplaceWithReallocate(pos, std::forward<Args>(args)...);
    }
    else
    {
        return EmplaceWithoutReallocate(pos, std::forward<Args>(args)...);
    }

    return iterator();
}

template<typename T>
typename Vector<T>::iterator Vector<T>::Erase(const_iterator pos) noexcept(
                                        std::is_nothrow_move_assignable_v<T>)
{
    assert(pos >= begin() && pos < end());
    assert(size_ > 0);
    auto distance = pos - cbegin();
    iterator pos_ = data_.GetAddress() + distance;
    std::move(pos_ + 1, end(), pos_);
    data_[size_ - 1].~T();
    --size_;
    return pos_;
}

template<typename T>
typename Vector<T>::iterator Vector<T>::Insert(const_iterator pos, const T& value) 
{
    return Emplace(pos, value);
}

template<typename T>
typename Vector<T>::iterator Vector<T>::Insert(const_iterator pos, T&& value) 
{
   return Emplace(pos, std::move(value));
}

template<typename T>
template<typename ...Args>
typename Vector<T>::iterator Vector<T>::EmplaceWithReallocate(const_iterator pos, Args&&...args)
{
    RawMemory<T>new_data(size_ == 0 ? 1 : size_ * 2);

    auto index = std::distance(cbegin(), pos);

    new (new_data + index) T(std::forward<Args>(args)...);

    try
    {
        SafeOp(data_.GetAddress(), index, new_data.GetAddress());
    }
    catch (...)
    {
        std::destroy_at(new_data + index);
        throw;
    }

    try
    {
        SafeOp(data_.GetAddress() + index, size_ - index, new_data.GetAddress() + (index + 1));
    }
    catch (...)
    {
        std::destroy_n(new_data.GetAddress(), index + 1);
        throw;
    }

    data_.Swap(new_data);
    ++size_;
    return begin() + index;
}

template<typename T>
template<typename ...Args>
typename Vector<T>::iterator Vector<T>::EmplaceWithoutReallocate(const_iterator pos, Args&&...args)
{
    auto index = std::distance(cbegin(), pos);

    T tmp(std::forward<Args>(args)...);

    new (end()) T(std::move(data_[size_ - 1]));

    std::move_backward(begin() + index, end() - 1, end());
    data_[index] = std::move(tmp);
    ++size_;
    return begin() + index;
}