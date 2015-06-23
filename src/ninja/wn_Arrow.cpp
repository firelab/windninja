//
// wn_OGRVector.cpp
//
// Copyright (c) 2015 Research In Motion
//

#include "wn_Arrow.h"


WN_Arrow::WN_Arrow()
{
    m_x          = m_y           = m_speed      = m_dir         = 0;
    m_xtip       = m_ytip        = m_xtail      = m_ytail       = 0;
    m_xhead_left = m_xhead_right = m_yhead_left = m_yhead_right = 0;
    m_cell_size  = m_nsplits     = 0;

    m_thresholds = NULL;
}

WN_Arrow::WN_Arrow(
        const double &x,           const double &y,
        const double &speed,       const double &direction,
        const double &cell_size,   const double *thresholds,
        const unsigned int nsplits
        )
{
    m_x          = x;
    m_y          = y;
    m_speed      = speed;
    m_dir        = direction;
    m_cell_size  = cell_size;
    m_thresholds = thresholds;
    m_nsplits    = nsplits;

    if( NULL == m_thresholds )
    {
        throw std::invalid_argument( "wn_OGRVector: constructor called with thresholds = NULL" );
    }

    _computeVectorPoints();

}

WN_Arrow::~WN_Arrow()
{
    
}

void WN_Arrow::asGeometry(OGRGeometryH & hLine)
{
    double z = m_cell_size / 8.0;

    OGR_G_AddPoint_2D( hLine, m_xhead_right, m_yhead_right );
    OGR_G_AddPoint_2D( hLine, m_xtip       , m_ytip        );
    OGR_G_AddPoint_2D( hLine, m_xhead_left , m_yhead_left  );
    OGR_G_AddPoint_2D( hLine, m_xtip       , m_ytip        );
    OGR_G_AddPoint_2D( hLine, m_xtail      , m_ytail       );

    return;
}

void WN_Arrow::_computeVectorPoints()
{
    double xpt       = 0;
    double ypt       = 0;
    double theta     = 0;
    double getheta   = 0;
    double yscale    = 0.5;
    double xscale    = 0.5 * yscale;
    double sin_theta = 0;
    double cos_theta = 0;


    float scale_factor =  1.0 / m_nsplits;
    for ( unsigned int i = 0; i < m_nsplits; ++i )
    {
        if( m_speed <= m_thresholds[i] )
        {
            yscale *= (scale_factor * (i+1));
            break;
        }
    }
    xscale = yscale * 0.40;

    getheta = m_dir;
    theta   = m_dir + 180.0;
    if( theta > 360 )
    {
        theta -= 360;
    }
    theta = 360 - theta;
    theta = theta * (pi / 180.0);

    if( areEqual( m_speed, 0.0 ) )
    {
        double offset = m_cell_size / 16;
        m_xtip        = m_x - offset;
        m_ytip        = m_y + offset;
        m_xtail       = m_x + offset;
        m_ytail       = m_y + offset;
        m_xhead_left  = m_x - offset;
        m_yhead_left  = m_y - offset;
        m_xhead_right = m_x + offset;
        m_yhead_right = m_y - offset;
    }
    else
    {
        xpt           = 0;
        ypt           = m_cell_size * yscale;
        cos_theta     = cos(theta);
        sin_theta     = sin(theta);

        m_xtip        = ( xpt * cos_theta ) - ( ypt * sin_theta );
        m_ytip        = ( xpt * sin_theta ) + ( ypt * cos_theta );
        m_xtail       = -m_xtip;
        m_ytail       = -m_ytip;

        xpt           = m_cell_size * xscale;
        ypt           = m_cell_size * ( yscale - xscale );

        m_xhead_right = ( xpt * cos_theta ) - ( ypt * sin_theta );
        m_yhead_right = ( xpt * sin_theta ) + ( ypt * cos_theta );

        xpt           = -xpt;
        ypt           =  ypt;

        m_xhead_left = ( xpt * cos_theta ) - ( ypt * sin_theta );
        m_yhead_left = ( xpt * sin_theta ) + ( ypt * cos_theta );

        m_xtip        += m_x;
        m_ytip        += m_y;
        m_xtail       += m_x;
        m_ytail       += m_y;
        m_xhead_right += m_x;
        m_yhead_right += m_y;
        m_xhead_left  += m_x;
        m_yhead_left  += m_y;

    }

}

