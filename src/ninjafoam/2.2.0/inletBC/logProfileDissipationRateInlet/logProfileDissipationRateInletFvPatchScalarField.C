/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright held by original author
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

\*---------------------------------------------------------------------------*/

#include "logProfileDissipationRateInletFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "surfaceFields.H"
#include "RASModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

logProfileDissipationRateInletFvPatchScalarField::logProfileDissipationRateInletFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(p, iF),
    UfreeStream_(0.0),
    inputWindHeight_Veg_(0),
    z0_(0),
    Rd_(0),
    relativeHeight_(0),
    firstCellHeight_(0)
{}


logProfileDissipationRateInletFvPatchScalarField::logProfileDissipationRateInletFvPatchScalarField
(
    const logProfileDissipationRateInletFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchScalarField(ptf, p, iF, mapper),
    UfreeStream_(ptf.UfreeStream_),
    inputWindHeight_Veg_(ptf.inputWindHeight_Veg_),
    z0_(ptf.z0_),
    Rd_(ptf.Rd_),
    relativeHeight_(ptf.relativeHeight_),
    firstCellHeight_(ptf.firstCellHeight_)
{}


logProfileDissipationRateInletFvPatchScalarField::logProfileDissipationRateInletFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchScalarField(p, iF),
    UfreeStream_(readScalar(dict.lookup("UfreeStream"))),
    inputWindHeight_Veg_(readScalar(dict.lookup("inputWindHeight_Veg"))),
    z0_(readScalar(dict.lookup("z0"))),
    Rd_(readScalar(dict.lookup("Rd"))),
    firstCellHeight_(readScalar(dict.lookup("firstCellHeight")))
{
    // Access to face Centers
    const pointField& faceCenters = patch().Cf();
    scalarField Dist(faceCenters.size(), 0.0);

    List<pointField> faceCenterField(Pstream::nProcs()); 
    label nFace = faceCenters.size();
    reduce(nFace, sumOp<label>()); 

    pointField faceCenterPoints(0);
    faceCenterField[Pstream::myProcNo()]=patch().Cf();
    Pstream::gatherList(faceCenterField);

    if(Pstream::master())
    {	
	forAll(faceCenterField, procI)
	{
		forAll(faceCenterField[procI],pointI)
		{
			faceCenterPoints.append(faceCenterField[procI][pointI]);
		}
	}
    }
    Pstream::scatter(faceCenterPoints);
    forAll(faceCenters, pointI)
	{

	point currentPt = faceCenters[pointI];
	// Set to High Values
	Dist[pointI] = GREAT;
	label minPointId = -1;
	forAll(faceCenterPoints, pointJ)
	{
	   if(mag(currentPt.z()-faceCenterPoints[pointJ].z()) > .1 )
 	   {
		scalar checkOrdinate_FC = 0.0, checkOrdinate_CP = 0.0;
		// Lookup for patch ids and check if its orientation is along west/east/south/north
		// If current patch is west_face or east_face, check the Y ordinates
    		const word& curPatchName = patch().name();
		if(curPatchName == "west_face" || curPatchName == "east_face")
		{
		    checkOrdinate_FC = faceCenterPoints[pointJ].y();
		    checkOrdinate_CP = currentPt.y();
		}
		else if(curPatchName == "north_face" || curPatchName == "south_face")
		     {
		    	checkOrdinate_FC = faceCenterPoints[pointJ].x() ;
		    	checkOrdinate_CP = currentPt.x();
		     }
		else
		{
         	    FatalErrorIn("logProfileVelocityInletFvPatchVectorField")
     		       << "Boundary condition applied on incorrect patch"
		       << abort(FatalError);
    		}
		// The bracket for checking the ordinates has been set to 1
		if( mag(checkOrdinate_FC - checkOrdinate_CP) < 10 )
	        {
		    if( faceCenterPoints[pointJ].z() < Dist[pointI] )
		    {
			Dist[pointI] = faceCenterPoints[pointJ].z();
			minPointId = pointJ;
	    	    }
		}
	    }
	}

	// If no other values are found assuming the bracket range is sufficient
	if(Dist[pointI] == GREAT || minPointId == -1 || Dist[pointI] > currentPt.z() )
	{
	    Dist[pointI] = currentPt.z();
	}
	// Add the half first cell Height read from dictionary
	Dist[pointI] = currentPt.z() - Dist[pointI] + firstCellHeight_/2;
	relativeHeight_.setSize(Dist.size());
	relativeHeight_ = Dist;
	}
    evaluate();
}


logProfileDissipationRateInletFvPatchScalarField::logProfileDissipationRateInletFvPatchScalarField
(
    const logProfileDissipationRateInletFvPatchScalarField& fcvpvf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(fcvpvf, iF),
    UfreeStream_(fcvpvf.UfreeStream_),
    inputWindHeight_Veg_(fcvpvf.inputWindHeight_Veg_),
    z0_(fcvpvf.z0_),
    Rd_(fcvpvf.Rd_),
    relativeHeight_(fcvpvf.relativeHeight_),
    firstCellHeight_(fcvpvf.firstCellHeight_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void logProfileDissipationRateInletFvPatchScalarField::updateCoeffs()
{
    // Caluculate Input Wind Height
    scalarField epsilonp(patch().Cf().size(), scalar(0));

    scalar kappa = 0.4187;
    scalar ustar = UfreeStream_*0.41/log((inputWindHeight_Veg_)/z0_);

    // Loop over all the faces in that patch
    forAll(epsilonp, faceI)
    {
        // relative height from ground for face lists
        scalar& AGL = relativeHeight_[faceI];
        epsilonp[faceI] = pow(ustar,3)/(kappa*(AGL + z0_));
    }

    operator==(epsilonp);

}

// Write
void logProfileDissipationRateInletFvPatchScalarField::write(Ostream& os) const
{
    fvPatchScalarField::write(os);
    os.writeKeyword("UfreeStream")
        << UfreeStream_ << token::END_STATEMENT << nl;
    os.writeKeyword("inputWindHeight_Veg")
        << inputWindHeight_Veg_ << token::END_STATEMENT << nl;
    os.writeKeyword("z0")
          << z0_ << token::END_STATEMENT << nl;
      os.writeKeyword("Rd")
          << Rd_ << token::END_STATEMENT << nl;
    os.writeKeyword("firstCellHeight")
	<< firstCellHeight_ << token::END_STATEMENT << nl;
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField(fvPatchScalarField, logProfileDissipationRateInletFvPatchScalarField);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
