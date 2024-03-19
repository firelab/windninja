/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | 
    \\  /    A nd           | Copyright held by original author
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "logProfileDissipationRateInletFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "volFields.H"
#include "surfaceFields.H"

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

//- set all the various height values specific for each cell of the domain
void Foam::logProfileDissipationRateInletFvPatchScalarField::setTerrainHeights()
{
    // Access to face Centers
    const pointField& faceCenters = patch().Cf();
    scalarField Dist(faceCenters.size(), 0.0);
    
    List<pointField> faceCenterField(Pstream::nProcs());
    label nFace = faceCenters.size();
    reduce(nFace, sumOp<label>());
    
    pointField faceCenterPoints(0);
    faceCenterField[Pstream::myProcNo()] = patch().Cf();
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
            //if current point is not the near-wall center point
	        if(mag(currentPt.z()-faceCenterPoints[pointJ].z()) > 0.1 )
 	        {
		        scalar checkOrdinate_FC = 0.0, checkOrdinate_CP = 0.0;
		        // Lookup patch ids and check if its orientation is along west/east/south/north
		        // If current patch is west_face or east_face, check the Y ordinates
    	        const word& curPatchName = patch().name();
		        //if(curPatchName == "xMin" || curPatchName == "xMax" || curPatchName == "west_face" || curPatchName == "east_face")    // use domains with different face names/orientation
		        if(curPatchName == "west_face" || curPatchName == "east_face")
		        {
		            checkOrdinate_FC = faceCenterPoints[pointJ].y();
		            checkOrdinate_CP = currentPt.y();
		        }
		        //else if(curPatchName == "yMax" || curPatchName == "yMin" || curPatchName == "north_face" || curPatchName == "south_face")    // use domains with different face names/orientation
		        else if(curPatchName == "north_face" || curPatchName == "south_face")
		        {
		            checkOrdinate_FC = faceCenterPoints[pointJ].x();
		            checkOrdinate_CP = currentPt.x();
		        }
		        else
		        {
                    FatalErrorIn("logProfileDissipationRateInletFvPatchScalarField")
 		                    << "Boundary condition applied on incorrect patch"
		                    << abort(FatalError);
    		    }
		        // The bracket for checking the ordinates has been set to 1     // Look for cells close to x or y coord (depending on the face) of currentPt
		        if( mag(checkOrdinate_FC - checkOrdinate_CP) < 10 )             // if( mag(checkOrdinate_FC - checkOrdinate_CP) < 100 )
	            {
                    //if we are close in x or y dimension, 
                    //and closer to the ground than the previous iteration, 
                    //calc and store distance to ground for this face
		            if( faceCenterPoints[pointJ].z() < Dist[pointI] )
		            {
                        //update distance to ground if we are in a lower cell than previous iteration
			            Dist[pointI] = faceCenterPoints[pointJ].z();
			            minPointId = pointJ;    //point with z nearest to ground (the near-ground cell below currentPt)
	    	        }
		        }
	        }
	    }
        
	    // If no other values are found (assuming the bracket range is sufficient)
	    if(Dist[pointI] == GREAT || minPointId == -1 || Dist[pointI] > currentPt.z() )
	    {
	        Dist[pointI] = currentPt.z();
	    }
        
        //Info << "firstCellHeight_ = " << firstCellHeight_ << endl;
        //Info << "Dist[" << pointI << "] = " << Dist[pointI] << endl;
        
	    // Add the half first cell Height read from dictionary
	    Dist[pointI] = currentPt.z() - Dist[pointI] + firstCellHeight_/2;
	    relativeHeight_.setSize(Dist.size());
	    relativeHeight_ = Dist;
        
        //Info << "currentPt.z() = " << currentPt.z() << endl;
        //Info << "relativeHeight_[pointI] = " << Dist[pointI] << endl;
    
	}
    
    
    /////--- method Loren used for porous media simulations on flat terrain at zero altitude
    /////--- the zero altitude resulted in a firstCellHeight smaller than z0, so log(z/z0) went negative
    /////--- seems the above algorythm is to find which cells are in a given non-straight column,
    /////--- the Dist before correction is an estimated lowest cell center value, so the algorythm seems to 
    /////--- be cutting out the altitude in z and replacing it with firstCellHeight_/2 (the estimate tends to be above z0)
    /*// assign the relative heights to the cell centers of the boundary patch rather than 
    //  using the complex WindNinja algorythm. That algorythm is meant for when the dx between
    //  cells varies because of the stretching and refinement done for a WindNinja terrain
    //  mesh. Will have to use a different method when the mesh is no longer a uniform mesh.
    // Access to face Centers, the cell centers of the faces that make up the boundary patch
    const pointField& faceCenters = patch().Cf();
    relativeHeight_.setSize(faceCenters.size());
    forAll(faceCenters, pointI)
	{
        point currentPt = faceCenters[pointI];
        relativeHeight_[pointI] = currentPt.z();
        //Info << "relativeHeight_[pointI] = " << relativeHeight_[pointI] << endl;
    }
    
    // find if any cell has a relativeHeight_ less than z0, found out if
    // all relativeHeights need corrected by z0
    bool foundLowRelativeHeight = false;
    forAll(faceCenters, pointI)
    {
        if( relativeHeight_[pointI] < z0_ )
        {
            foundLowRelativeHeight = true;
            break;
        }
    }
    
    // if any relativeHeight is less than z0, log(relativeHeight_/z0) is negative for quite a few cells
    // correct this problem by adding z0 to all relativeHeights
    if( foundLowRelativeHeight == true )
    {
        Info << "In logProfileDissipationRateInlet::setTerrainHeights(). Found relativeHeights below z0, correcting by adding z0 to all relativeHeights" << endl;
        relativeHeight_ = relativeHeight_ + z0_;
        //Info << "updated relativeHeight_ = " << relativeHeight_ << endl;
    }*/

}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

//- Construct from patch and internal field
Foam::logProfileDissipationRateInletFvPatchScalarField::
logProfileDissipationRateInletFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(p, iF),
    UfreeStream_(0.0),
    uDirection_(0.0,0.0,0.0),
    inputWindHeight_Veg_(0),
    z0_(0),
    Rd_(0),
    relativeHeight_(0),
    firstCellHeight_(0)
{}

//- Construct from patch, internal field and dictionary
Foam::logProfileDissipationRateInletFvPatchScalarField::
logProfileDissipationRateInletFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchScalarField(p, iF, dict, false),
    UfreeStream_(readScalar(dict.lookup("UfreeStream"))),
    uDirection_(dict.lookup("uDirection")),
    inputWindHeight_Veg_(readScalar(dict.lookup("inputWindHeight_Veg"))),
    z0_(readScalar(dict.lookup("z0"))),
    Rd_(readScalar(dict.lookup("Rd"))),
    relativeHeight_(0),
    firstCellHeight_(readScalar(dict.lookup("firstCellHeight")))
{
    setTerrainHeights();

    evaluate();
}

//- Construct by mapping given logProfileDissipationRateInletFvPatchScalarField
//  onto a new patch
Foam::logProfileDissipationRateInletFvPatchScalarField::
logProfileDissipationRateInletFvPatchScalarField
(
    const logProfileDissipationRateInletFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchScalarField(ptf, p, iF, mapper),
    UfreeStream_(ptf.UfreeStream_),
    uDirection_(ptf.uDirection_),
    inputWindHeight_Veg_(ptf.inputWindHeight_Veg_),
    z0_(ptf.z0_),
    Rd_(ptf.Rd_),
    relativeHeight_(ptf.relativeHeight_),
    firstCellHeight_(ptf.firstCellHeight_)
{}

//- Copy constructor
Foam::logProfileDissipationRateInletFvPatchScalarField::
logProfileDissipationRateInletFvPatchScalarField
(
    const logProfileDissipationRateInletFvPatchScalarField& ptf
)
:
    fixedValueFvPatchScalarField(ptf),
    UfreeStream_(ptf.UfreeStream_),
    uDirection_(ptf.uDirection_),
    inputWindHeight_Veg_(ptf.inputWindHeight_Veg_),
    z0_(ptf.z0_),
    Rd_(ptf.Rd_),
    relativeHeight_(ptf.relativeHeight_),
    firstCellHeight_(ptf.firstCellHeight_)
{}

//- Construct and return a clone
// done in the .H file

//- Copy constructor setting internal field reference
Foam::logProfileDissipationRateInletFvPatchScalarField::
logProfileDissipationRateInletFvPatchScalarField
(
    const logProfileDissipationRateInletFvPatchScalarField& ptf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(ptf, iF),
    UfreeStream_(ptf.UfreeStream_),
    uDirection_(ptf.uDirection_),
    inputWindHeight_Veg_(ptf.inputWindHeight_Veg_),
    z0_(ptf.z0_),
    Rd_(ptf.Rd_),
    relativeHeight_(ptf.relativeHeight_),
    firstCellHeight_(ptf.firstCellHeight_)
{}

//- Construct and return a clone setting internal field reference
// done in the .H file

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

//- Update the coefficients associated with the patch field
void Foam::logProfileDissipationRateInletFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

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

    fixedValueFvPatchScalarField::updateCoeffs();
}

//- Write
void Foam::logProfileDissipationRateInletFvPatchScalarField::write(Ostream& os) const
{
    fvPatchScalarField::write(os);
    writeEntry(os, "UfreeStream", UfreeStream_);
    writeEntry(os, "uDirection", uDirection_);
    writeEntry(os, "inputWindHeight_Veg", inputWindHeight_Veg_);
    writeEntry(os, "z0", z0_);
    writeEntry(os, "Rd", Rd_);
    writeEntry(os, "firstCellHeight", firstCellHeight_);
    writeEntry(os, "value", *this);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchScalarField,
        logProfileDissipationRateInletFvPatchScalarField
    );
}

// ************************************************************************* //
