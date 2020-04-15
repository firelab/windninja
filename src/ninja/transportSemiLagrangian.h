/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  TransportSemiLagrangian 
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

#ifndef TRANSPORTSEMILAGRANGIAN_H
#define TRANSPORTSEMILAGRANGIAN_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "ninjaException.h"
#include "wn_3dVectorField.h"

#include "mesh.h"

#ifdef _OPENMP
#include <omp.h>
#endif

class TransportSemiLagrangian
{
    public:
        TransportSemiLagrangian();
        ~TransportSemiLagrangian();
        
        void transportVector(const wn_3dVectorField &U0, wn_3dVectorField &U1, double dt);
        void transportScalar(const wn_3dVectorField &U0, const wn_3dScalarField &S0, wn_3dScalarField &S1, double dt);
        
        enum eTransportType{
            firstOrderTransport,
            settls,
            secondOrderRungeKutta,
            adaptiveParticleTracer
        };
        
        enum eInterpolationType{
            firstOrderInterpolation,
            secondOrderInterpolation
        };

        eTransportType transportType;
        eInterpolationType interpolationType;

    private:
        void traceParticle(const wn_3dVectorField &U0, const double &dt, int &startI, int &startJ, int &startK, double &endX, double &endY, double &endZ);
        
};

#endif	//TRANSPORTSEMILAGRANGIAN_H
