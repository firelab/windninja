/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Exception classes used in WindNinja
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

#ifndef NINJA_EXCEPTION_H
#define NINJA_EXCEPTION_H

#include <string.h>
#include <stdexcept>
#include <vector>
#include <string>
	
// Some useful exceptions available from the <stdexcept> header file are:
//		exception				->		a generic exception providing the "what" functionality, and a copy constructor and assignment operator
//			std::runtime_error		->		inherits from exception, adds a constructor that takes a standard string object, which is used to generate the C-string returned by the "what" function
//				std::range_error		->		inherits from std::runtime_error, used for range problems
//				overflow_error	->		inherits from std::runtime_error, used for overflow problems
//				underflow_error	->		inherits from std::runtime_error, used for underflow problems
//			std::logic_error			->		inherits from exception, 
//				std::domain_error	->		inherits from std::logic_error, used when parameters are passed to a function that are outside of it's domain, example is square root of a negative number
//				invalid_argument->		inherits from std::logic_error, used when function arguments are invalid
//				length_error	->		inherits from std::logic_error, used when a length error occurs
//				std::out_of_range	->		inherits from std::logic_error, used when out of range error occures
//			bad_alloc			->		inherits from exception, returned when "new" can't allocate requested memory
//
//	WITH ABOVE, REMEMBER TO USE std:: NAMESPACE!!


#ifdef WIN32
#define NOEXCEPT 
#else
#define NOEXCEPT noexcept
#endif 

// Below are user-defined exceptions
class cancelledByUser : public std::runtime_error {
public:
	cancelledByUser() : std::runtime_error("Simulation was cancelled by the user.") { }
};

class badForecastFile : public std::runtime_error {
public:
    badForecastFile(const std::string& message) : std::runtime_error(message) { }
};

class guiInitializationError : public std::runtime_error
{
 public:
   guiInitializationError( std::string message );
};

class armyException : public std::runtime_error
{
    public:
        armyException(std::vector<std::string> m);
        ~armyException() throw();
        std::vector<std::string> messages;
        const char* what() const NOEXCEPT override;
};


#endif /* NINJA_EXCEPTION_H */
