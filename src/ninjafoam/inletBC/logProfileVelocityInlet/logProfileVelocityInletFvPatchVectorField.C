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

#include "logProfileVelocityInletFvPatchVectorField.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "surfaceFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

logProfileVelocityInletFvPatchVectorField::logProfileVelocityInletFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(p, iF),
    UfreeStream_(0.0),
    uDirection_(0.0,0.0,0.0),
    inputWindHeight_Veg_(0),
    z0_(0),
    Rd_(0),
    relativeHeight_(0),
    firstCellHeight_(0)
{}


logProfileVelocityInletFvPatchVectorField::logProfileVelocityInletFvPatchVectorField
(
    const logProfileVelocityInletFvPatchVectorField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchVectorField(ptf, p, iF, mapper),
    UfreeStream_(ptf.UfreeStream_),
    uDirection_(ptf.uDirection_),
    inputWindHeight_Veg_(ptf.inputWindHeight_Veg_),
    z0_(ptf.z0_),
    Rd_(ptf.Rd_),
    relativeHeight_(ptf.relativeHeight_),
    firstCellHeight_(ptf.firstCellHeight_)
{}

/* used later in updateCoeffs() --> 
   this constructor is the one called by simpleFoam */
logProfileVelocityInletFvPatchVectorField::logProfileVelocityInletFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchVectorField(p, iF),
    UfreeStream_(readScalar(dict.lookup("UfreeStream"))),
    uDirection_(dict.lookup("uDirection")),
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
	// Set to high values
	Dist[pointI] = GREAT;
	label minPointId = -1;
	forAll(faceCenterPoints, pointJ)
	{
        //if current point is not the near-wall center point	   
        if(mag(currentPt.z()-faceCenterPoints[pointJ].z()) > .1 )
 	   {
		    scalar checkOrdinate_FC = 0.0, checkOrdinate_CP = 0.0;
		    // Lookup patch ids and check if its orientation is along west/east/south/north
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
            // Look for cells close to x or y coord (depending on the face) of currentPt 
		    if( mag(checkOrdinate_FC - checkOrdinate_CP) < 100 )
	            {
                //if we are close in x or y dimension, 
                //and closer to the ground than the previous iteration, 
                //calc and store distance to ground for this face
		        if( faceCenterPoints[pointJ].z() < Dist[pointI] )
		        {
                    //update distance to ground if we are in a lower cell than previous iteration
			        Dist[pointI] = faceCenterPoints[pointJ].z();
			        minPointId = pointJ; //point with z nearest to ground (the near-ground cell below currentPt)
	    	    }
		    }
	    }
	}

	// If no other values are found (assuming the bracket range is sufficient)
	if(Dist[pointI] == GREAT || minPointId == -1 || Dist[pointI] > currentPt.z() )
	{
	    Dist[pointI] = currentPt.z();
	}

	// Add the half first cell Height read from dictionary
    //Info<<"firstCellHeight_ = "<<firstCellHeight_<<endl;
 	
    Dist[pointI] = currentPt.z() - Dist[pointI] + firstCellHeight_/2;
	relativeHeight_.setSize(Dist.size());
    relativeHeight_ = Dist;

    //Info<<"currentPt.z() = "<<currentPt.z()<<endl;    
    //Info<<"Dist[pointI] = "<<Dist[pointI]<<endl;

	}

    evaluate();
}


logProfileVelocityInletFvPatchVectorField::logProfileVelocityInletFvPatchVectorField
(
    const logProfileVelocityInletFvPatchVectorField& fcvpvf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(fcvpvf, iF),
    UfreeStream_(fcvpvf.UfreeStream_),
    uDirection_(fcvpvf.uDirection_),
    inputWindHeight_Veg_(fcvpvf.inputWindHeight_Veg_),
    z0_(fcvpvf.z0_),
    Rd_(fcvpvf.Rd_),
    relativeHeight_(fcvpvf.relativeHeight_),
    firstCellHeight_(fcvpvf.firstCellHeight_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

//called by simpleFoam
void logProfileVelocityInletFvPatchVectorField::updateCoeffs()
{
    scalar ustar = UfreeStream_*0.41/Foam::log((inputWindHeight_Veg_)/z0_);
    scalar ucalc(0.0);
    vectorField Up(patch().Cf().size(), vector(0.0, 0.0, 0.0));
    
    // Loop over all the faces in that patch
    forAll(Up, faceI )
    {
        // relative height from ground for face lists
        scalar& AGL = relativeHeight_[faceI];

        //Apply the log profile
        ucalc = ustar/0.41*Foam::log((AGL)/z0_);
        Up[faceI] = ucalc*uDirection_;
    }

    operator==(Up);

}

// Write
void logProfileVelocityInletFvPatchVectorField::write(Ostream& os) const
{
    fvPatchVectorField::write(os);
    os.writeKeyword("UfreeStream")
        << UfreeStream_ << token::END_STATEMENT << nl;
    os.writeKeyword("uDirection")
        << uDirection_ << token::END_STATEMENT << nl;
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

makePatchTypeField(fvPatchVectorField, logProfileVelocityInletFvPatchVectorField);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
