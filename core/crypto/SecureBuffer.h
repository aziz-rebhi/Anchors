#ifndef SECUREBUFFER_H
#define SECUREBUFFER_H

#pragma once
#include <sodium.h>
#include <cstddef>
#include <utility>

class SecureBuffer{
public:
    explicit SecureBuffer(size_t size)
        : m_size(size)
        , m_data(size > 0 ? static_cast<unsigned char *>(sodium_malloc(size)) : nullptr)
    {
    }


    ~SecureBuffer(){
        if (m_data){
            sodium_free(m_data);
        }
    }

    SecureBuffer(const SecureBuffer &) = delete ;
    SecureBuffer &operator = (const SecureBuffer &) = delete;

    SecureBuffer(SecureBuffer &&other) noexcept : m_size(other.m_size), m_data(other.m_data){
        other.m_size = 0;
        other.m_data = nullptr ;
    }

    SecureBuffer &operator=(SecureBuffer &&other) noexcept {
        if (this != &other){
            if (m_data){
                sodium_free(m_data);
            }
            m_data = other.m_data;
            m_size = other.m_size;
            other.m_data = nullptr;
            other.m_size = 0;
        }
        return *this;
    }

    unsigned char *data() {return m_data;}
    const unsigned char *data() const {return m_data;}
    size_t size() const {return m_size;}
    bool isValid() const {return m_data != nullptr;}

private:
    size_t m_size;
    unsigned char *m_data;

};

#endif // SECUREBUFFER_H
