#ifndef _RT_MACROS_H_
#define _RT_MACROS_H_
#include <iterator>
#ifndef ASSERT
#include <cassert>
#define ASSERT assert
#endif

// ���������� ����� �� ����� Chromium (��. http://habrahabr.ru/blogs/google_chrome/119815/ )
template <typename T, size_t N> char(&ArraySizeHelper(T(&array)[N]))[N];
#define size_array(array) (sizeof(ArraySizeHelper(array)))

#define zero_array(x)   memset( x,0,sizeof(x[0])*size_array(x) ) //-V531, ������ ��� PVS Studio

#define ZDA_DEPRECATE(_Text) __declspec(deprecated(_Text))
#define ZDA_DANGEROUS(_Replacement) ZDA_DEPRECATE("Usage this function may be dangerous. Consider using " #_Replacement " instead.")

#if _MSC_VER<1600
#define nullptr 0 /* C++0x */
#endif

//
// ���������� size_t � unsigned � ���������
// ������ ��� x64, ������������� ��� x32
//
// ���������� size() ��� ������������� ����������
template <typename T>
unsigned size32(const T& t)
{
    typename T::size_type sz = t.size();
    unsigned sz32 = static_cast<unsigned>(sz);
    ASSERT(sz32 == sz);
    return sz32;
}
// ���������� distance() ��� ���� ����������
template <typename T>
int distance32(T it1, T it2)
{
    auto dist = std::distance(it1, it2);
    int dist32 = static_cast<int>(dist);
    ASSERT(dist32 == dist);
    return dist32;
}

// ���������� size_t
inline unsigned to32(size_t sz)
{
    unsigned sz32 = static_cast<unsigned>(sz);
    ASSERT(sz32 == sz); //-V104
    return sz32;
}

#endif