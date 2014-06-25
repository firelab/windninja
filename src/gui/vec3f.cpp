/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  OpenGL 3d vector class
 * Author:   See below
 *
 ******************************************************************************
 *
 * SEE COPYRIGHT BELOW
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

/* Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* File for "Terrain" lesson of the OpenGL tutorial on
 * www.videotutorialsrock.com
 */



#include <math.h>

#include "vec3f.h"



Vec3f::Vec3f() {
	
}

Vec3f::Vec3f(float x, float y, float z) {
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

float &Vec3f::operator[](int index) {
	return v[index];
}

float Vec3f::operator[](int index) const {
	return v[index];
}

Vec3f Vec3f::operator*(float scale) const {
	return Vec3f(v[0] * scale, v[1] * scale, v[2] * scale);
}

Vec3f Vec3f::operator/(float scale) const {
	return Vec3f(v[0] / scale, v[1] / scale, v[2] / scale);
}

Vec3f Vec3f::operator+(const Vec3f &other) const {
	return Vec3f(v[0] + other.v[0], v[1] + other.v[1], v[2] + other.v[2]);
}

Vec3f Vec3f::operator-(const Vec3f &other) const {
	return Vec3f(v[0] - other.v[0], v[1] - other.v[1], v[2] - other.v[2]);
}

Vec3f Vec3f::operator-() const {
	return Vec3f(-v[0], -v[1], -v[2]);
}

const Vec3f &Vec3f::operator*=(float scale) {
	v[0] *= scale;
	v[1] *= scale;
	v[2] *= scale;
	return *this;
}

const Vec3f &Vec3f::operator/=(float scale) {
	v[0] /= scale;
	v[1] /= scale;
	v[2] /= scale;
	return *this;
}

const Vec3f &Vec3f::operator+=(const Vec3f &other) {
	v[0] += other.v[0];
	v[1] += other.v[1];
	v[2] += other.v[2];
	return *this;
}

const Vec3f &Vec3f::operator-=(const Vec3f &other) {
	v[0] -= other.v[0];
	v[1] -= other.v[1];
	v[2] -= other.v[2];
	return *this;
}

float Vec3f::magnitude() const {
	return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float Vec3f::magnitudeSquared() const {
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

Vec3f Vec3f::normalize() const {
	float m = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	return Vec3f(v[0] / m, v[1] / m, v[2] / m);
}

float Vec3f::dot(const Vec3f &other) const {
	return v[0] * other.v[0] + v[1] * other.v[1] + v[2] * other.v[2];
}

Vec3f Vec3f::cross(const Vec3f &other) const {
	return Vec3f(v[1] * other.v[2] - v[2] * other.v[1],
				 v[2] * other.v[0] - v[0] * other.v[2],
				 v[0] * other.v[1] - v[1] * other.v[0]);
}

Vec3f operator*(float scale, const Vec3f &v) {
	return v * scale;
}

std::ostream &operator<<(std::ostream &output, const Vec3f &v) {
	std::cout << '(' << v[0] << ", " << v[1] << ", " << v[2] << ')';
	return output;
}









