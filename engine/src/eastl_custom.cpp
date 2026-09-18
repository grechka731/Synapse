#include <cstddef>
#include <cstdlib>
#include <new>

void* operator new[](std::size_t size, const char* /*pName*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/) {
    return std::malloc(size);
}

void* operator new[](std::size_t size, std::size_t alignment, std::size_t alignmentOffset, const char* /*pName*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/) {
#if defined(_MSC_VER)
    return _aligned_malloc(size, alignment);
#else
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return nullptr;
    }
    return ptr;
#endif
}