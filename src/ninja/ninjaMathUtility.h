/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  utility functions for angles
 * Author:   Jason Forthofer <jforthofer@gmail.com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER 
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef NINJA_MATH_UTILITY_H
#define NINJA_MATH_UTILITY_H

#include <math.h>
#include <cmath>
#include <cassert>
#include <limits>
#include "constants.h"
#include "ninjaException.h"

//Below from last functions
#include <cstddef>
#if defined _MSC_VER  // may need to check your VC++ version
  typedef unsigned __int32 uint32_t;
  typedef unsigned __int64 uint64_t;
#else
  #include <stdint.h>
#endif
#include <limits>


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function returns the angle in a standard math xy sense, given the
geographic angle from North. The incoming angle must be between 0 and
360. The returned angle is also between 0 and 360.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

template<class T>
T n_to_xy(T north_angle)
{
  T xy_angle;

  if((north_angle < 0.0) || (north_angle > 360.0))
	  return -1;
  xy_angle = 450.0 - north_angle;
  if(xy_angle > 360.0)
	  xy_angle = xy_angle - 360;

  return xy_angle;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function returns the geographic angle from North, given the standard
math xy sense angle. The incoming angle must be between 0 and 360.
The returned angle is also between 0 and 360.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

template <class T>
T xy_to_n(T xy_angle)
{
  T north_angle;

  if((xy_angle < 0.0) || (xy_angle > 360.0))
	  return -1;
  north_angle = 450.0 - xy_angle;
  if(north_angle > 360.0)
	  north_angle = north_angle - 360;

  return north_angle;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Functions converts 2D vector magnitude/direction to uv and vice-versa
direction is angle from north.
NOTE:  Use wind_sd_to_uv() to convert wind (since wind direction
	 is direction FROM rather than TO
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

template<class T>
void sd_to_uv(T speed, T direction, T *u, T *v)
{

  *u = speed * cos(n_to_xy(direction) * pi / 180);
  *v = speed * sin(n_to_xy(direction) * pi / 180);
}

template<class T>
void uv_to_sd(T u, T v, T *s, T *d)
{
  double inter_dir;
  *s = std::sqrt((u * u) + (v * v));
  inter_dir = (atan2(v,u) * 180 / pi);
  if(inter_dir < 0)
	  inter_dir += 360.0;
  *d = xy_to_n(inter_dir);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Functions converts speed/direction to uvw and vice-versa
r is vector magnitude (should be > 0)
theta is angle from North in a clockwise fashion (range is 0 to 360)
phi is angle up (or down) from horizontal (horizontal is 0) (range -90 to +90)
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
template<class T>
void uvw_to_rThetaPhi(T u, T v, T w, T *r, T *theta, T *phi)
{
  double inter_dir;

  *r = std::sqrt((u * u) + (v * v) + (w * w));
  if(*r == 0.0)
  {
	  *theta = 90.0;
	  *phi = 0.0;
	  return;
  }

  if(u == 0.0)   // handle u=0 so don't divide by zero later...
  {
	  *theta = 90.0;
  }else
  {
	  inter_dir = (atan2(v,u) * 180 / pi);
	  if(inter_dir < 0)
		  inter_dir += 360.0;
	  *theta = xy_to_n(inter_dir);
  }


  *phi = acos(w / *r) * 180 / pi;
  *phi -= 90;
  *phi = -*phi;
}

template<class T>
void rThetaPhi_to_uvw(T r, T theta, T phi, T *u, T *v, T *w)
{
  theta = n_to_xy(theta);

  *u = r * cos(theta * pi / 180) * sin((90 - phi) * pi / 180);
  *v = r * sin(theta * pi / 180) * sin((90 - phi) * pi / 180);
  *w = r * cos((90 - phi) * pi / 180);
}

template<class T>
void wind_sd_to_uv(T speed, T direction, T *u, T *v)
{	//Function converts from wind speed (measured degrees from North, direction wind comes from,
  //to u,v components
  if(direction>360.0)
  {
	  direction = direction - 360.0;	//Try this incase it's just a little over 360 (ie 360.0000003)
	  if(direction>360.0)
		  throw std::runtime_error("Direction greater than 360 degrees in wind_sd_to_uv().");
  }
  if(direction<0.0)
	  throw std::runtime_error("Direction less than zero degrees in wind_sd_to_uv().");

  if(direction==360.0) direction=0.0;    //so I don't have to worry about 360

  //Set u
  if((direction==0.0)||(direction==180.0))
  {
	  *u=0.0;
  }else{
	  *u=-speed*sin(direction*pi/180.0);
  }

  //Set v
  if((direction==90.0)||(direction==270.0))
  {
	  *v=0.0;
  }else{
	  *v=-speed*cos(direction*pi/180.0);
  }
}

template<class T>
void wind_uv_to_sd(T u, T v, T *s, T *d)
{//Function converts from u,v wind components to wind speed (measured degrees from North, direction wind comes from)
  double inter_dir;
  *s = std::sqrt((u * u) + (v * v));
  inter_dir = (atan2(v,u) * 180 / pi);
  inter_dir = inter_dir - 180.0;
  if(inter_dir < 0)
	  inter_dir += 360.0;
  *d = xy_to_n(inter_dir);
}
/**
 * Function to test if 2 doubles are nearly equal.
 * The function AlmostEqual2sComplement() may be better, but this is simple.
 * @param a First value to test.
 * @param b Second value to test.
 * @param epsilonMultiplier A value that is multiplied by the machine precision epsilon to test "closeness".
 * @return True if equal, false if not equal.
 */
bool areEqual(double a, double b, double epsilonMultiplier = 100.0);

int computeMode(std::vector<int> &numbers);


//Jason took below from http://www.working-software.com/node/35

// -*- c++ -*-
//
// Copyright 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Authors: Zhanyong Wan, Sean Mcafee
//
// Taken from The Google C++ Testing Framework (Google Test).
// Modified for this discussion by Fred Richards.
//

namespace
{
    template <size_t bytes>
    struct TypeWithSize
    {
        typedef void UInt;
    };

    template <>
    struct TypeWithSize<4>
    {
        typedef uint32_t UInt;
    };

    template <>
    struct TypeWithSize<8>
    {
        typedef uint64_t UInt;
    };
}


template <typename RawType>
class FloatingPoint
{
public:
    typedef typename TypeWithSize<sizeof(RawType)>::UInt Bits;

    static const size_t kBitCount = 8*sizeof(RawType);
    static const size_t kFracBitCount = std::numeric_limits<RawType>::digits - 1;
    static const size_t kExpBitCount = kBitCount - 1 - kFracBitCount;

    static const Bits kSignBitMask = static_cast<Bits>(1) << (kBitCount - 1);
    static const Bits kFracBitMask = ~static_cast<Bits>(0) >> (kExpBitCount + 1);
    static const Bits kExpBitMask = ~(kSignBitMask | kFracBitMask);

    static const size_t kMaxUlps = 4;


    explicit FloatingPoint(const RawType& x) : value_(x) {}

    //
    // Now checking for NaN to match == behavior.
    //
    bool AlmostEquals(const FloatingPoint& rhs) const {
        if (is_nan() || rhs.is_nan()) return false;
        return ULP_diff(bits_, rhs.bits_) <= kMaxUlps;
    }

private:
    bool is_nan() const {
        return ((kExpBitMask & bits_) == kExpBitMask) &&
	        ((kFracBitMask & bits_) != 0);
    }

    Bits SignAndMagnitudeToBiased(const Bits& sam) const {
        if (kSignBitMask & sam) {
            return ~sam + 1;  // two's complement
        } else {
            return kSignBitMask | sam;  // * 2
        }
    }

    Bits ULP_diff(const Bits& sam1, const Bits& sam2) const
    {
        const Bits biased1 = SignAndMagnitudeToBiased(sam1);
        const Bits biased2 = SignAndMagnitudeToBiased(sam2);

        return (biased1 >= biased2) ? (biased1 - biased2) : (biased2 - biased1);
    }

    union {
        RawType value_;
        Bits bits_;
    };
};

#endif	//NINJA_MATH_UTILITY_H
