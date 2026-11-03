/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2026 OpenFOAM Foundation  or  Copyright held by original author
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

#include "calcTerrainHeights.H"
#include "addToRunTimeSelectionTable.H"
#include <map>
#include "Function1.H"
#include "Table.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{
    defineTypeNameAndDebug(calcTerrainHeights, 0);
    addToRunTimeSelectionTable(functionObject, calcTerrainHeights, dictionary);
}
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::functionObjects::calcTerrainHeights::computeTerrainHeights()
{
    tmp<volScalarField> tZAGL
    (
        volScalarField::New
        (
            "zAGL",
            mesh_,
            dimensionedScalar("zero", dimLength, 0.0)
        )
    );

    volScalarField& zAGL = tZAGL.ref();

    label minZPatchID = mesh_.boundaryMesh().findPatchID(groundPatchName_);
    if(minZPatchID == -1)
    {
        FatalErrorIn("computeTerrainHeights")
            << "Could not find a patch named '" << groundPatchName_ << "' in the mesh"
            << abort(FatalError);
    }
    const fvPatch& minZPatch = mesh_.boundary()[minZPatchID];
    const pointField& minZ_points = minZPatch.patch().localPoints();

    forAll(patchNames_, patchI)
    {
        const word& curPatchName = patchNames_[patchI];
        label patchID = mesh_.boundaryMesh().findPatchID(curPatchName);
        if(patchID == -1)
        {
            WarningInFunction
                << "Could not find a patch named '" << curPatchName << "' in the mesh"
                << abort(FatalError);
            continue;
        }

        const fvPatch& patch = mesh_.boundary()[patchID];
        const pointField& faceCenters = patch.Cf();

        direction normDir = vector::X;
        direction tanDir = vector::Y;
        if(curPatchName == "west_face" || curPatchName == "east_face")
        {
            // inlet points in the x direction
            normDir = vector::X;
            tanDir = vector::Y;
        }
        else if(curPatchName == "north_face" || curPatchName == "south_face")
        {
            // inlet points in the y direction
            normDir = vector::Y;
            tanDir = vector::X;
        }
        else
        {
            FatalErrorIn("computeTerrainHeights")
                << "unable to apply axis logic for patch " << curPatchName
                << abort(FatalError);
        }

        scalar patchNormPos = -GREAT;
        if(faceCenters.size() > 0)
        {
            patchNormPos = faceCenters[0].component(normDir);  // the x or y common position, of the patch, to compare against
        }
        if(Pstream::parRun())
        {
            // safely get the value from the other processors, for those with no faceCenters,
            // because valid coords will be always be greater than -GREAT
            reduce(patchNormPos, maxOp<scalar>());
        }

        // Filter and collect local profiles
        scalar tolerance = 0.1;  // search distance away from the inlet patch, of the minZ patch
        scalarField localPoints_tanPos(minZ_points.size());
        scalarField localPoints_zPos(minZ_points.size());
        label localPoints_count = 0;
        forAll(minZ_points, pointI)
        {
            // search by the normal coordinate
            if(abs(minZ_points[pointI].component(normDir) - patchNormPos) < tolerance)
            {
                localPoints_tanPos[localPoints_count] = minZ_points[pointI].component(tanDir);
                localPoints_zPos[localPoints_count] = minZ_points[pointI].z();
                localPoints_count++;
            }
        }
        localPoints_tanPos.setSize(localPoints_count);
        localPoints_zPos.setSize(localPoints_count);

        // Gather (parallel communication) data arrays across parallel subdomains
        List<scalarField> gatheredPoints_tanPos;
        List<scalarField> gatheredPoints_zPos;
        if(Pstream::parRun())
        {
            gatheredPoints_tanPos.setSize(Pstream::nProcs());
            gatheredPoints_zPos.setSize(Pstream::nProcs());

            gatheredPoints_tanPos[Pstream::myProcNo()] = localPoints_tanPos;
            gatheredPoints_zPos[Pstream::myProcNo()] = localPoints_zPos;

            Pstream::gatherList(gatheredPoints_tanPos);
            Pstream::gatherList(gatheredPoints_zPos);
        }
        else
        {
            // sets up a single full list of points for when not a parallel run
            gatheredPoints_tanPos.setSize(1);
            gatheredPoints_zPos.setSize(1);
            gatheredPoints_tanPos[0] = localPoints_tanPos;
            gatheredPoints_zPos[0] = localPoints_zPos;
        }

        // master processes and sorts the clean global minZ 1D line list of points
        // into a profile map (std::map also sorts the points)
        // notice this is setup such that it also works for non-parallel processes,
        // just moves around the single full list, sorting the minZ 1D line list of points
        scalarField globalFlat_tanPos;
        scalarField globalFlat_zPos;
        if(Pstream::master())
        {
            std::map<scalar, scalar> globalGroundProfileMap;

            forAll(gatheredPoints_tanPos, procI)
            {
                forAll(gatheredPoints_tanPos[procI], itemI)
                {
                    // Map automatically handles sorting by the tangential coordinate
                    const scalar current_tanPos = gatheredPoints_tanPos[procI][itemI];
                    const scalar current_zPos = gatheredPoints_zPos[procI][itemI];
                    globalGroundProfileMap[current_tanPos] = current_zPos;
                }
            }

            if(globalGroundProfileMap.empty())
            {
                FatalErrorIn("computeTerrainHeights")
                    << "No matching minZ faces found near the inlet patch across any processors"
                    << abort(FatalError);
            }

            // Flatten the map back out into simple fields
            globalFlat_tanPos.setSize(globalGroundProfileMap.size());
            globalFlat_zPos.setSize(globalGroundProfileMap.size());
            label idx = 0;
            for(const std::pair<const scalar, scalar>& pair : globalGroundProfileMap)
            {
                globalFlat_tanPos[idx] = pair.first;
                globalFlat_zPos[idx] = pair.second;
                idx++;
            }
        }

        // scatter (parallel communication) the full sorted list of minZ 1D line points,
        // to each separate processor, for use in the final calculations
        // note the sizing stuff, the size was set above for Pstream::master() but not yet for the rest of the threads
        // Pstream::scatter() behaves like a fence, it takes the Pstream::master() values and spreads them to the rest of the threads
        if(Pstream::parRun())
        {
            // Broadcast the final unified size from Master so slaves can allocate memory
            label globalSize = globalFlat_tanPos.size();  // slaves get a value of 0, master gets the proper value
            Pstream::scatter(globalSize);  // spread the value from master to slaves, with a fence

            // Slaves must resize their local receivers to match the master's size
            if(!Pstream::master())
            {
                globalFlat_tanPos.setSize(globalSize);
                globalFlat_zPos.setSize(globalSize);
            }

            // Now send the actual data, with a fence
            Pstream::scatter(globalFlat_tanPos);
            Pstream::scatter(globalFlat_zPos);
        }

        // skip the calculations if no faceCenters to act on
        // still needed the above just in case no faceCenters, but still some minZ 1D line points on the given processor domain
        // note that this continues, rather than returns, staying within the function till the end of the function
        //  A return too early while in parallel would break things!
        if(faceCenters.empty())
        {
            continue;
        }

        // Repack into Tuple2 format for the interpolation Table constructor
        // Pack values into the standard interpolator table structure
        List<Tuple2<scalar, scalar>> globalTableEntries(globalFlat_tanPos.size());
        forAll(globalTableEntries, i)
        {
            globalTableEntries[i] = Tuple2<scalar, scalar>(globalFlat_tanPos[i], globalFlat_zPos[i]);
        }

        // Set out-of-bounds behavior to CLAMP (retains edge values if inlet exceeds minZ bounds)
        dictionary tableDict;
        tableDict.add("values", globalTableEntries);
        tableDict.add("outOfBounds", "clamp");
        Foam::Function1s::Table<scalar> zGroundTable("zGroundTable", tableDict);

        // Assign local face data mapping inside the 3D volScalarField's boundary registry
        scalarField& patchzAGL = zAGL.boundaryFieldRef()[patchID];
        forAll(faceCenters, faceI)
        {
            // do the calculation in the tangential coordinate direction
            scalar current_coord = faceCenters[faceI].component(tanDir);
            scalar z_ground = zGroundTable.value(current_coord);

            scalar z_face = faceCenters[faceI].z();
            scalar z_AGL = z_face - z_ground;

            patchzAGL[faceI] = z_AGL;
        }
    }

    // Move store() completely outside of the master block so ALL processors register it
    // CRITICAL: Store at the absolute end, outside the loop!
    // Every single processor safely hits this line together, even if they skipped the math loops above.
    this->store("zAGL", tZAGL);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::functionObjects::calcTerrainHeights::calcTerrainHeights
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    fvMeshFunctionObject(name, runTime, dict),
    patchNames_(),
    groundPatchName_(dict.lookup("groundPatchName"))
{
    read(dict);
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::functionObjects::calcTerrainHeights::~calcTerrainHeights()
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::functionObjects::calcTerrainHeights::read(const dictionary& dict)
{
    fvMeshFunctionObject::read(dict);

    dict.lookup("patches") >> patchNames_;
    dict.lookup("groundPatchName") >> groundPatchName_;

    return true;
}

bool Foam::functionObjects::calcTerrainHeights::execute()
{
    // only compute zAGL here the first timestep, when zAGL has not yet been generated.
    // zAGL will then only be updated by the various mesh.changed() functions
    if(!mesh_.foundObject<volScalarField>("zAGL"))
    {
        computeTerrainHeights();
    }

    return true;
}

bool Foam::functionObjects::calcTerrainHeights::write()
{
    // Retrieve the registered field from the mesh database
    const volScalarField& zAGL = mesh_.lookupObject<volScalarField>("zAGL");

    // Write it directly to the active time directory (e.g. 52/zAGL)
    zAGL.write();

    return true;
}

void Foam::functionObjects::calcTerrainHeights::movePoints(const polyMesh& mesh)
{
    computeTerrainHeights();
}

void Foam::functionObjects::calcTerrainHeights::topoChange(const polyTopoChangeMap& map)
{
    computeTerrainHeights();
}

void Foam::functionObjects::calcTerrainHeights::mapMesh(const polyMeshMap& map)
{
    computeTerrainHeights();
}

void Foam::functionObjects::calcTerrainHeights::distribute(const polyDistributionMap&)
{
    computeTerrainHeights();
}

// ************************************************************************* //
