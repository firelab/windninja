#ifndef CPLISNAN_H
#define CPLISNAN_H

#include <cmath>

template <class T>
inline bool cplIsNan(T t)
{
#ifdef WIN32
  return CPLIsNan((double)t); // we need to disambiguate
#else
  return std::isnan(static_cast<double>(t)); // we need to disambiguate
#endif
}
#endif