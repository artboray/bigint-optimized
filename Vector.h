#ifndef BIGINT_VECTOR_H
#define BIGINT_VECTOR_H

#include <memory>
#include <cstring>

class Vector
{
    public:
        Vector();
        ~Vector();
        Vector(const Vector& other);
        Vector& operator=(Vector other);

        void pop_back();
        void set_size();
        void resize(size_t sz);
        void clear();
        void push_back(uint32_t);
        size_t size() const;
        uint32_t* get_id() const;

        uint32_t& operator[](size_t);
        const uint32_t& operator[](size_t) const;

    private:
        static const size_t START_SIZE = 6;

        size_t _size;
        bool is_big;

        void change();

        struct big_data
        {
            size_t _cap;
            std::shared_ptr<uint32_t> ptr;
            big_data(size_t cap, std::shared_ptr<uint32_t> ptr) : _cap(cap), ptr(std::move(ptr)) {}
        };

        union data
        {
            uint32_t small[START_SIZE];
            big_data big;
            data() {}
            ~data() {}
        } _data;
};


#endif