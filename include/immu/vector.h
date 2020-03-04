#ifndef IMMUTABLESTRUCTURES_VECTOR_H
#define IMMUTABLESTRUCTURES_VECTOR_H

#include <utility>
#include "immu/details/rrbt.h"

namespace immu {
    template<typename T>
    class vector {
    public:
        using impl_t = immu::details::rrbt_t<T>;
        using size_t = typename impl_t::size_t;

        vector() : impl_(impl_t::empty()) {}

        vector(T value, size_t n) : impl_(impl_t::fill_with_value(n, std::move(value))) {}

        const T &operator[](size_t index) const &{
            return impl_->at(index);
        }

        vector push_back(T value) const &{
            return (vector) impl_->push_back_copy(value);
        }

        vector &&push_back(T value) &&{
            impl_->push_back_transient(value);
            return std::move(*this);
        }

    protected:
        impl_t *impl_;

        explicit vector(impl_t *impl) : impl_(impl) {};
    };
} //immu

#endif //IMMUTABLESTRUCTURES_VECTOR_H
