#ifndef IMMU_EXCEPTIONS_H
#define IMMU_EXCEPTIONS_H


#include <exception>
#include <sstream>

namespace immu::details {

    class random_access_out_of_range : public std::exception {
    public:
        random_access_out_of_range(size_t index, size_t size)
                : index_(index), size_(size) {}

        [[nodiscard]] const char *what() const noexcept override {
            std::stringstream ss;
            ss << "Out of range exception was called because ";
            ss << "(index = " << index_ << ") >= (size = " << size_ << ")" << std::endl;
            return ss.str().c_str();
        }

    private:
        size_t index_, size_;
    };
}

#endif //IMMU_EXCEPTIONS_H
