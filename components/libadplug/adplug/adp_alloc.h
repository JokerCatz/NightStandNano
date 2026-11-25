/* libadplug patch 0009: the STL containers of the players use their own allocator type (so their template instantiations are never merged with
 * another translation unit's std::vector<T> that allocates with the toolchain's operator new, which would free tracked memory the wrong way).
 * It allocates through adplug_c.cpp's per-handle list (caps, longjmp on failure, freed when a hostile file is abandoned). */
#ifndef ADP_ALLOC_H
#define ADP_ALLOC_H
#include <stddef.h>
#include <deque>
#include <stack>
#include <vector>
extern "C" void *adp_raw_alloc(size_t n);
extern "C" void adp_raw_free(void *p);
template <class T> struct adp_std_alloc {
    typedef T value_type;
    adp_std_alloc() noexcept {}
    template <class U> adp_std_alloc(const adp_std_alloc<U> &) noexcept {}
    T *allocate(size_t n) { return (T *)adp_raw_alloc(n > (size_t)0x7FFFFFFF / sizeof(T) ? (size_t)-1 : n * sizeof(T)); }
    void deallocate(T *p, size_t) noexcept { adp_raw_free(p); }
};
template <class T, class U> bool operator==(const adp_std_alloc<T> &, const adp_std_alloc<U> &) { return true; }
template <class T, class U> bool operator!=(const adp_std_alloc<T> &, const adp_std_alloc<U> &) { return false; }
#define ADP_VECTOR(T) std::vector<T, adp_std_alloc<T> >
#define ADP_STACK(T) std::stack<T, std::deque<T, adp_std_alloc<T> > >
#endif
