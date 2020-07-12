#pragma once
#include <vector>
#include <dots/type/Typeless.h>

namespace dots::type
{
    template <typename T = Typeless, typename = void>
    struct Vector;

    template <>
    struct Vector<Typeless>
    {        
        virtual ~Vector() = default;

        virtual Vector& operator = (const Vector& /*rhs*/)
        {
            // note that this is only a necessary dummy implementation that always gets overriden by sub-class
            return *this;
        }
        
        virtual Vector& operator = (Vector&& /*rhs*/) noexcept
        {
            // note that this is only a necessary dummy implementation that always gets overriden by sub-class
            return *this;
        }

        virtual size_t typelessSize() const noexcept = 0;
        virtual Typeless& typelessAt(size_t pos) = 0;
        virtual const Typeless& typelessAt(size_t pos) const = 0;
        virtual const Typeless* typelessData() const = 0;
        virtual Typeless* typelessData() = 0;
        virtual void typelessPushBack(const Typeless& value) = 0;
        virtual void typelessPushBack(Typeless&& value) = 0;

    protected:

        Vector() = default;
        Vector(const Vector& other) = default;
        Vector(Vector&& other) = default;
    };

    template <typename T>
    struct Vector<T> : Vector<Typeless, void>, std::vector<std::conditional_t<std::is_same_v<T, bool>, uint8_t, T>>
    {
        using vector_t = std::vector<std::conditional_t<std::is_same_v<T, bool>, uint8_t, T>>;
        
        Vector() = default;
        
        Vector(const Vector& other) = default;
        Vector(Vector&& other) = default;

        using std::vector<std::conditional_t<std::is_same_v<T, bool>, uint8_t, T>>::vector;

        Vector(const vector_t& other) noexcept :
            vector_t(other)
        {
            /* do nothing */
        }

        Vector(vector_t&& other) noexcept :
            vector_t(std::move(other))
        {
            /* do nothing */
        }
        
        // ReSharper disable CppHidingFunction
        ~Vector() = default; // note: hiding a non-virtual destructor is harmless for stateless sub-classes
        // ReSharper restore CppHidingFunction

        Vector& operator = (const Vector& rhs) = default;
        Vector& operator = (Vector&& rhs) = default;

        using vector_t::operator=;

        Vector& operator = (const vector_t& rhs) noexcept
        {
            return vector_t::operator=(rhs);
        }
        
        Vector& operator = (vector_t&& rhs) noexcept
        {
            return vector_t::operator=(std::move(rhs));
        }

        Vector& operator = (const Vector<Typeless>& rhs) override
        {
            // TODO: ensure compatible type
            *this = static_cast<const Vector&>(rhs);
            return *this;
        }
        
        Vector& operator = (Vector<Typeless>&& rhs) noexcept override
        {
            // TODO: ensure compatible type
            *this = static_cast<Vector&&>(rhs);
            return *this;
        }

        size_t typelessSize() const noexcept override
        {
            return vector_t::size();
        }

        const Typeless& typelessAt(size_t pos) const override
        {
            return Typeless::From(vector_t::operator[](pos));
        }
        
        Typeless& typelessAt(size_t pos) override
        {
            return Typeless::From(vector_t::operator[](pos));
        }        

        Typeless* typelessData() override
        {
            return Typeless::From(vector_t::data());
        }
        
        const Typeless* typelessData() const override
        {
            return Typeless::From(vector_t::data());
        }

        void typelessPushBack(const Typeless& value) override
        {
            vector_t::push_back(value.to<T>());
        }
        
        void typelessPushBack(Typeless&& value) override
        {
            vector_t::push_back(std::move(value).to<T>());
        }
    };
}