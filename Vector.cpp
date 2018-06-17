#include "Vector.h"

#include <iostream>

Vector::Vector() : _size(0), is_big(false) {}

Vector::Vector(const Vector& other) : _size(other._size), is_big(other.is_big)
{
    if (is_big)
        new (&_data.big) big_data(other._data.big);
    else
        for (size_t i = 0; i < _size; ++i)
            _data.small[i] = other._data.small[i];
}

void Vector::swap(Vector& other) noexcept
{
    std::swap(_size, other._size);
    std::swap(is_big, other.is_big);

    char tmp[sizeof(data)];
    memcpy(tmp, &_data, sizeof(data));
    memcpy(&_data, &other._data, sizeof(data));
    memcpy(&other._data, tmp, sizeof(data));
}

Vector& Vector::operator=(Vector other)
{
    swap(other);
    return *this;
}

void Vector::set_size()
{
    if (!is_big && _size > START_SIZE)
    {
        size_t new_cap = START_SIZE;
        while (new_cap < _size)
            new_cap *= 2;
        uint32_t* tmp = new uint32_t[new_cap];
        memcpy(tmp, _data.small, START_SIZE * sizeof(uint32_t));
        new (&_data.big) big_data(new_cap, std::shared_ptr<uint32_t>(tmp, std::default_delete<uint32_t>()));
        is_big = true;
    }
    else if (is_big && _size > _data.big._cap)
    {
        size_t old_cap = _data.big._cap;
        size_t new_cap = _data.big._cap;
        while (new_cap < _size)
            new_cap *= 2;
        uint32_t* tmp = new uint32_t[new_cap];
        memcpy(tmp, _data.big.ptr.get(), old_cap * sizeof(uint32_t));
        _data.big.ptr = std::shared_ptr<uint32_t>(tmp, std::default_delete<uint32_t>());
        _data.big._cap = new_cap;
    }
}

void Vector::resize(size_t size)
{
    change();
    size_t old_size = _size;
    _size = size;
    set_size();
    for (size_t i = old_size; i < size; i++)
        (*this)[i] = 0;
}

void Vector::push_back(uint32_t val)
{
    change();
    _size++;
    set_size();
    (*this)[_size - 1] = val;
}


void Vector::pop_back() {
    change();
    _size--;
}

void Vector::clear() {
    change();
    _size = 0;
}

uint32_t* Vector::get_id() const
{
    if (is_big)
        return _data.big.ptr.get();
    else
        return (uint32_t*)(_data.small);
}

uint32_t& Vector::operator[](size_t id)
{
    change();
    return *(get_id() + id);
}

const uint32_t &Vector::operator[](size_t id) const
{
    return *(get_id() + id);
}

size_t Vector::size() const
{
    return _size;
}

void Vector::change()
{
    if (!is_big || _data.big.ptr.unique())
        return;
    uint32_t* tmp = new uint32_t[_data.big._cap];
    memcpy(tmp, _data.big.ptr.get(), _data.big._cap * sizeof(uint32_t));
    _data.big.ptr = std::shared_ptr<uint32_t>(tmp, std::default_delete<uint32_t>());
}
