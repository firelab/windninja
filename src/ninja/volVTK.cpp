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

volVTK::volVTK(wn_3dScalarField const& u, wn_3dScalarField const& v, wn_3dScalarField const& w, wn_3dArray& x, 
	       wn_3dArray& y, wn_3dArray& z, int ncols, int nrows, int nlayers,
	       std::string filename)
{
  writeVolVTK(u, v, w, x, y, z, ncols, nrows, nlayers, filename);
}

volVTK::~volVTK()
{

}

bool volVTK::writeVolVTK(wn_3dScalarField const& u, wn_3dScalarField const& v, wn_3dScalarField const& w, 
			 wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
			 int ncols, int nrows, int nlayers, std::string filename)
{
  FILE *fout;
  std::string surface_filename;
  surface_filename=filename;
  int pos;
  pos = surface_filename.find_last_of(".");
  surface_filename.erase(pos, surface_filename.size());
  surface_filename.append("_surf.vtk");
  
  //Write surface grid
  fout = fopen(surface_filename.c_str(), "w");
  if(fout == NULL)
	  throw std::runtime_error("VTK file cannot be opened for writing.");
  
  //Write header stuff
  fprintf(fout, "# vtk DataFile Version 3.0\n");
  fprintf(fout, "This is a ground surface written by WindNinja.  It is on a structured grid.\n");
  fprintf(fout, "ASCII\n");
  
  fprintf(fout, "DATASET STRUCTURED_GRID\n");
  fprintf(fout, "DIMENSIONS %i %i %i\n", ncols, nrows, 1);
  fprintf(fout, "POINTS %i double\n", ncols*nrows*1);
  
  for(int ii=0; ii<ncols; ii++)
  {
      for(int jj=0; jj<nrows; jj++)
	  {
		fprintf(fout, "%lf %lf %lf\n", x(1*ncols*nrows + ii*nrows + jj), 
		  y(1*ncols*nrows + ii*nrows + jj), z(1*ncols*nrows + ii*nrows + jj));
	  }
  }
  
  fclose(fout);
  
  //Write volume grid and u,v,w data
  fout = fopen(filename.c_str(), "w");
  if(fout == NULL)
	  throw std::runtime_error("VTK file cannot be opened for writing.");
  
  //Write header stuff
  fprintf(fout, "# vtk DataFile Version 3.0\n");
  fprintf(fout, "This is a 3D wind field written by WindNinja.  It is on a structured grid, with u, v, w wind components.\n");
  fprintf(fout, "ASCII\n");
  
  //Write volume grid
  fprintf(fout, "\nDATASET STRUCTURED_GRID\n");
  fprintf(fout, "DIMENSIONS %i %i %i\n", ncols, nrows, nlayers);
  
  fprintf(fout, "POINTS %i double\n", ncols*nrows*nlayers);
  for(int kk=0; kk<nlayers; kk++)
    {
      for(int ii=0; ii<ncols; ii++)
	{
	  for(int jj=0; jj<nrows; jj++)
	    {
	      fprintf(fout, "%lf %lf %lf\n", x(kk*ncols*nrows + ii*nrows + jj), 
		      y(kk*ncols*nrows + ii*nrows + jj), z(kk*ncols*nrows + ii*nrows + jj));
	    }
	}
    }
  
  //Write data
  fprintf(fout, "\nPOINT_DATA %i\n", ncols*nrows*nlayers);
  fprintf(fout, "VECTORS wind_vectors double\n");
  
  for(int kk=0; kk<nlayers; kk++)
    {
      for(int ii=0; ii<ncols; ii++)
	{
	  for(int jj=0; jj<nrows; jj++)
	    {
	      fprintf(fout, "%lf %lf %lf\n", u(kk*ncols*nrows + ii*nrows + jj), 
		      v(kk*ncols*nrows + ii*nrows + jj), w(kk*ncols*nrows + ii*nrows + jj));
	    }
	}
    }
  
  fclose(fout); 
  return true;
}

bool volVTK::writeMeshVolVTK(wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
                            int ncols, int nrows, int nlayers, std::string filename)
{
  FILE *fout;
  std::string surface_filename;
  surface_filename=filename;
  int pos;
  pos = surface_filename.find_last_of(".");
  surface_filename.erase(pos, surface_filename.size());
  surface_filename.append("_surf.vtk");
  
  //Write surface grid
  fout = fopen(surface_filename.c_str(), "w");
  if(fout == NULL)
	  throw std::runtime_error("VTK file cannot be opened for writing.");
  
  //Write header stuff
  fprintf(fout, "# vtk DataFile Version 3.0\n");
  fprintf(fout, "This is a ground surface written by WindNinja.  It is on a structured grid.\n");
  fprintf(fout, "ASCII\n");
  
  fprintf(fout, "DATASET STRUCTURED_GRID\n");
  fprintf(fout, "DIMENSIONS %i %i %i\n", ncols, nrows, 1);
  fprintf(fout, "POINTS %i double\n", ncols*nrows*1);
  
  for(int ii=0; ii<ncols; ii++)
  {
      for(int jj=0; jj<nrows; jj++)
	  {
		fprintf(fout, "%lf %lf %lf\n", x(1*ncols*nrows + ii*nrows + jj), 
		  y(1*ncols*nrows + ii*nrows + jj), z(1*ncols*nrows + ii*nrows + jj));
	  }
  }
  
  fclose(fout);
  
  //Write volume grid
  fout = fopen(filename.c_str(), "w");
  if(fout == NULL)
	  throw std::runtime_error("VTK file cannot be opened for writing.");
  
  //Write header stuff
  fprintf(fout, "# vtk DataFile Version 3.0\n");
  fprintf(fout, "This is a 3D volume mesh written by WindNinja.  It is on a structured grid.\n");
  fprintf(fout, "ASCII\n");
  
  //Write volume grid
  fprintf(fout, "\nDATASET STRUCTURED_GRID\n");
  fprintf(fout, "DIMENSIONS %i %i %i\n", ncols, nrows, nlayers);
  
  fprintf(fout, "POINTS %i double\n", ncols*nrows*nlayers);
  for(int kk=0; kk<nlayers; kk++)
    {
      for(int ii=0; ii<ncols; ii++)
	{
	  for(int jj=0; jj<nrows; jj++)
	    {
	      fprintf(fout, "%lf %lf %lf\n", x(kk*ncols*nrows + ii*nrows + jj), 
		      y(kk*ncols*nrows + ii*nrows + jj), z(kk*ncols*nrows + ii*nrows + jj));
	    }
	}
    }
  
  fclose(fout); 
  return true;
}
