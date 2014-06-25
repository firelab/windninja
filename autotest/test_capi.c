/*
 * =====================================================================================
 *
 *       Filename:  test_capi.c
 *
 *    Description:  Tests for the C API functionality
 *
 *
 *         Author:  Levi Malott (), lmnn3@mst.edu
 *   Organization:  Wind Ninja
 *
 * =====================================================================================
 */

#include "windninja.h"
#include <assert.h>
#include <stdio.h>

//For some reason these directives are not working
// and the code will not compile unless netCDF_lock
// is explicitely created
//#ifdef _OPENMP
//
//#ifndef NETCDF_LOCK_SET
//#define NETCDF_LOCK_SET
//
//#include "omp.h"
//omp_lock_t netCDF_lock;
//
//#endif  //NETCDF_LOCK_SET
//#endif //_OPENMP

#include "omp.h"

int errval = 0;
NinjaH * ninja = NULL;

void checkInitializationMethods()
{
    const char * init_method;
    //check domainAverageInitialization
    errval = NinjaSetInitializationMethod( ninja, 0, "domain" );
    assert( errval == NINJA_SUCCESS );

    init_method = NinjaGetInitializationMethod( ninja, 0 );
    assert( 0 == strcmp( init_method, "domainAverageInitializationFlag" ) );

    //check pointInitialization
    errval = NinjaSetInitializationMethod( ninja, 0, "point" );
    assert( errval == NINJA_SUCCESS );
    
    init_method = NinjaGetInitializationMethod( ninja, 0 );
    assert( 0 == strcmp( init_method, "pointInitializationFlag" ) );

    //check wxModelInitialization
    errval = NinjaSetInitializationMethod( ninja, 0, "wxmodel" );
    assert( errval == NINJA_SUCCESS );

    init_method = NinjaGetInitializationMethod( ninja, 0 );
    assert( 0 == strcmp( init_method, "wxModelInitializationFlag" ) );

    //check passing NULL as ninja
    errval = NinjaSetInitializationMethod( NULL, 0, "domain" );
    assert( errval == NINJA_E_NULL_PTR );

    init_method = NinjaGetInitializationMethod( NULL, 0 );
    assert( NULL == init_method );
}

void checkInputMethods()
{
    //check input speed with kilometers per hour
    errval = NinjaSetInputSpeed( ninja, 0, 50, "kph" );
    assert( errval == NINJA_SUCCESS );

    //check input speed with miles per hour
    errval = NinjaSetInputSpeed( ninja, 0, 50, "mph" );
    assert( errval == NINJA_SUCCESS );

    //check input speed with meters per second
    errval = NinjaSetInputSpeed( ninja, 0, 10, "mps" );
    assert( errval == NINJA_SUCCESS );

    //check invalid input
    errval = NinjaSetInputSpeed( ninja, 0, 10, "m" );
    assert( errval == NINJA_E_INVALID );

    //check with NULL ninja
    errval = NinjaSetInputSpeed( NULL, 0, 10, "kph" );
    assert( errval == NINJA_E_NULL_PTR );

    //check valid input direction
    errval = NinjaSetInputDirection( ninja, 0, 1.0 );
    assert( errval == NINJA_SUCCESS );

    //verify NULL input returns correct error
    errval = NinjaSetInputDirection( NULL, 0, 1.0 );
    assert( errval == NINJA_E_NULL_PTR );

    //check input wind height with correct units
    errval = NinjaSetInputWindHeight( ninja, 0, 1.0, "ft" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetInputWindHeight( ninja, 0, 1.0, "m" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetInputWindHeight( ninja, 0, 1.0, "mi" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetInputWindHeight( ninja, 0, 1.0, "km" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetInputWindHeight( ninja, 0, 1.0, "ftx10" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetInputWindHeight( ninja, 0, 1.0, "mx10" );
    assert( errval == NINJA_SUCCESS );

    //check with NULL ninja
    errval = NinjaSetInputWindHeight( NULL, 0, 1.0, "km" );
    assert( errval == NINJA_E_NULL_PTR );

    //check invalid height units
    errval = NinjaSetInputWindHeight( ninja, 0, 1.0, "x" );
    assert( errval == NINJA_E_INVALID );
    
}

void checkOutputMethods()
{
    //check input wind height with correct units
    errval = NinjaSetOutputWindHeight( ninja, 0, 1.0, "ft" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetOutputWindHeight( ninja, 0, 1.0, "m" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetOutputWindHeight( ninja, 0, 1.0, "mi" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetOutputWindHeight( ninja, 0, 1.0, "km" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetOutputWindHeight( ninja, 0, 1.0, "ftx10" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetOutputWindHeight( ninja, 0, 1.0, "mx10" );
    assert( errval == NINJA_SUCCESS );

    //check with NULL ninja
    errval = NinjaSetOutputWindHeight( NULL, 0, 1.0, "km" );
    assert( errval == NINJA_E_NULL_PTR );

    //check invalid height units
    errval = NinjaSetOutputWindHeight( ninja, 0, 1.0, "x" );
    assert( errval == NINJA_E_INVALID );

    //check speed units
    errval = NinjaSetOutputSpeedUnits( ninja, 0, "mps" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetOutputSpeedUnits( ninja, 0, "mph" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetOutputSpeedUnits( ninja, 0, "kph" );
    assert( errval == NINJA_SUCCESS );

    //check invalid speed units
    errval = NinjaSetOutputSpeedUnits( ninja, 0, "x" );
    assert( errval == NINJA_E_INVALID );

    //check with NULL ninja
    errval = NinjaSetOutputSpeedUnits( NULL, 0, "mph" );
    assert( errval == NINJA_E_NULL_PTR );
}

void checkEnvironmentMethods()
{
    /*  Check SetDiurnalWinds  */
    int flag = FALSE;

    errval = NinjaSetDiurnalWinds( ninja, 0, TRUE );
    assert( errval == NINJA_SUCCESS );

    flag = NinjaGetDiurnalWindFlag( ninja, 0 );
    assert( flag );

    errval = NinjaSetDiurnalWinds( ninja, 0, FALSE );
    assert( errval == NINJA_SUCCESS );

    flag = NinjaGetDiurnalWindFlag( ninja, 0 );
    assert( !flag );

    //check with NULL ninja
    errval = NinjaSetDiurnalWinds( NULL, 0, TRUE );
    assert( errval == NINJA_E_NULL_PTR );

    /* Check SetUniAirTemp */
    errval = NinjaSetUniAirTemp( ninja, 0, 30, "K" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetUniAirTemp( ninja, 0, 30, "C" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetUniAirTemp( ninja, 0, 30, "R" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetUniAirTemp( ninja, 0, 30, "F" );
    assert( errval == NINJA_SUCCESS );

    //check with invalid units
    errval = NinjaSetUniAirTemp( ninja, 0, 30, "x" );
    assert( errval == NINJA_E_INVALID );

    //check with NULL ninja
    errval = NinjaSetUniAirTemp( NULL, 0, 30, "F" );
    assert( errval == NINJA_E_NULL_PTR ); 

    /*  check SetUniCloudCover  */
    errval = NinjaSetUniCloudCover( ninja, 0, 0.4, "fraction" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetUniCloudCover( ninja, 0, 30, "percent" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetUniCloudCover( ninja, 0, 1.0, "canopy_category" );
    assert( errval == NINJA_SUCCESS );

    //check with invalid cloud cover units
    errval = NinjaSetUniCloudCover( ninja, 0, 1, "x" );
    assert( errval == NINJA_E_INVALID );

    //check with NULL ninja

    /*  check setUniVegetation  */
    errval = NinjaSetUniVegetation( ninja, 0, "grass" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetUniVegetation( ninja, 0, "brush" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetUniVegetation( ninja, 0, "trees" );
    assert( errval == NINJA_SUCCESS );

    //check with null ninja
    errval = NinjaSetUniVegetation( NULL , 0, "brush" );
    assert( errval == NINJA_E_NULL_PTR );

    //check with invalid vegetation
    errval = NinjaSetUniVegetation( ninja, 0, "x" );
    assert( errval == NINJA_E_INVALID );
}

void checkMeshResolution()
{
    //check valid inputs
    errval = NinjaSetMeshResolutionChoice( ninja, 0, "coarse" ); 
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetMeshResolutionChoice( ninja, 0, "medium" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetMeshResolutionChoice( ninja, 0, "fine" );
    assert( errval == NINJA_SUCCESS );

    //check with NULL ninja
    errval = NinjaSetMeshResolutionChoice( NULL, 0, "coarse" );
    assert( errval == NINJA_E_NULL_PTR );

    //check with bad mesh choice
    errval = NinjaSetMeshResolutionChoice( ninja, 0, "hello" );
    assert( errval == NINJA_E_INVALID );
}

void checkEmissions()
{
#ifdef EMISSIONS
#endif //EMISSIONS
}

void checkStability()
{
#ifdef STABILITY

#endif //STABILITY
}

void checkOutputWritingMethods()
{
    //check output buffer clipping with valid input
    errval = NinjaSetOutputBufferClipping( ninja, 0, 0.10 );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetOutputBufferClipping( NULL, 0, 0.10 );
    assert( errval  == NINJA_E_NULL_PTR );

    //setWxModelGoogOutFlag checks
    errval = NinjaSetWxModelGoogOutFlag( ninja, 0, TRUE );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetWxModelGoogOutFlag( NULL, 0, TRUE );
    assert( errval == NINJA_E_NULL_PTR );

    errval = NinjaSetWxModelGoogOutFlag( ninja, 0, FALSE );
    assert( errval == NINJA_SUCCESS );

    //SHP output methods
    errval = NinjaSetWxModelShpOutFlag( ninja, 0, TRUE );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetWxModelShpOutFlag( NULL, 0, TRUE );
    assert( errval == NINJA_E_NULL_PTR );

    errval = NinjaSetWxModelShpOutFlag( ninja, 0, FALSE );
    assert( errval == NINJA_SUCCESS );

    /************************ 
     * ASCII output methods *
     ************************/
    errval = NinjaSetWxModelAsciiOutFlag( ninja, 0, TRUE );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetWxModelAsciiOutFlag( NULL, 0, TRUE );
    assert( errval == NINJA_E_NULL_PTR );

    errval = NinjaSetWxModelAsciiOutFlag( ninja, 0, FALSE );
    assert( errval == NINJA_SUCCESS );

    /************************ 
     * Google output methods *
     ************************/
    errval = NinjaSetGoogOutFlag( ninja, 0, TRUE );
    assert( errval == NINJA_SUCCESS );

    /*  setGoogResolution  */
    errval = NinjaSetGoogResolution( ninja, 0, 10, "ft" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetGoogResolution( ninja, 0, 10, "m" );
    assert( errval == NINJA_SUCCESS );
    
    errval = NinjaSetGoogResolution( ninja, 0, 1, "mi" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetGoogResolution( ninja, 0, 1, "km" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetGoogResolution( ninja, 0, 10, "ftx10" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetGoogResolution( ninja, 0, 10, "mx10" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetGoogResolution( NULL, 0, 10, "ft" );
    assert( errval == NINJA_E_NULL_PTR );

    errval = NinjaSetGoogResolution( ninja, 0, 10, "x" );
    assert( errval == NINJA_E_INVALID ); 

    /*  setGoogSpeedScaling  */
    errval = NinjaSetGoogSpeedScaling( ninja, 0, "equal_color" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetGoogSpeedScaling( ninja, 0, "equal_interval" );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetGoogSpeedScaling( ninja, 0, "hello" );
    assert( errval == NINJA_E_INVALID );

    /*  setGoogLineWidth  */
    errval = NinjaSetGoogLineWidth( ninja, 0, 2 );
    assert( errval == NINJA_SUCCESS );

    errval = NinjaSetGoogLineWidth( NULL, 0, 2 );
    assert( errval == NINJA_E_NULL_PTR );



    /*  Invalid inputs for setGoogOutFlag */
    errval = NinjaSetGoogOutFlag( NULL, 0, TRUE );
    assert( errval == NINJA_E_NULL_PTR );

    errval = NinjaSetGoogOutFlag( ninja, 0, FALSE );
    assert( errval == NINJA_SUCCESS );

}

int main()
{
    //Create an army
    ninja  = NinjaCreateArmy( 1, NULL );
    assert( NULL != ninja );

    checkInitializationMethods();
    checkInputMethods();
    checkOutputMethods();
    checkEnvironmentMethods();
    checkMeshResolution();
    checkOutputWritingMethods();
   
    errval =  NinjaDestroyArmy( ninja );
    assert( errval == NINJA_SUCCESS );

    return 0;
}


