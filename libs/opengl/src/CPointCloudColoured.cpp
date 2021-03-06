/* +---------------------------------------------------------------------------+
   |          The Mobile Robot Programming Toolkit (MRPT) C++ library          |
   |                                                                           |
   |                       http://www.mrpt.org/                                |
   |                                                                           |
   |   Copyright (C) 2005-2011  University of Malaga                           |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics Lab, University of Malaga (Spain).                          |
   |    Contact: Jose-Luis Blanco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MRPT project.                                   |
   |                                                                           |
   |     MRPT is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MRPT is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */

#include <mrpt/opengl.h>  // Precompiled header


#include <mrpt/opengl/CPointCloudColoured.h>
#include <mrpt/math/utils.h>

#include "opengl_internals.h"

using namespace mrpt;
using namespace mrpt::opengl;
using namespace mrpt::utils;
//using namespace mrpt::slam;
using namespace mrpt::math;
using namespace std;

IMPLEMENTS_SERIALIZABLE( CPointCloudColoured, CRenderizable, mrpt::opengl )

/*---------------------------------------------------------------
							render
  ---------------------------------------------------------------*/
void   CPointCloudColoured::render() const
{
#if MRPT_HAS_OPENGL_GLUT
	octree_assure_uptodate(); // Rebuild octree if needed
	m_last_rendered_count_ongoing = 0;

	// Info needed by octree renderer:
	gl_utils::TRenderInfo ri;
	gl_utils::getCurrentRenderingInfo(ri);

	if ( m_color.A != 255 )
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

    glPointSize( m_pointSize );

    if (m_pointSmooth)
			glEnable ( GL_POINT_SMOOTH );
	else 	glDisable( GL_POINT_SMOOTH );


    glBegin( GL_POINTS );
	octree_render(ri); // Render all points recursively:
    glEnd();

	// Undo flags:
	if ( m_color.A != 255 )
		glDisable(GL_BLEND);

	if (m_pointSmooth)
		glDisable( GL_POINT_SMOOTH );

	m_last_rendered_count = m_last_rendered_count_ongoing;

	checkOpenGLError();
#endif
}

/** Render a subset of points (required by octree renderer) */
void  CPointCloudColoured::render_subset(const bool all, const std::vector<size_t>& idxs, const float render_area_sqpixels ) const
{
#if MRPT_HAS_OPENGL_GLUT
	const size_t N  = all ? m_points.size() : idxs.size();
	const size_t decimation = mrpt::utils::round( std::max(1.0f, static_cast<float>(N / (mrpt::global_settings::OCTREE_RENDER_MAX_DENSITY_POINTS_PER_SQPIXEL * render_area_sqpixels)) ) );

	m_last_rendered_count_ongoing += N/decimation;

	m_last_rendered_count_ongoing += (all ? m_points.size() : idxs.size())/decimation;

	if (all)
	{
		for (size_t i=0;i<N;i+=decimation)
		{
			const TPointColour &p = m_points[i];
			glColor4f( p.R, p.G, p.B,m_color.A*1.0f/255.f );
			glVertex3f( p.x,p.y,p.z );
		}
	}
	else
	{
		for (size_t i=0;i<N;i+=decimation)
		{
			const TPointColour &p = m_points[idxs[i]];
			glColor4f( p.R, p.G, p.B,m_color.A*1.0f/255.f  );
			glVertex3f( p.x,p.y,p.z );
		}
	}
#endif
}


/*---------------------------------------------------------------
   Implements the writing to a CStream capability of
     CSerializable objects
  ---------------------------------------------------------------*/
void  CPointCloudColoured::writeToStream(CStream &out,int *version) const
{

	if (version)
		*version = 2;
	else
	{
		writeToStreamRender(out);
		out << m_points;
		out << m_pointSize;
		out << m_pointSmooth; // Added in v2
	}
}

/*---------------------------------------------------------------
	Implements the reading from a CStream capability of
		CSerializable objects
  ---------------------------------------------------------------*/
void  CPointCloudColoured::readFromStream(CStream &in,int version)
{
	switch(version)
	{
	case 1:
	case 2:
		{
			readFromStreamRender(in);
			in >> m_points >> m_pointSize;

			if (version>=2)
					in >> m_pointSmooth;
			else 	m_pointSmooth = false;

		} break;
	case 0:
		{
			readFromStreamRender(in);

			// Old vector_serializable:
			uint32_t n;
			in >> n;
			m_points.resize(n);
			for (uint32_t i=0;i<n;i++) in >> m_points[i];

			in >> m_pointSize;
		} break;
	default:
		MRPT_THROW_UNKNOWN_SERIALIZATION_VERSION(version)
	};
	markAllPointsAsNew();
}

CStream& mrpt::opengl::operator >> (CStream& in,  CPointCloudColoured::TPointColour &o)
{
	in >> o.x >> o.y >> o.z >> o.R >> o.G >> o.B;
	return in;
}


CStream& mrpt::opengl::operator << (CStream& out, const CPointCloudColoured::TPointColour &o)
{
	out << o.x << o.y << o.z << o.R << o.G << o.B;
	return out;
}


/** Write an individual point (checks for "i" in the valid range only in Debug). */
void CPointCloudColoured::setPoint(size_t i, const TPointColour &p )
{
#ifdef _DEBUG
	ASSERT_BELOW_(i,size())
#endif
	m_points[i] = p;

	// JL: TODO note: Well, this can be clearly done much more efficiently but...I don't have time! :-(
	markAllPointsAsNew();
}

/** Inserts a new point into the point cloud. */
void CPointCloudColoured::push_back(float x,float y,float z, float R, float G, float B)
{
	m_points.push_back(TPointColour(x,y,z,R,G,B));

	// JL: TODO note: Well, this can be clearly done much more efficiently but...I don't have time! :-(
	markAllPointsAsNew();
}


// Do needed internal work if all points are new (octree rebuilt,...)
void CPointCloudColoured::markAllPointsAsNew()
{
	octree_mark_as_outdated();
}

/** In a base class, reserve memory to prepare subsequent calls to PLY_import_set_vertex */
void CPointCloudColoured::PLY_import_set_vertex_count(const size_t N)
{
	this->resize(N);
}

/** In a base class, will be called after PLY_import_set_vertex_count() once for each loaded point.
  *  \param pt_color Will be NULL if the loaded file does not provide color info.
  */
void CPointCloudColoured::PLY_import_set_vertex(const size_t idx, const mrpt::math::TPoint3Df &pt, const mrpt::utils::TColorf *pt_color)
{
	if (!pt_color)
	      this->setPoint(idx,TPointColour(pt.x,pt.y,pt.z,1,1,1));
	else  this->setPoint(idx,TPointColour(pt.x,pt.y,pt.z,pt_color->R,pt_color->G,pt_color->B));
}

/** In a base class, return the number of vertices */
size_t CPointCloudColoured::PLY_export_get_vertex_count() const
{
	return this->size();
}

/** In a base class, will be called after PLY_export_get_vertex_count() once for each exported point.
  *  \param pt_color Will be NULL if the loaded file does not provide color info.
  */
void CPointCloudColoured::PLY_export_get_vertex(
	const size_t idx,
	mrpt::math::TPoint3Df &pt,
	bool &pt_has_color,
	mrpt::utils::TColorf &pt_color) const
{
	const TPointColour &p = m_points[idx];
	pt.x = p.x;
	pt.y = p.y;
	pt.z = p.z;
	pt_color.R = p.R;
	pt_color.G = p.G;
	pt_color.B = p.B;
	pt_has_color=true;
}
