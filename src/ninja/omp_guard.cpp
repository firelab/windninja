/******************************************************************************
 *
 * $Id: omp_guard.cpp 823 2011-02-17 14:05:53Z jaforthofer $
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

#include "omp_guard.h"

#ifdef _OPENMP
/** Construct guard object and acquire our lock */
omp_guard::omp_guard (omp_lock_t &lock) : lock_ (&lock)
, owner_ (false)
{
    acquire ();
}
/** Explicitly set our lock */
void omp_guard::acquire ()
{
    omp_set_lock (lock_);
    owner_ = true;
}
/** Explicitly unset our lock.
* Only unset it, though, if we are still the owner.
*/
void omp_guard::release ()
{
    if (owner_) {
        owner_ = false;
        omp_unset_lock (lock_);
    }
}

/** Destruct guard object, release the lock */
omp_guard::~omp_guard ()
{
    release ();
}
#endif
