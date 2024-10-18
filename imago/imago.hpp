#pragma once
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

template <typename T> class Image
{
  public:
    Image(size_t width, size_t height) : m_width(width), m_height(height)
    {
        m_pixels = std::make_unique<T[]>(width * height);
    }
    Image(size_t width, size_t height, T fill_value) : m_width(width), m_height(height)
    {
        size_t element_count = width * height;
        m_pixels = std::make_unique<T[]>(element_count);
        std::fill(m_pixels.get(), m_pixels.get() + element_count, fill_value);
    }
    Image(size_t width, size_t height, std::unique_ptr<T[]> &&pixels)
        : m_width(width), m_height(height), m_pixels(std::move(pixels))
    {
    }

    class Iterator
    {
      public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;
        using reference = T &;

        Iterator(pointer ptr) : m_ptr(ptr)
        {
        }

        reference operator*() const
        {
            return *m_ptr;
        }

        pointer operator->() const
        {
            return m_ptr;
        }

        Iterator &operator++()
        {
            ++m_ptr;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Iterator &operator--()
        {
            --m_ptr;
            return *this;
        }

        Iterator operator--(int)
        {
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        Iterator operator+(difference_type n) const
        {
            return iterator(m_ptr + n);
        }

        Iterator operator-(difference_type n) const
        {
            return iterator(m_ptr - n);
        }

        Iterator &operator+=(difference_type n)
        {
            m_ptr += n;
            return *this;
        }

        Iterator &operator-=(difference_type n)
        {
            m_ptr -= n;
            return *this;
        }

        auto operator<=>(const Iterator &) const = default;

        difference_type operator-(const Iterator &other) const
        {
            return m_ptr - other.m_ptr;
        }

        reference operator[](difference_type n) const
        {
            return *(m_ptr + n);
        }

      private:
        pointer m_ptr;
    };

    const Iterator cbegin() const
    {
        return Iterator(m_pixels.get());
    }

    Iterator begin()
    {
        return Iterator(m_pixels.get());
    }

    Iterator end()
    {
        return Iterator(m_pixels.get() + size());
    }

    Iterator cend() const
    {
        return Iterator(m_pixels.get() + size());
    }

    T *data() const
    {
        return m_pixels.get();
    }

    T &at(size_t x, size_t y) const
    {
        return m_pixels[y * m_width + x];
    }

    T &at(size_t x, size_t y)
    {
        return m_pixels[y * m_width + x];
    }

    size_t width() const
    {
        return m_width;
    }

    size_t height() const
    {
        return m_height;
    }

    size_t size() const
    {
        return m_width * m_height;
    }

  private:
    size_t m_width;
    size_t m_height;
    std::unique_ptr<T[]> m_pixels;
};

struct RGBA32
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

template <typename T, typename UnaryFunction> auto convert_image(const Image<T> &image, const UnaryFunction &function)
{
    using output_type = decltype(function(T{}));
    Image<output_type> output(image.width(), image.height());
    std::transform(image.cbegin(), image.cend(), output.begin(), function);
    return output;
}

inline uint8_t rgba_to_gray(const RGBA32 rgba)
{
    const uint32_t red = static_cast<uint32_t>(rgba.r);
    const uint32_t green = static_cast<uint32_t>(rgba.g);
    const uint32_t blue = static_cast<uint32_t>(rgba.b);
    return (299u * red + 587u * green + 114u * blue) / 1000u;
}

inline Image<RGBA32> load_image(const fs::path &path);
inline bool save_image(const Image<RGBA32> &image, const fs::path &path);

#ifdef IMAGO_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include "stb_image.h"
#include "stb_image_write.h"
#pragma GCC diagnostic pop

inline Image<RGBA32> load_image(const fs::path &path)
{
    int width, height, components;
    uint8_t *bytes = stbi_load(path.c_str(), &width, &height, &components, STBI_rgb_alpha);
    assert(bytes != NULL);
    size_t size = width * height;
    std::unique_ptr<RGBA32[]> pixels = std::make_unique<RGBA32[]>(size);
    memcpy(pixels.get(), bytes, size * sizeof(RGBA32));
    Image<RGBA32> image(width, height, std::move(pixels));
    free(bytes);
    return image;
}

inline bool save_image(const Image<RGBA32> &image, const fs::path &path)
{
    const size_t component_count = 4;
    const size_t stride_in_bytes = image.width() * component_count;

    int result =
        stbi_write_png(path.c_str(), image.width(), image.height(), component_count, image.data(), stride_in_bytes);
    return result != 0;
}

#endif
