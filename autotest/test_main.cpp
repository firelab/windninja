#if !defined( WIN32 )
    #define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#ifdef _OPENMP

#ifndef NETCDF_LOCK_SET
#define NETCDF_LOCK_SET

#include "omp_guard.h"
omp_lock_t netCDF_lock;

#endif  //NETCDF_LOCK_SET

#endif //_OPENMP
