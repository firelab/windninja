/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Friciton velocity calculations
 * Author:   Natalie Wagenbrenner <nwagenbrenner@gmail.com>
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
#include "frictionVelocity.h"


FrictionVelocity::FrictionVelocity()        //default constructor
{

}

FrictionVelocity::~FrictionVelocity()      //destructor
{

}

/**
 * @brief Calculate vertex normals for each node in the mesh
 * Vertex normals are the weighted average of each facet
 * contributing to the node; 4 facets contribute to nodes
 * on the intereior of the mesh, 2 contribute to the non-corner
 * boundaries, and 1 contributes to the corner nodes
 *
 * @param mesh WindNinja computational mesh.
 * @param input WindNinjaInputs 
 */
void FrictionVelocity::ComputeVertexNormals(Mesh const& mesh, WindNinjaInputs &input)
{
    int i, j;
    double x_node, y_node, z_node;
    double x_north, y_north, z_north, x_south, y_south, z_south;
    double x_east, y_east, z_east, x_west, y_west, z_west;
    double v1x, v1y, v1z, v2x, v2y, v2z, normal_factor, x_normal, y_normal, z_normal;
    double x_normal_ne, y_normal_ne, z_normal_ne, x_normal_se, y_normal_se, z_normal_se;
    double x_normal_sw, y_normal_sw, z_normal_sw, x_normal_nw, y_normal_nw, z_normal_nw;
    double area_ne, area_se, area_sw, area_nw;
    double weight_ne, weight_se, weight_sw, weight_nw;
    
    VertexNormalX.set_headerData(input.dem.get_nCols(),
                            input.dem.get_nRows(),
                            input.dem.get_xllCorner(), 
                            input.dem.get_yllCorner(), 
                            input.dem.get_cellSize(), 
                            -9999.0, 
                            -9999.0, 
                            input.dem.prjString);
    VertexNormalY.set_headerData(VertexNormalX);
    VertexNormalZ.set_headerData(VertexNormalX);

    for(i=0;i<mesh.nrows;i++)
    {
        for(j=0; j<mesh.ncols; j++)
        {
            if(i>0 && i<(mesh.nrows-1) && j>0 && j<(mesh.ncols-1)) //if we are not at the N, S, E, or W boundary
            {
                x_node = mesh.XORD(i, j, 0);
                y_node = mesh.YORD(i, j, 0);
                z_node = mesh.ZORD(i, j, 0);
                x_north = mesh.XORD(i+1, j, 0);
                y_north = mesh.YORD(i+1, j, 0);
                z_north = mesh.ZORD(i+1, j, 0);
                x_east = mesh.XORD(i, j+1, 0);
                y_east = mesh.YORD(i, j+1, 0);
                z_east = mesh.ZORD(i, j+1, 0);
                x_south = mesh.XORD(i-1, j, 0);
                y_south = mesh.YORD(i-1, j, 0);
                z_south = mesh.ZORD(i-1, j, 0);
                x_west = mesh.XORD(i, j-1, 0);
                y_west = mesh.YORD(i, j-1, 0);
                z_west = mesh.ZORD(i, j-1, 0);

                //--------- NE facet ------------------------------------------------------------
                v1x = x_east - x_node; //get the vectors; start with NE facet and move clockwise
                v1y = y_east - y_node;
                v1z = z_east - z_node;
                v2x = x_north - x_node;
                v2y = y_north - y_node;
                v2z = z_north - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_ne = x_normal/normal_factor; //calculate normals for NE facet
                y_normal_ne = y_normal/normal_factor;
                z_normal_ne = z_normal/normal_factor;
                area_ne = 0.5*normal_factor; //area = 0.5(cross product of the two vectors)

                //---------- SE facet -----------------------------------------------------------
                v1x = x_south - x_node; //get vectors for SE facet
                v1y = y_south - y_node;
                v1z = z_south - z_node;
                v2x = x_east - x_node;
                v2y = y_east - y_node;
                v2z = z_east - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_se = x_normal/normal_factor;//calculate normals for SE facet
                y_normal_se = y_normal/normal_factor;
                z_normal_se = z_normal/normal_factor;
                area_se = 0.5*normal_factor; //area = 0.5(cross product of the two vectors)

                //---------- SW facet -----------------------------------------------------------
                v1x = x_west - x_node; //get vectors for SW facet
                v1y = y_west - y_node;
                v1z = z_west - z_node;
                v2x = x_south - x_node;
                v2y = y_south - y_node;
                v2z = z_south - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_sw = x_normal/normal_factor; //calculate normals for SW facet
                y_normal_sw = y_normal/normal_factor;
                z_normal_sw = z_normal/normal_factor;
                area_sw = 0.5*normal_factor; //area = 0.5(cross product of the two vectors)

                //---------- NW facet -----------------------------------------------------------
                v1x = x_north - x_node; //get vectors for NW facet
                v1y = y_north - y_node;
                v1z = z_north - z_node;
                v2x = x_west - x_node;
                v2y = y_west - y_node;
                v2z = z_west - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_nw = x_normal/normal_factor; //calculate normals for NW facet
                y_normal_nw = y_normal/normal_factor;
                z_normal_nw = z_normal/normal_factor;
                area_nw = 0.5*normal_factor; //area = 0.5(cross product of the two vectors)

                //---------- average facet normals ----------------------------------------------
                weight_ne = area_ne/(area_ne+area_se+area_sw+area_nw);
                weight_se = area_se/(area_ne+area_se+area_sw+area_nw);
                weight_sw = area_sw/(area_ne+area_se+area_sw+area_nw);
                weight_nw = area_nw/(area_ne+area_se+area_sw+area_nw);
                VertexNormalX(i,j) = weight_ne*x_normal_ne + weight_se*x_normal_se + weight_sw*x_normal_sw + weight_nw*x_normal_nw;
                VertexNormalY(i,j) = weight_ne*y_normal_ne + weight_se*y_normal_se + weight_sw*y_normal_sw + weight_nw*y_normal_nw;
                VertexNormalZ(i,j) = weight_ne*z_normal_ne + weight_se*z_normal_se + weight_sw*z_normal_sw + weight_nw*z_normal_nw;
            }
            else if(i==0 && j>0 && j<(mesh.ncols-1)) //if we are at the south boundary, but not a corner
            {
                x_node = mesh.XORD(i, j, 0);
                y_node = mesh.YORD(i, j, 0);
                z_node = mesh.ZORD(i, j, 0);
                x_north = mesh.XORD(i+1, j, 0);
                y_north = mesh.YORD(i+1, j, 0);
                z_north = mesh.ZORD(i+1, j, 0);
                x_east = mesh.XORD(i, j+1, 0);
                y_east = mesh.YORD(i, j+1, 0);
                z_east = mesh.ZORD(i, j+1, 0);
                x_west = mesh.XORD(i, j-1, 0);
                y_west = mesh.YORD(i, j-1, 0);
                z_west = mesh.ZORD(i, j-1, 0);

                //--------- NE facet ------------------------------------------------------------
                v1x = x_east - x_node; //get the vectors; start with NE facet and move clockwise
                v1y = y_east - y_node;
                v1z = z_east - z_node;
                v2x = x_north - x_node;
                v2y = y_north - y_node;
                v2z = z_north - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_ne = x_normal/normal_factor; //calculate normals for NE facet
                y_normal_ne = y_normal/normal_factor;
                z_normal_ne = z_normal/normal_factor;
                area_ne = 0.5*normal_factor; //area = 0.5(cross product of the two vectors)

                //---------- NW facet -----------------------------------------------------------
                v1x = x_north - x_node; //get vectors for NW facet
                v1y = y_north - y_node;
                v1z = z_north - z_node;
                v2x = x_west - x_node;
                v2y = y_west - y_node;
                v2z = z_west - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_nw = x_normal/normal_factor; //calculate normals for NW facet
                y_normal_nw = y_normal/normal_factor;
                z_normal_nw = z_normal/normal_factor;
                area_nw = 0.5*normal_factor; //area = 0.5(cross product of the two vectors)

                //---------- average facet normals ----------------------------------------------
                weight_ne = area_ne/(area_ne+area_nw);
                weight_nw = area_nw/(area_ne+area_nw);
                VertexNormalX(i,j) = weight_ne*x_normal_ne + weight_nw*x_normal_nw;
                VertexNormalY(i,j) = weight_ne*y_normal_ne + weight_nw*y_normal_nw;
                VertexNormalZ(i,j) = weight_ne*z_normal_ne + weight_nw*z_normal_nw;
            }
            else if(i==(mesh.nrows-1) && j>0 && j<(mesh.ncols-1)) //if we are at the north boundary, but not a corner
            {
                x_node = mesh.XORD(i, j, 0);
                y_node = mesh.YORD(i, j, 0);
                z_node = mesh.ZORD(i, j, 0);
                x_east = mesh.XORD(i, j+1, 0);
                y_east = mesh.YORD(i, j+1, 0);
                z_east = mesh.ZORD(i, j+1, 0);
                x_south = mesh.XORD(i-1, j, 0);
                y_south = mesh.YORD(i-1, j, 0);
                z_south = mesh.ZORD(i-1, j, 0);
                x_west = mesh.XORD(i, j-1, 0);
                y_west = mesh.YORD(i, j-1, 0);
                z_west = mesh.ZORD(i, j-1, 0);

                 //---------- SE facet -----------------------------------------------------------
                v1x = x_south - x_node; //get vectors for SE facet
                v1y = y_south - y_node;
                v1z = z_south - z_node;
                v2x = x_east - x_node;
                v2y = y_east - y_node;
                v2z = z_east - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_se = x_normal/normal_factor;//calculate normals for SE facet
                y_normal_se = y_normal/normal_factor;
                z_normal_se = z_normal/normal_factor;
                area_se = 0.5*normal_factor; //area = 0.5(cross product of the two vectors)

                //---------- SW facet -----------------------------------------------------------
                v1x = x_west - x_node; //get vectors for SW facet
                v1y = y_west - y_node;
                v1z = z_west - z_node;
                v2x = x_south - x_node;
                v2y = y_south - y_node;
                v2z = z_south - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_sw = x_normal/normal_factor; //calculate normals for SW facet
                y_normal_sw = y_normal/normal_factor;
                z_normal_sw = z_normal/normal_factor;
                area_sw = 0.5*normal_factor; //area = 0.5(cross product of the two vectors)

                //---------- average facet normals ----------------------------------------------
                weight_se = area_se/(area_se+area_sw);
                weight_sw = area_sw/(area_se+area_sw);
                VertexNormalX(i,j) = weight_se*x_normal_se + weight_sw*x_normal_sw;
                VertexNormalY(i,j) = weight_se*y_normal_se + weight_sw*y_normal_sw;
                VertexNormalZ(i,j) = weight_se*z_normal_se + weight_sw*z_normal_sw;
            }
            else if(i>0 && i<(mesh.nrows-1) && j==0) //if we are at the west boundary, but not a corner
            {
                x_node = mesh.XORD(i, j, 0);
                y_node = mesh.YORD(i, j, 0);
                z_node = mesh.ZORD(i, j, 0);
                x_north = mesh.XORD(i+1, j, 0);
                y_north = mesh.YORD(i+1, j, 0);
                z_north = mesh.ZORD(i+1, j, 0);
                x_east = mesh.XORD(i, j+1, 0);
                y_east = mesh.YORD(i, j+1, 0);
                z_east = mesh.ZORD(i, j+1, 0);
                x_south = mesh.XORD(i-1, j, 0);
                y_south = mesh.YORD(i-1, j, 0);
                z_south = mesh.ZORD(i-1, j, 0);

                //--------- NE facet ------------------------------------------------------------
                v1x = x_east - x_node; //get the vectors; start with NE facet and move clockwise
                v1y = y_east - y_node;
                v1z = z_east - z_node;
                v2x = x_north - x_node;
                v2y = y_north - y_node;
                v2z = z_north - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_ne = x_normal/normal_factor; //calculate normals for NE facet
                y_normal_ne = y_normal/normal_factor;
                z_normal_ne = z_normal/normal_factor;
                area_ne = 0.5*normal_factor; //area = 0.5(cross product of the two vectors)

                //---------- SE facet -----------------------------------------------------------
                v1x = x_south - x_node; //get vectors for SE facet
                v1y = y_south - y_node;
                v1z = z_south - z_node;
                v2x = x_east - x_node;
                v2y = y_east - y_node;
                v2z = z_east - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_se = x_normal/normal_factor;//calculate normals for SE facet
                y_normal_se = y_normal/normal_factor;
                z_normal_se = z_normal/normal_factor;
                area_se = 0.5*normal_factor; //area = 0.5(cross product of the two vectors)

                //---------- average facet normals ----------------------------------------------
                weight_ne = area_ne/(area_ne+area_se);
                weight_se = area_se/(area_ne+area_se);
                VertexNormalX(i,j) = weight_ne*x_normal_ne + weight_se*x_normal_se;
                VertexNormalY(i,j) = weight_ne*y_normal_ne + weight_se*y_normal_se;
                VertexNormalZ(i,j) = weight_ne*z_normal_ne + weight_se*z_normal_se;

            }
            else if(i>0 && i<(mesh.nrows-1) && j==(mesh.ncols-1)) //if we are at the east boundary, but not a corner
            {
                x_node = mesh.XORD(i, j, 0);
                y_node = mesh.YORD(i, j, 0);
                z_node = mesh.ZORD(i, j, 0);
                x_north = mesh.XORD(i+1, j, 0);
                y_north = mesh.YORD(i+1, j, 0);
                z_north = mesh.ZORD(i+1, j, 0);
                x_south = mesh.XORD(i-1, j, 0);
                y_south = mesh.YORD(i-1, j, 0);
                z_south = mesh.ZORD(i-1, j, 0);
                x_west = mesh.XORD(i, j-1, 0);
                y_west = mesh.YORD(i, j-1, 0);
                z_west = mesh.ZORD(i, j-1, 0);

                //---------- SW facet -----------------------------------------------------------
                v1x = x_west - x_node; //get vectors for SW facet
                v1y = y_west - y_node;
                v1z = z_west - z_node;
                v2x = x_south - x_node;
                v2y = y_south - y_node;
                v2z = z_south - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_sw = x_normal/normal_factor; //calculate normals for SW facet
                y_normal_sw = y_normal/normal_factor;
                z_normal_sw = z_normal/normal_factor;
                area_sw = 0.5*normal_factor; //area = 0.5(cross product of the two vectors)

                //---------- NW facet -----------------------------------------------------------
                v1x = x_north - x_node; //get vectors for NW facet
                v1y = y_north - y_node;
                v1z = z_north - z_node;
                v2x = x_west - x_node;
                v2y = y_west - y_node;
                v2z = z_west - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_nw = x_normal/normal_factor; //calculate normals for NW facet
                y_normal_nw = y_normal/normal_factor;
                z_normal_nw = z_normal/normal_factor;
                area_nw = 0.5*normal_factor; //area = 0.5(cross product of the two vectors)

                //---------- average facet normals ----------------------------------------------
                weight_sw = area_sw/(area_sw+area_nw);
                weight_nw = area_nw/(area_sw+area_nw);
                VertexNormalX(i,j) = weight_sw*x_normal_sw + weight_nw*x_normal_nw;
                VertexNormalY(i,j) = weight_sw*y_normal_sw + weight_nw*y_normal_nw;
                VertexNormalZ(i,j) = weight_sw*z_normal_sw + weight_nw*z_normal_nw;
            }
            else if(i==0 && j==0) //if we are at the SW corner
            {
                x_node = mesh.XORD(i, j, 0);
                y_node = mesh.YORD(i, j, 0);
                z_node = mesh.ZORD(i, j, 0);
                x_north = mesh.XORD(i+1, j, 0);
                y_north = mesh.YORD(i+1, j, 0);
                z_north = mesh.ZORD(i+1, j, 0);
                x_east = mesh.XORD(i, j+1, 0);
                y_east = mesh.YORD(i, j+1, 0);
                z_east = mesh.ZORD(i, j+1, 0);

                //--------- NE facet ------------------------------------------------------------
                v1x = x_east - x_node; //get the vectors; start with NE facet and move clockwise
                v1y = y_east - y_node;
                v1z = z_east - z_node;
                v2x = x_north - x_node;
                v2y = y_north - y_node;
                v2z = z_north - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_ne = x_normal/normal_factor; //calculate normals for NE facet
                y_normal_ne = y_normal/normal_factor;
                z_normal_ne = z_normal/normal_factor;
                VertexNormalX(i,j) = x_normal_ne;
                VertexNormalY(i,j) = y_normal_ne;
                VertexNormalZ(i,j) = z_normal_ne;
            }
            else if(i==0 && j==(mesh.ncols-1)) //if we are at the SE corner
            {
                x_node = mesh.XORD(i, j, 0);
                y_node = mesh.YORD(i, j, 0);
                z_node = mesh.ZORD(i, j, 0);
                x_north = mesh.XORD(i+1, j, 0);
                y_north = mesh.YORD(i+1, j, 0);
                z_north = mesh.ZORD(i+1, j, 0);
                x_west = mesh.XORD(i, j-1, 0);
                y_west = mesh.YORD(i, j-1, 0);
                z_west = mesh.ZORD(i, j-1, 0);

                //---------- NW facet -----------------------------------------------------------
                v1x = x_north - x_node; //get vectors for NW facet
                v1y = y_north - y_node;
                v1z = z_north - z_node;
                v2x = x_west - x_node;
                v2y = y_west - y_node;
                v2z = z_west - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_nw = x_normal/normal_factor; //calculate normals for NW facet
                y_normal_nw = y_normal/normal_factor;
                z_normal_nw = z_normal/normal_factor;
                VertexNormalX(i,j) = x_normal_nw;
                VertexNormalY(i,j) = y_normal_nw;
                VertexNormalZ(i,j) = z_normal_nw;
            }
            else if(i==(mesh.nrows-1) && j==0) //if we are at the NW corner
            {
                x_node = mesh.XORD(i, j, 0);
                y_node = mesh.YORD(i, j, 0);
                z_node = mesh.ZORD(i, j, 0);
                x_east = mesh.XORD(i, j+1, 0);
                y_east = mesh.YORD(i, j+1, 0);
                z_east = mesh.ZORD(i, j+1, 0);
                x_south = mesh.XORD(i-1, j, 0);
                y_south = mesh.YORD(i-1, j, 0);
                z_south = mesh.ZORD(i-1, j, 0);

                //---------- SE facet -----------------------------------------------------------
                v1x = x_south - x_node; //get vectors for SE facet
                v1y = y_south - y_node;
                v1z = z_south - z_node;
                v2x = x_east - x_node;
                v2y = y_east - y_node;
                v2z = z_east - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_se = x_normal/normal_factor;//calculate normals for SE facet
                y_normal_se = y_normal/normal_factor;
                z_normal_se = z_normal/normal_factor;
                VertexNormalX(i,j) = x_normal_se;
                VertexNormalY(i,j) = y_normal_se;
                VertexNormalZ(i,j) = z_normal_se;
            }
            else if(i==(mesh.nrows-1) && j==(mesh.ncols-1)) //if we are at the NE corner
            {
                x_node = mesh.XORD(i, j, 0);
                y_node = mesh.YORD(i, j, 0);
                z_node = mesh.ZORD(i, j, 0);
                x_south = mesh.XORD(i-1, j, 0);
                y_south = mesh.YORD(i-1, j, 0);
                z_south = mesh.ZORD(i-1, j, 0);
                x_west = mesh.XORD(i, j-1, 0);
                y_west = mesh.YORD(i, j-1, 0);
                z_west = mesh.ZORD(i, j-1, 0);

                //---------- SW facet -----------------------------------------------------------
                v1x = x_west - x_node; //get vectors for SW facet
                v1y = y_west - y_node;
                v1z = z_west - z_node;
                v2x = x_south - x_node;
                v2y = y_south - y_node;
                v2z = z_south - z_node;
                x_normal = (v1y*v2z) - (v1z*v2y); //calculate the cross product (move CC from v1 to v2) and normalize
                y_normal = -((v2z*v1x) - (v2x*v1z));
                z_normal = (v1x*v2y) - (v1y*v2x);
                normal_factor = std::sqrt((x_normal*x_normal) + (y_normal*y_normal) + (z_normal*z_normal));
                x_normal_sw = x_normal/normal_factor; //calculate normals for SW facet
                y_normal_sw = y_normal/normal_factor;
                z_normal_sw = z_normal/normal_factor;
                VertexNormalX(i,j) = x_normal_sw;
                VertexNormalY(i,j) = y_normal_sw;
                VertexNormalZ(i,j) = z_normal_sw;
            }
            else
            {
                throw std::out_of_range("Range error in frictionVelocity::GetVertexNormals()");
            }
        }
    }
    //VertexNormalX = grid_x;
    //VertexNormalY = grid_y;
    //VertexNormalZ = grid_z;
}

/**
 * @brief Calculate friction velocity at each ground node
 * Calculate the gradient vectors at each node. Store gradient vectors for
 * one vertical layer in 9 AsciiGrids. Transform the gradient vectors so that
 * the z-axis is parallel to the vertex normal vector. Calculate the
 * velocity gradient (dU/dz) at each node from du'/dz' and dv'/dz' where (')
 * denotes the transformed coordinates. Friction velocity is calculated as
 * ustar = sqrt(tau0/rho) where tau0 is the shear stress at the ground:
 * tau0 = (kappa^2)(z^2)(dU/dz). kappa = 0.4, z = distance to zero-plane
 * displacement height.
 * Units for ustar are m/s.
 * @param Reference to inputs
 * @param Reference to the ustar gradient grid to be populated
 * @param References to the u, v, and w arrays
 * @param Reference to the mesh
 * @param calcMethod to indicate which method should be used; default is logProfile
 */
void FrictionVelocity::ComputeUstar(WindNinjaInputs &input,
                                   AsciiGrid<double> &grid,
                                   wn_3dScalarField &u,
                                   wn_3dScalarField &v,
                                   wn_3dScalarField &w,
                                   const Mesh &mesh,
                                   std::string calcMethod)
{
    wn_3dVectorField uDerivatives, vDerivatives, wDerivatives;

    u.ComputeGradient(input, uDerivatives);  //calculates du/dx, du/dy, du/dz
    v.ComputeGradient(input, vDerivatives);  //calculates dv/dx, dv/dy, dv/dz
    w.ComputeGradient(input, wDerivatives);  //calculates dw/dx, dw/dy, dw/dz

    //cout << "### uDeriv.vectorData_z(0,0,0) = " << (*uDerivatives.vectorData_z)(0,0,0) << endl;
    //cout << "### uDeriv.vectorData_z = " << (uDerivatives.vectorData_z) << endl;

    AsciiGrid<double> dudxGrid(grid);    //grids to store vector gradients for 1 vertical layer
    AsciiGrid<double> dudyGrid(grid);
    AsciiGrid<double> dudzGrid(grid);
    AsciiGrid<double> dvdxGrid(grid);
    AsciiGrid<double> dvdyGrid(grid);
    AsciiGrid<double> dvdzGrid(grid);
    AsciiGrid<double> dwdxGrid(grid);
    AsciiGrid<double> dwdyGrid(grid);
    AsciiGrid<double> dwdzGrid(grid);

    int i, j, k, layer;
    for(i=0;i<grid.get_nRows();i++)
    {
        for(j=0;j<grid.get_nCols();j++)
        {
            for(layer=0;layer<mesh.nlayers;layer++)
            {
                if(input.surface.Rough_h(i,j) < (mesh.ZORD(i,j,layer+1)-mesh.ZORD(i,j,0)))
                {
                    k=layer+2; //I think this should be layer+2
                    break;
                }
            }
            if(k<1)
            {
                throw std::out_of_range("k cannot be < 1 in frictionVelocity::ComputeUstar()");
            }

            dudxGrid(i,j) = (*uDerivatives.vectorData_x)(i,j,k);
            dudyGrid(i,j) = (*uDerivatives.vectorData_y)(i,j,k);
            dudzGrid(i,j) = (*uDerivatives.vectorData_z)(i,j,k);
            dvdxGrid(i,j) = (*vDerivatives.vectorData_x)(i,j,k);
            dvdyGrid(i,j) = (*vDerivatives.vectorData_y)(i,j,k);
            dvdzGrid(i,j) = (*vDerivatives.vectorData_z)(i,j,k);
            dwdxGrid(i,j) = (*wDerivatives.vectorData_x)(i,j,k);
            dwdyGrid(i,j) = (*wDerivatives.vectorData_y)(i,j,k);
            dwdzGrid(i,j) = (*wDerivatives.vectorData_z)(i,j,k);
        }
    }

    /*--------------------------------------------------------------------*
     *    Now transform the gradient vectors at each ground node so       *
     *    that the z-axis is parallel to the vertex normal vector (i.e.,  *
     *    rotate dudxGrid, dudyGrid, etc, about the z-axis). The          *
     *    transformation is done based on: S' = LSTL where S' is the      *
     *    transformed 2nd order tensor for stress (i.e., the 3x3 matrix   *
     *    of gradient vectors), L is the 2nd order unit vector tensor,    *
     *    and TL is the transpose of L. The rotated coordinates are       *
     *    denoted x', y', z'.                                             *
     *--------------------------------------------------------------------*/

    /*make L; we only have the z'-axis, so need to find some x' and y' axes
      normal to the z'-axis (dot product between normal vectors = 0):
      z'-axis: i=VertexNormalX, j=VertexNormalY, k=VertexNormalZ
      x'-axis: set i=1, j=0, solve for k so that z DOT x = 0
      y'-axis: set i=0, j=1, solve for k so that z DOT y = 0
      The x' and y' axes here are chosen arbitrarily; if we want to use gradients
      along a direction other than normal to the ground, these will need to be
      changed accordingly */

    AsciiGrid<double> Xprime_i(grid);    //make grids to store i, j, k components for x'and y'
    AsciiGrid<double> Xprime_j(grid);
    AsciiGrid<double> Xprime_k(grid);
    AsciiGrid<double> Yprime_i(grid);
    AsciiGrid<double> Yprime_j(grid);
    AsciiGrid<double> Yprime_k(grid);
    Xprime_i = 1;
    Xprime_j = 0;
    Xprime_k = 0;
    Yprime_i = 0;
    Yprime_j = 1;
    Yprime_k = 0;

    for(i=0;i<grid.get_nRows();i++)    //set z' DOT x' = 0 and z' DOT y' = 0 and rearrange to solve for k
    {
        for(j=0;j<grid.get_nCols();j++)
        {
                Xprime_k(i,j) = (-VertexNormalX(i,j)*Xprime_i(i,j)-VertexNormalY(i,j)*Xprime_j(i,j))/VertexNormalZ(i,j);
                Yprime_k(i,j) = (-VertexNormalX(i,j)*Yprime_i(i,j)-VertexNormalY(i,j)*Yprime_j(i,j))/VertexNormalZ(i,j);
        }
    }//now we have all we need for L (3x3 matrix whose memebers are i, j, k coefficients of the x',y',z' unit vectors)

    L = new double[9];  //arrays to store L,TL,S at each node
    TL = new double[9];
    S = new double[9];
    SxTL = new double[9];
    LxSxTL = new double[9];
    AsciiGrid<double> dudzPrime(grid);
    AsciiGrid<double> dvdzPrime(grid);
    AsciiGrid<double> dUdz(grid);
    const double kappa_ = 0.4;   //von Karman constant
    AsciiGrid<double> z_(grid);  //distance from dU/dz to zero-plane displacement height
    double P1x, P1y, P1z, P0x, P0y, P0z, deltaPx, deltaPy, deltaPz;

    //============for alternate ustar calculation based on log profile===================================================
    S2 = new double[3];    //use u and v from rotated coords: S' = LSTL
    S2xTL = new double[3];
    LxS2xTL = new double[3];
    AsciiGrid<double> uPrime(grid); //grids to store u,v,w in rotated coords
    AsciiGrid<double> vPrime(grid);
    AsciiGrid<double> wPrime(grid);
    //====================================================================================================================

    for (i=0;i<grid.get_nRows();i++)    //make L, TL, S, calculate S', and send du'/dz' and dv'/dz' to ascii grids
    {
        for(j=0;j<grid.get_nCols();j++)
        {
            L[0]=Xprime_i(i,j);             //       | x'i  x'j  x'k |
            L[1]=Xprime_j(i,j);             //       |               |
            L[2]=Xprime_k(i,j);             //  L =  | y'i  y'j  y'k |
            L[3]=Yprime_i(i,j);             //       |               |
            L[4]=Yprime_j(i,j);             //       | z'i  z'j  z'k |
            L[5]=Yprime_k(i,j);
            L[6]=VertexNormalX(i,j);
            L[7]=VertexNormalY(i,j);
            L[8]=VertexNormalZ(i,j);
            TL[0]=L[0];                      //       | x'i  y'i  z'i |
            TL[1]=L[3];                      //       |               |
            TL[2]=L[6];                      //  TL = | x'j  y'j  z'j |
            TL[3]=L[1];                      //       |               |
            TL[4]=L[4];                      //       | x'k  y'k  z'k |
            TL[5]=L[7];
            TL[6]=L[2];
            TL[7]=L[5];
            TL[8]=L[8];
            S[0]=dudxGrid(i,j);              //      | du/dx  du/dy  du/dz |
            S[1]=dudyGrid(i,j);              //      |                     |
            S[2]=dudzGrid(i,j);              //  S = | dv/dx  dv/dy  dv/dz |
            S[3]=dvdxGrid(i,j);              //      |                     |
            S[4]=dvdyGrid(i,j);              //      | dw/dx  dw/dy  dw/dz |
            S[5]=dvdzGrid(i,j);
            S[6]=dwdxGrid(i,j);
            S[7]=dwdyGrid(i,j);
            S[8]=dwdzGrid(i,j);
            SxTL[0]=S[0]*TL[0]+S[1]*TL[3]+S[2]*TL[6];    //S x TL
            SxTL[1]=S[0]*TL[1]+S[1]*TL[4]+S[2]*TL[7];
            SxTL[2]=S[0]*TL[2]+S[1]*TL[5]+S[2]*TL[8];
            SxTL[3]=S[3]*TL[0]+S[4]*TL[3]+S[5]*TL[6];
            SxTL[4]=S[3]*TL[1]+S[4]*TL[4]+S[5]*TL[7];
            SxTL[5]=S[3]*TL[2]+S[4]*TL[5]+S[5]*TL[8];
            SxTL[6]=S[6]*TL[0]+S[7]*TL[3]+S[8]*TL[6];
            SxTL[7]=S[6]*TL[1]+S[7]*TL[4]+S[8]*TL[7];
            SxTL[8]=S[6]*TL[2]+S[7]*TL[5]+S[8]*TL[8];
            LxSxTL[0]=L[0]*SxTL[0]+L[1]*SxTL[3]+L[2]*SxTL[6];    //S' = L x S x TL
            LxSxTL[1]=L[0]*SxTL[1]+L[1]*SxTL[4]+L[2]*SxTL[7];
            LxSxTL[2]=L[0]*SxTL[2]+L[1]*SxTL[5]+L[2]*SxTL[8];
            LxSxTL[3]=L[3]*SxTL[0]+L[4]*SxTL[3]+L[5]*SxTL[6];
            LxSxTL[4]=L[3]*SxTL[1]+L[4]*SxTL[4]+L[5]*SxTL[7];
            LxSxTL[5]=L[3]*SxTL[2]+L[4]*SxTL[5]+L[5]*SxTL[8];
            LxSxTL[6]=L[6]*SxTL[0]+L[7]*SxTL[3]+L[8]*SxTL[6];
            LxSxTL[7]=L[6]*SxTL[1]+L[7]*SxTL[4]+L[8]*SxTL[7];
            LxSxTL[8]=L[6]*SxTL[2]+L[7]*SxTL[5]+L[8]*SxTL[8];
            dudzPrime(i,j)=LxSxTL[2];    //store du'/dz' and dv'/dz'
            dvdzPrime(i,j)=LxSxTL[5];
            dUdz(i,j)=std::sqrt(dudzPrime(i,j)*dudzPrime(i,j)+dvdzPrime(i,j)*dvdzPrime(i,j)); //final dU/dz
            //----- get distance to zero-plane displacement height (normal to the ground) -------------------------------
            P1x=mesh.XORD(i,j,k);  //D = N DOT (P1-P0), where N is unit normal, P1 is dU/dz point, P0 is zd point
            P1y=mesh.YORD(i,j,k);
            P1z=mesh.ZORD(i,j,k);
            P0x=mesh.XORD(i,j,0);
            P0y=mesh.YORD(i,j,0);
            P0z=mesh.ZORD(i,j,0)+input.surface.Rough_d(i,j);
            deltaPx=P1x-P0x;
            deltaPy=P1y-P0y;
            deltaPz=P1z-P0z;
            z_(i,j)=VertexNormalX(i,j)*deltaPx+VertexNormalY(i,j)*deltaPy+VertexNormalZ(i,j)*deltaPz; //distance to zd
            if(calcMethod == "shearStress")
            {
                grid(i,j)=std::sqrt(kappa_*kappa_*z_(i,j)*z_(i,j)*dUdz(i,j)*dUdz(i,j));  //calculate ustar
            }


            //=======================================================================================================================
            //-------check to see if ustar problem is with vertex normals: do ustar calc with dU/dz from regular coords ------------
            //dUdz(i,j)=std::sqrt(dudzGrid(i,j)*dudzGrid(i,j)+dvdzGrid(i,j)*dvdzGrid(i,j));
            //z_(i,j)=mesh.ZORD(i,j,k)-mesh.ZORD(i,j,0)+input.surface.Rough_d(i,j);
            //grid(i,j)=std::sqrt(kappa_*kappa_*z_(i,j)*z_(i,j)*dUdz(i,j)*dUdz(i,j));

            //=======================================================================================================================
            //-------alternate calculation for ustar based on log profile: u(z) = ustar/kappa(ln((z-d)/z0))--------------------------
            if (calcMethod == "logProfile")
            {
                for(layer=0;layer<mesh.nlayers;layer++) //find k based on height of vegetation
                {
                    if(input.surface.Rough_h(i,j) < (mesh.ZORD(i,j,layer+1)-mesh.ZORD(i,j,0)))
                    {
                        k=layer+4; //we want layer 3 or 4 above ground/vegetation
                        break;
                    }
                }
                if(k<1)
                {
                    throw std::out_of_range("k cannot be < 1 in frictionVelocity::ComputeUstar()");
                }
                S2[0]=u(i,j,k);
                S2[1]=v(i,j,k);     //  S2 = | u  v  w |
                S2[2]=w(i,j,k);
                S2xTL[0]=S2[0]*TL[0]+S2[1]*TL[3]+S2[2]*TL[6];    //S2 x TL
                S2xTL[1]=S2[0]*TL[1]+S2[1]*TL[4]+S2[2]*TL[7];
                S2xTL[2]=S2[0]*TL[2]+S2[1]*TL[5]+S2[2]*TL[8];
                LxS2xTL[0]=L[0]*S2xTL[0]+L[3]*S2xTL[1]+L[6]*S2xTL[2];    //S' = L x S2 x TL
                LxS2xTL[1]=L[1]*S2xTL[0]+L[4]*S2xTL[1]+L[7]*S2xTL[2];
                LxS2xTL[2]=L[2]*S2xTL[0]+L[5]*S2xTL[1]+L[8]*S2xTL[2];
                
                uPrime(i,j)=LxS2xTL[0];
                vPrime(i,j)=LxS2xTL[1];
                wPrime(i,j)=LxS2xTL[2];

                grid(i,j)=std::sqrt(uPrime(i,j)*uPrime(i,j)+vPrime(i,j)*vPrime(i,j))*kappa_/log((z_(i,j)-input.surface.Rough_d(i,j))/
                        input.surface.Roughness(i,j));  //calculate ustar in rotated coords

                //z_(i,j)=mesh.ZORD(i,j,k)-mesh.ZORD(i,j,0);
                //grid(i,j)=std::sqrt(u(i,j,k)*u(i,j,k)+v(i,j,k)*v(i,j,k))*kappa_/log((z_(i,j)-input.surface.Rough_d(i,j))/
                            //input.surface.Roughness(i,j));  //calculate ustar in regular coords
            }
        }
    }

    //======================================================================
    //------ for looking at vertical profiles of wind speed ----------------
    /*double spd;
    double deltaz;
    ofstream out_file;
    out_file.open("wind_profile.txt");
    for (i=0;i<grid.get_nRows();i++)
    {
        for(j=0;j<grid.get_nCols();j++)
        {
            if(i==83 && j>42 && j<91)
            {
                out_file <<"cell: " << i << "," << j << endl;
                for(k=0; k<mesh.nlayers; k++)
                {
                    spd=std::sqrt(u(i,j,k)*u(i,j,k)+v(i,j,k)*v(i,j,k));
                    deltaz=mesh.ZORD(i,j,k)-mesh.ZORD(i,j,0);
                    out_file << "layer: "<< k <<" z " << deltaz << " " << "spd " << spd <<endl;
                }
            }
        }
    }*/

    //cout << "\n-------(83,55) ---------" << endl;
    //cout <<"u,v,w = " << uPrime[50][25] << ", " << vPrime[50][25] << ", " << wPrime[50][25] <<endl;
    //cout <<"dU/dz = " << dUdz[83][53] <<endl;
    //cout <<"z_ = " << z_[83][53] <<endl;
    //cout <<"vertex normal x = " << VertexNormalX[83][53] << ", " << VertexNormalY[83][53] << ", " << VertexNormalZ[83][53] <<endl;

    //cout << "\n-------(55,29) ---------" << endl;
    //cout <<"ustar = " << grid[55][29] <<endl;
    //cout <<"dU/dz = " << dUdz[84][54] <<endl;
    //cout <<"z_ = " << z_[84][54] <<endl;
    //cout <<"vertex normal x = " << VertexNormalX[55][29] << ", " << VertexNormalY[55][29] << ", " << VertexNormalZ[55][29] <<endl;

    //--------check for vertical log profile in the steep terrain-----------
    //cout <<"u component at from 0-6:"  << u(85,59,0) << ", " << u(85,59,1) << ", " << u(85,59,2) << ", " << u(85,59,3) << ", " << u(85,59,4) << ", " << u(85,59,5) << ", " << u(85,59,6) << ", " << endl;
    //cout <<"v component at from 0-6:" << v(85,59,0) << ", " << v(85,59,1) << ", " << v(85,59,2) << ", " << v(85,59,3) << ", " << v(85,59,4) << ", " << v(85,59,5) << ", " << v(85,59,6) << ", " << endl;
    //cout <<"z = " << mesh.ZORD(85,59,1)-mesh.ZORD(85,59,0) << endl;
    //cout <<"z = " << mesh.ZORD(85,59,2)-mesh.ZORD(85,59,0) << endl;
    //cout <<"z = " << mesh.ZORD(85,59,3)-mesh.ZORD(85,59,0) << endl;
    //cout <<"z = " << mesh.ZORD(85,59,4)-mesh.ZORD(85,59,0) << endl;
    //cout <<"z = " << mesh.ZORD(85,59,5)-mesh.ZORD(85,59,0) << endl;
    //cout <<"z = " << mesh.ZORD(85,59,6)-mesh.ZORD(85,59,0) << endl;


    delete[] L;
    L=NULL;

    delete[] TL;
    TL=NULL;

    delete[] S;
    S=NULL;

    delete[] SxTL;
    SxTL=NULL;

    delete[] LxSxTL;
    LxSxTL=NULL;

    delete[] S2;
    S2=NULL;

    delete[] S2xTL;
    S2xTL=NULL;

    delete[] LxS2xTL;
    LxS2xTL=NULL;
}

