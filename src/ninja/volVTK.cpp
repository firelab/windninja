/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for writing volume and surface vtk files
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

#include "volVTK.h"

volVTK::volVTK()
{

}

volVTK::volVTK(wn_3dScalarField const& u, wn_3dScalarField const& v, wn_3dScalarField const& w, 
               wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
               int i, int j, int k, std::string filename, std::string vtkWriteFormat)
{
    // determine byte order of this machine only once
    // sets value of isBigEndian for binary output
    determineEndianness();
    
    if ( vtkWriteFormat == "ascii" )
    {
        writeVolVTK(u, v, w, x, y, z, i, j, k, filename);
    } else if (vtkWriteFormat == "binary" )
    {
        writeVolVTK_binary(u, v, w, x, y, z, i, j, k, filename);
    } else
    {
        throw std::runtime_error("vtkWriteFormat must be \"ascii\" or \"binary\".");
    }
}

volVTK::~volVTK()
{

}

bool volVTK::writeVolVTK(wn_3dScalarField const& u, wn_3dScalarField const& v, wn_3dScalarField const& w, 
                         wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
                         int i, int j, int k, std::string filename)
{
    FILE *fout;
    std::string surface_filename;
    surface_filename = filename;
    int pos;
    pos = surface_filename.find_last_of(".");
    surface_filename.erase(pos, surface_filename.size());
    surface_filename.append("_surf.vtk");
    
    //Write surface grid
    fout = fopen(surface_filename.c_str(), "w");
    if( fout == NULL )
        throw std::runtime_error("VTK file cannot be opened for writing.");
    
    //Write header stuff
    fprintf(fout, "# vtk DataFile Version 3.0\n");
    fprintf(fout, "This is a ground surface written by WindNinja.  It is on a structured grid.\n");
    fprintf(fout, "ASCII\n");
    
    fprintf(fout, "DATASET STRUCTURED_GRID\n");
    fprintf(fout, "DIMENSIONS %i %i %i\n", i, j, 1);
    fprintf(fout, "POINTS %i double\n", i*j*1);
    
    for(int ii=0; ii<i; ii++)
    {
        for(int jj=0; jj<j; jj++)
        {
            fprintf(fout, "%lf %lf %lf\n", x(1*i*j + ii*j + jj), 
                    y(1*i*j + ii*j + jj), z(1*i*j + ii*j + jj));
        }
    }
    
    fclose(fout);
    
    //Write volume grid and u,v,w data
    fout = fopen(filename.c_str(), "w");
    if( fout == NULL )
        throw std::runtime_error("VTK file cannot be opened for writing.");
    
    //Write header stuff
    fprintf(fout, "# vtk DataFile Version 3.0\n");
    fprintf(fout, "This is a 3D wind field written by WindNinja.  It is on a structured grid, with u, v, w wind components.\n");
    fprintf(fout, "ASCII\n");
    
    //Write volume grid
    fprintf(fout, "\nDATASET STRUCTURED_GRID\n");
    fprintf(fout, "DIMENSIONS %i %i %i\n", i, j, k);
    
    fprintf(fout, "POINTS %i double\n", i*j*k);
    for(int kk=0; kk<k; kk++)
    {
        for(int ii=0; ii<i; ii++)
        {
            for(int jj=0; jj<j; jj++)
            {
                fprintf(fout, "%lf %lf %lf\n", x(kk*i*j + ii*j + jj), 
                        y(kk*i*j + ii*j + jj), z(kk*i*j + ii*j + jj));
            }
        }
    }
    
    //Write data
    fprintf(fout, "\nPOINT_DATA %i\n", i*j*k);
    fprintf(fout, "VECTORS wind_vectors double\n");
    
    for(int kk=0; kk<k; kk++)
    {
        for(int ii=0; ii<i; ii++)
        {
            for(int jj=0; jj<j; jj++)
            {
                fprintf(fout, "%lf %lf %lf\n", u(kk*i*j + ii*j + jj), 
                        v(kk*i*j + ii*j + jj), w(kk*i*j + ii*j + jj));
            }
        }
    }
    
    fclose(fout); 
    return true;
}

bool volVTK::writeMeshVolVTK(wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
                             int i, int j, int k, std::string filename)
{
    FILE *fout;
    std::string surface_filename;
    surface_filename = filename;
    int pos;
    pos = surface_filename.find_last_of(".");
    surface_filename.erase(pos, surface_filename.size());
    surface_filename.append("_surf.vtk");
    
    //Write surface grid
    fout = fopen(surface_filename.c_str(), "w");
    if( fout == NULL )
        throw std::runtime_error("VTK file cannot be opened for writing.");
    
    //Write header stuff
    fprintf(fout, "# vtk DataFile Version 3.0\n");
    fprintf(fout, "This is a ground surface written by WindNinja.  It is on a structured grid.\n");
    fprintf(fout, "ASCII\n");
    
    fprintf(fout, "DATASET STRUCTURED_GRID\n");
    fprintf(fout, "DIMENSIONS %i %i %i\n", i, j, 1);
    fprintf(fout, "POINTS %i double\n", i*j*1);
    
    for(int ii=0; ii<i; ii++)
    {
        for(int jj=0; jj<j; jj++)
        {
            fprintf(fout, "%lf %lf %lf\n", x(1*i*j + ii*j + jj), 
                    y(1*i*j + ii*j + jj), z(1*i*j + ii*j + jj));
        }
    }
    
    fclose(fout);
    
    //Write volume grid
    fout = fopen(filename.c_str(), "w");
    if( fout == NULL )
        throw std::runtime_error("VTK file cannot be opened for writing.");
    
    //Write header stuff
    fprintf(fout, "# vtk DataFile Version 3.0\n");
    fprintf(fout, "This is a 3D volume mesh written by WindNinja.  It is on a structured grid.\n");
    fprintf(fout, "ASCII\n");
    
    //Write volume grid
    fprintf(fout, "\nDATASET STRUCTURED_GRID\n");
    fprintf(fout, "DIMENSIONS %i %i %i\n", i, j, k);
    
    fprintf(fout, "POINTS %i double\n", i*j*k);
    for(int kk=0; kk<k; kk++)
    {
        for(int ii=0; ii<i; ii++)
        {
            for(int jj=0; jj<j; jj++)
            {
                fprintf(fout, "%lf %lf %lf\n", x(kk*i*j + ii*j + jj), 
                        y(kk*i*j + ii*j + jj), z(kk*i*j + ii*j + jj));
            }
        }
    }
    
    fclose(fout); 
    return true;
}


// found ideas for this within the code in EasyBMP.cpp and shpopen.cpp, here: https://github.com/firelab/windninja/blob/04b548e2af52cbaf2a1df930bfee466b3e2e3c4a/src/ninja/EasyBMP_DataStructures.h#L43
// and here: https://github.com/firelab/windninja/blob/04b548e2af52cbaf2a1df930bfee466b3e2e3c4a/src/ninja/shpopen.cpp#L363
// and here: https://github.com/firelab/windninja/blob/04b548e2af52cbaf2a1df930bfee466b3e2e3c4a/src/ninja/shpopen.cpp#L628
void volVTK::determineEndianness()
{
    short word = 0x0001;
    if( (*(char *)& word) != 0x01 )
    {
        isBigEndian = true;
    } else
    {
        isBigEndian = false;
    }
}

// found this here, they found it elsewhere: https://stackoverflow.com/questions/10913666/error-writing-binary-vtk-files
// later I found other examples for doing this in the code here: https://github.com/firelab/windninja/blob/04b548e2af52cbaf2a1df930bfee466b3e2e3c4a/src/ninja/EasyBMP.cpp#L52
//  and here: https://github.com/firelab/windninja/blob/04b548e2af52cbaf2a1df930bfee466b3e2e3c4a/src/ninja/shpopen.cpp#L200
// turns out to be necessary to convert the output binary format from little endian to big endian on little endian systems, 
//  because paraview only handles big endian format.
template <typename T>
void volVTK::swapEnd(T& var)
{
    char* varArray = reinterpret_cast<char*>(&var);
    for(long i = 0; i < static_cast<long>(sizeof(var)/2); i++)
        std::swap(varArray[sizeof(var) - 1 - i],varArray[i]);
}


// "legacy" VTK format always has to be written in BigEndian not in LittleEndian, paraview only accepts BigEndian format
//  this is described here: https://vtk.org/Wiki/VTK/Writing_VTK_files_using_python#.22legacy.22
bool volVTK::writeVolVTK_binary(wn_3dScalarField const& u, wn_3dScalarField const& v, wn_3dScalarField const& w, 
			                    wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
			                    int i, int j, int k, std::string filename)
{
    FILE *fout;
    std::string surface_filename;
    surface_filename = filename;
    int pos;
    pos = surface_filename.find_last_of(".");
    surface_filename.erase(pos, surface_filename.size());
    surface_filename.append("_surf.vtk");
    
    //Write surface grid
    fout = fopen(surface_filename.c_str(), "wb");
    if( fout == NULL )
        throw std::runtime_error("VTK file cannot be opened for writing.");
    
    //Write header stuff
    fprintf(fout, "# vtk DataFile Version 3.0\n");
    fprintf(fout, "This is a ground surface written by WindNinja.  It is on a structured grid.\n");
    fprintf(fout, "BINARY\n");
    
    fprintf(fout, "DATASET STRUCTURED_GRID\n");
    fprintf(fout, "DIMENSIONS %i %i %i\n", i, j, 1);
    fprintf(fout, "POINTS %i double\n", i*j*1);
    
    for(int ii=0; ii<i; ii++)
    {
        for(int jj=0; jj<j; jj++)
        {
            double x_tmp = x(1*i*j + ii*j + jj);
            double y_tmp = y(1*i*j + ii*j + jj);
            double z_tmp = z(1*i*j + ii*j + jj);
            if( isBigEndian == false )
            {
                swapEnd(x_tmp);
                swapEnd(y_tmp);
                swapEnd(z_tmp);
            }
            fwrite( &x_tmp, sizeof( double ), 1, fout );
            fwrite( &y_tmp, sizeof( double ), 1, fout );
            fwrite( &z_tmp, sizeof( double ), 1, fout );
        }
    }
    
    fclose(fout);
    
    //Write volume grid and u,v,w data
    fout = fopen(filename.c_str(), "wb");
    if( fout == NULL )
        throw std::runtime_error("VTK file cannot be opened for writing.");
    
    //Write header stuff
    fprintf(fout, "# vtk DataFile Version 3.0\n");
    fprintf(fout, "This is a 3D wind field written by WindNinja.  It is on a structured grid, with u, v, w wind components.\n");
    fprintf(fout, "BINARY\n");
    
    //Write volume grid
    fprintf(fout, "\nDATASET STRUCTURED_GRID\n");
    fprintf(fout, "DIMENSIONS %i %i %i\n", i, j, k);
    
    fprintf(fout, "POINTS %i double\n", i*j*k);
    for(int kk=0; kk<k; kk++)
    {
        for(int ii=0; ii<i; ii++)
        {
            for(int jj=0; jj<j; jj++)
            {
                double x_tmp = x(kk*i*j + ii*j + jj);
                double y_tmp = y(kk*i*j + ii*j + jj);
                double z_tmp = z(kk*i*j + ii*j + jj);
                if( isBigEndian == false )
                {
                    swapEnd(x_tmp);
                    swapEnd(y_tmp);
                    swapEnd(z_tmp);
                }
                fwrite( &x_tmp, sizeof( double ), 1, fout );
                fwrite( &y_tmp, sizeof( double ), 1, fout );
                fwrite( &z_tmp, sizeof( double ), 1, fout );
            }
        }
    }
    
    //Write data
    fprintf(fout, "\nPOINT_DATA %i\n", i*j*k);
    fprintf(fout, "VECTORS wind_vectors double\n");
    
    for(int kk=0; kk<k; kk++)
    {
        for(int ii=0; ii<i; ii++)
        {
            for(int jj=0; jj<j; jj++)
            {
                double u_tmp = u(kk*i*j + ii*j + jj);
                double v_tmp = v(kk*i*j + ii*j + jj);
                double w_tmp = w(kk*i*j + ii*j + jj);
                if( isBigEndian == false )
                {
                    swapEnd(u_tmp);
                    swapEnd(v_tmp);
                    swapEnd(w_tmp);
                }
                fwrite( &u_tmp, sizeof( double ), 1, fout );
                fwrite( &v_tmp, sizeof( double ), 1, fout );
                fwrite( &w_tmp, sizeof( double ), 1, fout );
            }
        }
    }
    
    fclose(fout); 
    return true;
}

// "legacy" VTK format always has to be written in BigEndian not in LittleEndian, paraview only accepts BigEndian format
//  this is described here: https://vtk.org/Wiki/VTK/Writing_VTK_files_using_python#.22legacy.22
bool volVTK::writeMeshVolVTK_binary(wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
                                    int i, int j, int k, std::string filename)
{
    FILE *fout;
    std::string surface_filename;
    surface_filename = filename;
    int pos;
    pos = surface_filename.find_last_of(".");
    surface_filename.erase(pos, surface_filename.size());
    surface_filename.append("_surf.vtk");
    
    //Write surface grid
    fout = fopen(surface_filename.c_str(), "wb");
    if( fout == NULL )
        throw std::runtime_error("VTK file cannot be opened for writing.");
    
    //Write header stuff
    fprintf(fout, "# vtk DataFile Version 3.0\n");
    fprintf(fout, "This is a ground surface written by WindNinja.  It is on a structured grid.\n");
    fprintf(fout, "BINARY\n");
    
    fprintf(fout, "DATASET STRUCTURED_GRID\n");
    fprintf(fout, "DIMENSIONS %i %i %i\n", i, j, 1);
    fprintf(fout, "POINTS %i double\n", i*j*1);
    
    for(int ii=0; ii<i; ii++)
    {
        for(int jj=0; jj<j; jj++)
        {
            double x_tmp = x(1*i*j + ii*j + jj);
            double y_tmp = y(1*i*j + ii*j + jj);
            double z_tmp = z(1*i*j + ii*j + jj);
            if( isBigEndian == false )
            {
                swapEnd(x_tmp);
                swapEnd(y_tmp);
                swapEnd(z_tmp);
            }
            fwrite( &x_tmp, sizeof( double ), 1, fout );
            fwrite( &y_tmp, sizeof( double ), 1, fout );
            fwrite( &z_tmp, sizeof( double ), 1, fout );
        }
    }
    
    fclose(fout);
    
    //Write volume grid
    fout = fopen(filename.c_str(), "wb");
    if( fout == NULL )
        throw std::runtime_error("VTK file cannot be opened for writing.");
    
    //Write header stuff
    fprintf(fout, "# vtk DataFile Version 3.0\n");
    fprintf(fout, "This is a 3D volume mesh written by WindNinja.  It is on a structured grid.\n");
    fprintf(fout, "BINARY\n");
    
    //Write volume grid
    fprintf(fout, "\nDATASET STRUCTURED_GRID\n");
    fprintf(fout, "DIMENSIONS %i %i %i\n", i, j, k);
    
    fprintf(fout, "POINTS %i double\n", i*j*k);
    for(int kk=0; kk<k; kk++)
    {
        for(int ii=0; ii<i; ii++)
        {
            for(int jj=0; jj<j; jj++)
            {
                double x_tmp = x(kk*i*j + ii*j + jj);
                double y_tmp = y(kk*i*j + ii*j + jj);
                double z_tmp = z(kk*i*j + ii*j + jj);
                if( isBigEndian == false )
                {
                    swapEnd(x_tmp);
                    swapEnd(y_tmp);
                    swapEnd(z_tmp);
                }
                fwrite( &x_tmp, sizeof( double ), 1, fout );
                fwrite( &y_tmp, sizeof( double ), 1, fout );
                fwrite( &z_tmp, sizeof( double ), 1, fout );
            }
        }
    }
    
    fclose(fout); 
    return true;
}


