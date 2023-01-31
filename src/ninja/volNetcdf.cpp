/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for writing volume and surface netcdf files
 * Author:   Loren Atwood <pianotocador@gmail.com>
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

#include "volNetcdf.h"

volNetcdf::volNetcdf()
{

}

volNetcdf::volNetcdf(wn_3dScalarField const& u, wn_3dScalarField const& v, wn_3dScalarField const& w, 
                     wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
                     int nCols, int nRows, int nLayers, std::string filename,  
                     std::string prjString, double meshRes, bool convertToTrueLatLong)
{
    //std::cout << "nCols = \"" << nCols << "\", nRows = \"" << nRows << "\", nLayers = \"" << nLayers << "\"" << std::endl;
    
    //prjString_latLong = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]";   // original, found online
    //prjString_latLong = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AXIS[\"Latitude\",NORTH],AXIS[\"Longitude\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]";   // output from exportToWkt
    
    // better method for getting the CORRECT lat/long projection string, the original one that was used seemed to cause problems
    // nope, looks like the problem is caused by something else. Still seems best to get the lat/long projection string from code libraries instead of using a hard coded value
    //  so I'm keeping this here. Plus these are good examples for the various ways to get the value.
    OGRSpatialReference oSRS;
    char* pszWKT = NULL;
    oSRS.SetWellKnownGeogCS( "EPSG:4326" );
    //oSRS.importFromEPSG( 4326 );  // alternative method
    //oSRS.SetWellKnownGeogCS("WGS84");  // yet another alternative method, I'm surprised that this one works as it doesn't seem as specific as the other ones. Same result though
    oSRS.exportToWkt( &pszWKT );    // normally only needed for getting the spatial reference in terms of projection string, which is required for warpVRT-like calls
    prjString_latLong = pszWKT;
    // seems to still be causing problems, because lat values are still not correct, might have to skip trying to use prjString for the transformation
    
    writeVolNetcdf(u, v, w, x, y, z, nCols, nRows, nLayers, filename,  prjString, meshRes, convertToTrueLatLong);
}

volNetcdf::~volNetcdf()
{

}

bool volNetcdf::writeVolNetcdf(wn_3dScalarField const& u, wn_3dScalarField const& v, wn_3dScalarField const& w, 
                               wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
                               int nCols, int nRows, int nLayers, std::string filename,  
                               std::string prjString, double meshRes, bool convertToTrueLatLong)
{
    
    // define error handling index var
    int retval;
    
    // create output file
    // the NC_NETCDF4 flag tells netCDF to create a netCDF-4/HDF5 file, the NC_CLOBBER flag tells netCDF to overwrite this file if it already exists
    int ncid;   // ID for holding the netcdf file
    if ((retval = nc_create( filename.c_str(), NC_NETCDF4|NC_CLOBBER, &ncid )))
        ERR(retval);
    
    // add global attributes
    // turns out to be the same method as adding a regular attribute, but using NC_GLOBAL instead of varid for the variable ID
    std::string WindNinja_Version = NINJA_VERSION_STRING;
    if ((retval = nc_put_att_text( ncid, NC_GLOBAL, "WindNinja_Version", WindNinja_Version.length(), WindNinja_Version.c_str() )))
        ERR(retval);
    if ( convertToTrueLatLong == true )
    {
        if ((retval = nc_put_att_text( ncid, NC_GLOBAL, "PROJCS", prjString_latLong.length(), prjString_latLong.c_str() )))
            ERR(retval);
    } else
    {
        if ((retval = nc_put_att_text( ncid, NC_GLOBAL, "PROJCS", prjString.length(), prjString.c_str() )))
            ERR(retval);
    }
    if ((retval = nc_put_att_double( ncid, NC_GLOBAL, "Mesh_Resolution", NC_DOUBLE, 1, &meshRes )))
        ERR(retval);
    
    
    // create dimensions
    int x_dimid;    // ID for holding the x dimension
    if ((retval = nc_def_dim( ncid, "nx", nCols, &x_dimid )))
        ERR(retval);
    int y_dimid;    // ID for holding the y dimension
    if ((retval = nc_def_dim( ncid, "ny", nRows, &y_dimid )))
        ERR(retval);
    int z_dimid;    // ID for holding the z dimension
    if ((retval = nc_def_dim( ncid, "nz", nLayers, &z_dimid )))
        ERR(retval);
    
    
    // create dimension vectors of the dimensions for use in creating variables of the dimensions
    // the dimids arrays are used to pass the IDs of the dimensions of the variable
    // useful method for going from vectors to arrays so that you can use vectors instead of arrays for this
    // https://stackoverflow.com/questions/2923272/how-to-convert-vector-to-array
    // otherwise stuck using dynamic arrays, so stuff like int* dimids_3D = new int[3]; which would require a delete[] dimids_3D; 
    //  at the end of the scope or you would get memory leak.
    // the above article may also help explain some of the interesting syntax of the netcdf c write commands
    std::vector<int> dimids_2D;
    dimids_2D.push_back(y_dimid);
    dimids_2D.push_back(x_dimid);
    
    std::vector<int> dimids_3D;
    dimids_3D.push_back(z_dimid);
    dimids_3D.push_back(y_dimid);
    dimids_3D.push_back(x_dimid);
    
    
    // now create variables for the dimensions
    
    int y_varid;     // ID for holding the y variable
    //if ((retval = nc_def_var( ncid, "y", NC_DOUBLE, dimids_2D.size(), &dimids_2D[0], &y_varid )))    // for vector form of dimids. &dimids[0] converts from std::vector<int> to pointer to array int*
    if ((retval = nc_def_var( ncid, "y", NC_DOUBLE, dimids_2D.size(), dimids_2D.data(), &y_varid )))    // for improved vector form of dimids, .data() acts like &dimids[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval);
    std::string y_units = "m";
    if ((retval = nc_put_att_text( ncid, y_varid, "units", y_units.length(), y_units.c_str() )))
        ERR(retval);
    std::string y_long_name = "2D point array of position, y component";
    if ((retval = nc_put_att_text( ncid, y_varid, "long_name", y_long_name.length(), y_long_name.c_str() )))
        ERR(retval);
    std::string y_coords = "x y";
    if ((retval = nc_put_att_text( ncid, y_varid, "coordinates", y_coords.length(), y_coords.c_str() )))
        ERR(retval);
    
    int x_varid;     // ID for holding the x variable
    //if ((retval = nc_def_var( ncid, "x", NC_DOUBLE, dimids_2D.size(), &dimids_2D[0], &x_varid )))    // for vector form of dimids. &dimids[0] converts from std::vector<int> to pointer to array int*
    if ((retval = nc_def_var( ncid, "x", NC_DOUBLE, dimids_2D.size(), dimids_2D.data(), &x_varid )))    // for improved vector form of dimids, .data() acts like &dimids[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval);
    std::string x_units = "m";
    if ((retval = nc_put_att_text( ncid, x_varid, "units", x_units.length(), x_units.c_str() )))
        ERR(retval);
    std::string x_long_name = "2D point array of position, x component";
    if ((retval = nc_put_att_text( ncid, x_varid, "long_name", x_long_name.length(), x_long_name.c_str() )))
        ERR(retval);
    std::string x_coords = "x y";
    if ((retval = nc_put_att_text( ncid, x_varid, "coordinates", x_coords.length(), x_coords.c_str() )))
        ERR(retval);
    
    
    int z_varid;     // ID for holding the z variable
    //if ((retval = nc_def_var( ncid, "z", NC_DOUBLE, dimids_3D.size(), &dimids_3D[0], &z_varid )))    // for vector form of dimids. &dimids[0] converts from std::vector<int> to pointer to array int*
    if ((retval = nc_def_var( ncid, "z", NC_DOUBLE, dimids_3D.size(), dimids_3D.data(), &z_varid )))    // for improved vector form of dimids, .data() acts like &dimids[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval);
    std::string z_units = "m";
    if ((retval = nc_put_att_text( ncid, z_varid, "units", z_units.length(), z_units.c_str() )))
        ERR(retval);
    std::string z_long_name = "3D point array of position, z component, in units of above ground level (AGL)";
    if ((retval = nc_put_att_text( ncid, z_varid, "long_name", z_long_name.length(), z_long_name.c_str() )))
        ERR(retval);
    std::string z_coords = "x y";
    if ((retval = nc_put_att_text( ncid, z_varid, "coordinates", z_coords.length(), z_coords.c_str() )))
        ERR(retval);
    
    
    // now create all other variables
    
    int terrain_varid;     // ID for holding the terrain variable
    //if ((retval = nc_def_var( ncid, "terrain", NC_DOUBLE, dimids_2D.size(), &dimids_2D[0], &terrain_varid )))    // for vector form of dimids. &dimids[0] converts from std::vector<int> to pointer to array int*
    if ((retval = nc_def_var( ncid, "terrain", NC_DOUBLE, dimids_2D.size(), dimids_2D.data(), &terrain_varid )))    // for improved vector form of dimids, .data() acts like &dimids[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval);
    std::string terrain_units = "m";
    if ((retval = nc_put_att_text( ncid, terrain_varid, "units", terrain_units.length(), terrain_units.c_str() )))
        ERR(retval);
    std::string terrain_long_name = "terrain height in units of above sea level (ASL)";
    if ((retval = nc_put_att_text( ncid, terrain_varid, "long_name", terrain_long_name.length(), terrain_long_name.c_str() )))
        ERR(retval);
    std::string terrain_coords = "x y";
    if ((retval = nc_put_att_text( ncid, terrain_varid, "coordinates", terrain_coords.length(), terrain_coords.c_str() )))
        ERR(retval);
    
    int u_varid;     // ID for holding the u variable
    //if ((retval = nc_def_var( ncid, "u", NC_DOUBLE, dimids_3D.size(), &dimids_3D[0], &u_varid )))    // for vector form of dimids. &dimids[0] converts from std::vector<int> to pointer to array int*
    if ((retval = nc_def_var( ncid, "u", NC_DOUBLE, dimids_3D.size(), dimids_3D.data(), &u_varid )))    // for improved vector form of dimids, .data() acts like &dimids[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval);
    std::string u_units = "m s-1";
    if ((retval = nc_put_att_text( ncid, u_varid, "units", u_units.length(), u_units.c_str() )))
        ERR(retval);
    std::string u_long_name = "velocity, u component";
    if ((retval = nc_put_att_text( ncid, u_varid, "long_name", u_long_name.length(), u_long_name.c_str() )))
        ERR(retval);
    std::string u_coords = "x y";
    if ((retval = nc_put_att_text( ncid, u_varid, "coordinates", u_coords.length(), u_coords.c_str() )))
        ERR(retval);
    
    int v_varid;     // ID for holding the v variable
    //if ((retval = nc_def_var( ncid, "v", NC_DOUBLE, dimids_3D.size(), &dimids_3D[0], &v_varid )))    // for vector form of dimids. &dimids[0] converts from std::vector<int> to pointer to array int*
    if ((retval = nc_def_var( ncid, "v", NC_DOUBLE, dimids_3D.size(), dimids_3D.data(), &v_varid )))    // for improved vector form of dimids, .data() acts like &dimids[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval);
    std::string v_units = "m s-1";
    if ((retval = nc_put_att_text( ncid, v_varid, "units", v_units.length(), v_units.c_str() )))
        ERR(retval);
    std::string v_long_name = "velocity, v component";
    if ((retval = nc_put_att_text( ncid, v_varid, "long_name", v_long_name.length(), v_long_name.c_str() )))
        ERR(retval);
    std::string v_coords = "x y";
    if ((retval = nc_put_att_text( ncid, v_varid, "coordinates", v_coords.length(), v_coords.c_str() )))
        ERR(retval);
    
    int w_varid;     // ID for holding the w variable
    //if ((retval = nc_def_var( ncid, "w", NC_DOUBLE, dimids_3D.size(), &dimids_3D[0], &w_varid )))    // for vector form of dimids. &dimids[0] converts from std::vector<int> to pointer to array int*
    if ((retval = nc_def_var( ncid, "w", NC_DOUBLE, dimids_3D.size(), dimids_3D.data(), &w_varid )))    // for improved vector form of dimids, .data() acts like &dimids[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval);
    std::string w_units = "m s-1";
    if ((retval = nc_put_att_text( ncid, w_varid, "units", w_units.length(), w_units.c_str() )))
        ERR(retval);
    std::string w_long_name = "velocity, w component";
    if ((retval = nc_put_att_text( ncid, w_varid, "long_name", w_long_name.length(), w_long_name.c_str() )))
        ERR(retval);
    std::string w_coords = "x y";
    if ((retval = nc_put_att_text( ncid, w_varid, "coordinates", w_coords.length(), w_coords.c_str() )))
        ERR(retval);
    
    
    // end metadata defining mode
    // some of the examples say that the first call to nc_put_var() causes nc_enddef() to be called in the background
    // better safe to just call it to be sure
    if ((retval = nc_enddef(ncid)))
        ERR(retval);
    
    
    // now ready to start filling the variables, except that the data filling requires the data
    // to be in the form of linearized 1D arrays of 3D data or 3D arrays of 3D data, already in the order that is required
    // for the output. Even if the data is in the right order and format, the netcdf output writers require grabbing all
    // the data as the full array/vector rather than grabbing the data one point at a time. The wn_3dArray datatype is a 
    // 1D linearized array (so 3D but linearized to be 1D), but the data type only allows grabbing the data one point at a time
    // from the arrays. In addition, something quirky seems to be going on with the order of i,j,k vs x,y,z
    // or in other words the data is stored as cols before rows rather than rows before cols or something. Definitely
    // not in the right order as required for the output either way.
    // also, some data requires a different organization than the full 3D stuff (need to create a few new 1D vars 
    // from the 3D input data).
    // 
    // so need to reformat the data for output before filling the variables
    // 
    // note, the wn_3dScalarField datatype is essentially the same thing as wn_3dArray datatype, returns the wn_3dArray datatype
    // but has a bunch of added function calls to do fun stuff like interpolation or gradient calculation on the wn_3dArray data
    // looks like the raw data is a 1D linearized array of doubles, so use type double for the reformatted data
    // 
    // I've figured out the i,j,k vs x,y,z quirkiness. x is nCols, y is nRows. The storage is cols before rows
    // so idx = kk*nCols*nRows + jj*nCols + ii;
    // the volVtk output is rows before cols, so idx = kk*nCols*nRows + ii*nRows + jj;
    // in the wn_3dArray storage, it uses rows as i, cols as j, layers as k, but the volVtk uses
    // i as cols, j as rows, and k as layers. I've indexed the above two index descriptions using
    // ii as cols, jj as rows, kk as layers, so consistent between both indices, looking at the raw code will 
    // make the formula look the same but it is not because of this rows vs cols ordering stuff
    // anyhow, the 1D values had to be output in terms of the storage, the volVtk seems to just need to be consistent in the ordering
    // 
    // data is ordered fine if filled in order of first lon (x) then lat (y) then layer (z)
    // this results in the netcdf functions being in order of z,y,x but the data filled in x,y,z
    // resulting in what appears to be the proper output
    
    std::vector<double> y_2D;
    std::vector<double> x_2D;
    std::vector<double> z_3D;
    std::vector<double> terrain;
    std::vector<double> u_reformatted;
    std::vector<double> v_reformatted;
    std::vector<double> w_reformatted;
    
    for(int jj=0; jj<nRows; jj++)
    {
        for(int ii=0; ii<nCols; ii++)
        {
            //int idx = kk*nCols*nRows + ii*nRows + jj;  // if it were the full vtkOutput style 3D loop, no alterations from input
            //int idx = kk*nCols*nRows + jj*nCols + ii;  // if it were the storage style index, which is what is used here, the full 3D loop version
            int idx = jj*nCols + ii;    // kk is 0, storage style index with cols before rows
            x_2D.push_back( x(idx) );
            y_2D.push_back( y(idx) );
            
            terrain.push_back( z(idx) );
        }
    }
    
    for(int kk=0; kk<nLayers; kk++)
    {
        for(int jj=0; jj<nRows; jj++)
        {
            for(int ii=0; ii<nCols; ii++)   // switch the ordering of the ii and jj for loops to affect the ordering of the output fill
            {
                // changing the order of the output doesn't seem to be changing the result in paraview, may have to revisit this later
                //int idx = kk*nCols*nRows + ii*nRows + jj; // rows before column form, volVtk output form
                int idx = kk*nCols*nRows + jj*nCols + ii; // column before rows form, storage form
                int terrain_idx = 0*nCols*nRows + jj*nCols + ii;
                z_3D.push_back( z(idx) - z(terrain_idx) );
                u_reformatted.push_back( u(idx) );
                v_reformatted.push_back( v(idx) );
                w_reformatted.push_back( w(idx) );
            }
        }
    }
    
    
    // if required, convert the x/y positions from the input projection to the desired output projection
    if ( convertToTrueLatLong == true )
    {
        // create the necessary transform function for the conversion process
        OGRSpatialReference oSourceSRS, oTargetSRS;
        OGRCoordinateTransformation *poCT;
        oSourceSRS.importFromWkt( prjString.c_str() );     // .SetWellKnownGeogCS( datum); is used for datum inputs
        //oTargetSRS.importFromWkt( prjString_latLong.c_str() );    // using prjString for both seems to cause problems, examples always have the lat/long one as a datum rather than prjString
        oTargetSRS.SetWellKnownGeogCS( "EPSG:4326" );
        //oTargetSRS.importFromEPSG( 4326 );  // alternative method
        //oTargetSRS.SetWellKnownGeogCS("WGS84");  // yet another alternative method, I'm surprised that this one works as it doesn't seem as specific as the other ones. Same result though
#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    oSourceSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    oTargetSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */
        
        // debugging, somehow the lat dropped the values a bit?
        //std::cout << "prjString = \"" << prjString << "\"" << std::endl;
        //std::cout << "prjString_latLong = \"" << prjString_latLong << "\"" << std::endl;
        
        poCT = OGRCreateCoordinateTransformation( &oSourceSRS, &oTargetSRS );
        if( poCT == NULL )
        {
	        std::cout << "!!! volNetcdf Error!!! couldn't create OGRCoordinateTransformation poCT!!!\n" << std::endl;
            exit(ERRCODE);
        }
        
        for(int idx=0; idx<nCols*nRows; idx++)
        {
            // call the transform function on each given point
            poCT->Transform( 1, &x_2D[idx], &y_2D[idx] );
        }
        
        // finished using the transform function, need to cleanup the memory
        OGRCoordinateTransformation::DestroyCT( poCT );
    }
    
    
    // now that data is in the right format, time to use it to fill the netcdf variables
    
    //if ((retval = nc_put_var_double( ncid, y_varid, &y_2D[0] )))   // &data[0] converts from std::vector<datatype> to pointer to array datatype*
    if ((retval = nc_put_var_double( ncid, y_varid, y_2D.data() )))  // .data() acts like &data[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval)
    
    //if ((retval = nc_put_var_double( ncid, x_varid, &x_2D[0] )))   // &data[0] converts from std::vector<datatype> to pointer to array datatype*
    if ((retval = nc_put_var_double( ncid, x_varid, x_2D.data() )))  // .data() acts like &data[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval)
    
    
    //if ((retval = nc_put_var_double( ncid, z_varid, &z_3D[0] )))   // &data[0] converts from std::vector<datatype> to pointer to array datatype*
    if ((retval = nc_put_var_double( ncid, z_varid, z_3D.data() )))  // .data() acts like &data[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval)
    
    
    //if ((retval = nc_put_var_double( ncid, terrain_varid, &terrain[0] )))   // &data[0] converts from std::vector<datatype> to pointer to array datatype*
    if ((retval = nc_put_var_double( ncid, terrain_varid, terrain.data() )))  // .data() acts like &data[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval)
    
    
    //if ((retval = nc_put_var_double( ncid, u_varid, &u_reformatted[0] )))   // &data[0] converts from std::vector<datatype> to pointer to array datatype*
    if ((retval = nc_put_var_double( ncid, u_varid, u_reformatted.data() )))  // .data() acts like &data[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval)
    
    //if ((retval = nc_put_var_double( ncid, v_varid, &v_reformatted[0] )))   // &data[0] converts from std::vector<datatype> to pointer to array datatype*
    if ((retval = nc_put_var_double( ncid, v_varid, v_reformatted.data() )))  // .data() acts like &data[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval)
    
    //if ((retval = nc_put_var_double( ncid, w_varid, &w_reformatted[0] )))   // &data[0] converts from std::vector<datatype> to pointer to array datatype*
    if ((retval = nc_put_var_double( ncid, w_varid, w_reformatted.data() )))  // .data() acts like &data[0] but is safe from empty vectors, only works for c++ 11 or greater though
        ERR(retval)
    
    
    // close the netcdf file, writing is complete
    if ((retval = nc_close(ncid)))
        ERR(retval);
    
    return true;
    
}


