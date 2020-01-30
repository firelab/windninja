/******************************************************************************
 *
 * $Id: omp_guard.h 823 2011-02-17 14:05:53Z jaforthofer $
 *
 * Project:  WindNinja
 * Purpose:  A class for automatically setting and unsetting omp locks (as
 *              opposed to using a critical section, this way you can throw
 *              out)
 * Author:   obtained from http://www.thinkingparallel.com/2006/08/21/
 *                  scoped-locking-vs-critical-in-openmp-a-personal-shootout/
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

#ifdef _OPENMP

#ifndef OMP_GUARD_H
#define OMP_GUARD_H

#include <omp.h>

/** This is a class for guard objects using OpenMP
*  It is adapted from the book
*  "Pattern-Oriented Software Architecture". */

class omp_guard {

public:

    /** Acquire the lock and store a pointer to it */
    omp_guard (omp_lock_t &lock);

    /** Set the lock explicitly */
    void acquire ();

    /** Release the lock explicitly (owner thread only!) */
    void release ();

    /** Destruct guard object */
    ~omp_guard ();

private:

    omp_lock_t *lock_;  // pointer to our lock
    bool owner_;   // is this object the owner of the lock?

    // Disallow copies or assignment
    omp_guard (const omp_guard &);
    void operator= (const omp_guard &);
};

#endif /*OMP_GUARD_H*/

#endif /* _OPENMP */
