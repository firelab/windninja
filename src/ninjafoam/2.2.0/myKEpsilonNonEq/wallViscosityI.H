/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2008 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Global
    wallViscosity

Description
    Calculate wall viscosity from wall-functions.

\*---------------------------------------------------------------------------*/

{
    scalar Cmu25 = pow(Cmu_.value(), 0.25);

    scalar ypl = 11.0;

    for (int i=0; i<10; i++)
    {
        ypl = log(max(E_*ypl, 1))/kappa_;
    }

    const scalar yPlusLam_ = ypl;


    const fvPatchList& patches = mesh_.boundary();

    forAll(patches, patchi)
    {
        const fvPatch& curPatch = patches[patchi];
        int iter = this->turbulenceModel::db().time().value();
        int freq = nonEqFreq_;
        if(iter>=nonEqFreqStart_ && iter<=nonEqFreqEnd_) freq = nonEqFreq_;
        //else if(iter>500 && iter<=700) freq = nonEqFreq_/5;
        else if(iter<nonEqFreqStart_ || iter>nonEqFreqEnd_) freq = -1;

        if (isType<wallFvPatch>(curPatch))
        {
            const scalarField& nuw = nu()().boundaryField()[patchi];
            scalarField& nutw = nut_.boundaryField()[patchi];
            scalarField psiw_(nuw.size());
            forAll(curPatch, facei)
               psiw_[facei] = 1.0;

            if(freq <=0) // if freq <= 0, skip non-equilibrium function
		;
            else if((iter>=nonEqFreqStart_) && ((iter % freq)==0))
            {
               //***********Non-equilibrium wall function starts**********//
               cout << "\nAdding non-equilibrium effects..." <<nl;
               const volScalarField& p = this->turbulenceModel::db().lookupObject<volScalarField>("p");
               const volVectorField& U = this->turbulenceModel::db().lookupObject<volVectorField>("U");
               const volVectorField gradP = fvc::grad(p);
               const volTensorField gradU = fvc::grad(U);
               forAll(curPatch, facei)
               {
                   label faceCelli = curPatch.faceCells()[facei];
                   vector Cu_ = gradP[faceCelli] + (U[faceCelli] & gradU[faceCelli]);
                   scalar magU = mag(U[faceCelli]);
                   scalar Cux = Cu_.x()*U[faceCelli].x() + Cu_.y()*U[faceCelli].y() + \
                                Cu_.z()*U[faceCelli].z();
                   Cux /= magU;
                   scalar uStar = Cmu25 * sqrt(k_[faceCelli]);
                   psiw_[facei] -= (Cux*y_[patchi][facei]) / (magU*kappa_*uStar + SMALL);
               }
               cout << "psiw: min " << min(psiw_) << "\tmax " << max(psiw_) << "\taverage " << average(psiw_) <<nl;

               //***********Non-equilibrium wall function ends************//
            }

            forAll(curPatch, facei)
            {
		label faceCelli = curPatch.faceCells()[facei];

                scalar yPlus =
                    Cmu25*y_[patchi][facei]*sqrt(k_[faceCelli])/nuw[facei];

		scalar uStar = Cmu25 * sqrt(k_[faceCelli]);
		scalar KsPlus = Ks_.value() * uStar / nuw[facei];

		// Hydrodynamically Smooth
		scalar logDB = 1.0;

		if (KsPlus > 2.25 )
		{
		    // fully Rough
		    if (KsPlus > 90.0 )
		    {
			logDB = 1.0 + Cs_.value() * KsPlus;
		    }
		    else // Transitional
		    {
		    	if (Ks_.value() > 0.0)
		    	{
			    scalar n = ::sin( 0.4258 * (log(KsPlus) - 0.811 ));
			    scalar a = (KsPlus - 2.25 ) / 87.75 + Cs_.value() * KsPlus ;
			    logDB = ::pow(a, n);
			}
		    }
		}

                if (yPlus > yPlusLam_ || Ks_.value() > 0)
                {
                      // Non-equilibrium wall-function
                      nutw[facei] =
                         min(nuw[facei]*(yPlus*kappa_*psiw_[facei]/log(E_*yPlus/logDB) - 1), 10000 * nuw[facei]);
                }
                else
                {
                    nutw[facei] = 0.0;
                }
            }
        }
    }
}


// ************************************************************************* //
