
/* === S Y N F I G ========================================================= */
/*!	\file vectorizer/vectorizerparameters.h
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */


/* === H E A D E R S ======================================================= */

#include <iostream>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

//*****************************************************************************
//    VectorizerConfiguration  definition
//*****************************************************************************

/*!
  \brief    Provides a base container for vectorization options.

  \details  All vectorization options and informations are passed from higher
            (like \p VectorizerPopup) to lower layers (\p VectorizerCore) of
            the vectorization process inside a \p VectorizerConfiguration
  variable.
            This typically includes vectorization modes, various sensitivity and
            accuracy parameters, and post-processing informations. This class
            merely acts as a base parameters container (although no pure virtual
            method is present) - meaning that every vectorization method
  inherits
            this base class to include its specific parameters.

  \sa       Classes \p OutlineConfiguration, \p CenterlineConfiguration,
            \p VectorizerPopup, \p Vectorizer and \p VectorizerCore.
*/
class VectorizerConfiguration {
public:
  bool m_outline;  //!< Vectorization mode between outline and centerline

  int m_threshold;  //!< Cut-out parameter to distinguish paper or painted
                    //! background
  //!  from recognizable strokes. A pixel whose tone (for colormaps)
  //!  or HSV value is under \p m_threshold is considered ink-colored.
  bool m_leaveUnpainted;  //!< Whether color recognition for areas should be
                          //! applied

  double m_thickScale;  //!< Impose a thickness reduction by this ratio

public:
  VectorizerConfiguration(bool outline)
      : m_outline(outline)
      , m_threshold(200)
      , m_leaveUnpainted(true)
      , m_thickScale(1.0) {}
};

//*****************************************************************************
//    CenterlineConfiguration  definition
//*****************************************************************************

/*!
  \brief    CenterlineConfiguration is the VectorizerConfiguration
            specialization for the centerline vectorization method.
*/

class CenterlineConfiguration final : public VectorizerConfiguration {
public:
  /*!After threshold is done, raster zones of uniform ink or paint color whose
area is under this parameter
are discarded from vectorization process. This typically helps in reducing image
scannerization noise.*/
  int m_despeckling;

  /*!Specifies the maximum thickness allowed for stroke detection. Large ink
regions can
therefore be painted with dark colors, rather than covered with very thick
strokes.
Observe that placing 0 here has the effect of an outline vectorization.*/
  double m_maxThickness;

  /*!The m_accuracy dual (see VectorizerPopup). Specifies the user preference
between
accuracy of the identified strokes, and their simplicity. It generally does not
affect the vectorization speed.
For the conversion accuracy=>penalty, see
VectorizerParameters::getCenterlineConfiguration, defined in
vectorizerparameters.cpp
*/
  double m_penalty;

  //! Imposes a thickness reduction by this ratio, at the end of
  //! VectorizerCore::vectorize method.
  double m_thicknessRatio;

  /*!Includes the transparent frame of the image in the output. Region computing
can take
advantage of it to identify close-to-boundary regions.*/
  bool m_makeFrame;

  /*!Assume that the source input is a full-color non-antialiased image (e.g.
painted level made with Retas).
This kind of image must be pre-processed and transformed to toonz-image */
  bool m_naaSource;

public:
  /*!Constructs a VectorizerConfiguration with default values.
Default options consists of a full-thickness centerline vectorization, medium
accuracy settings,
with activated region computing and painting.*/
  CenterlineConfiguration()
      : VectorizerConfiguration(false)
      , m_despeckling(10)
      , m_maxThickness(100.0)
      , m_penalty(0.5)
      , m_thicknessRatio(100.0)
      , m_makeFrame(false)
      , m_naaSource(false) {}
};

class NewOutlineConfiguration final : public VectorizerConfiguration {
public:
  double m_adherenceTol;  //!< Adherence to contour corners
  double m_angleTol;      //!< Angle-based corners tolerance
  double m_relativeTol;   //!< Relative curvature radius-based corners tolerance
  double m_mergeTol;      //!< Quadratics merging factor
  int m_despeckling;  //!< Despeckling edge size (size x size gets despeckled)

  int m_maxColors;  //!< Maximum number of palette color from fullcolor
                    //! quantization
                                //! the fullcolor case

  int m_toneTol;  //!< Tone threshold to be used in the colormap case

public:
  NewOutlineConfiguration()
      : VectorizerConfiguration(true)
      , m_adherenceTol(0.5)
      , m_angleTol(0.25)
      , m_relativeTol(0.25)
      , m_mergeTol(1.0)
      , m_despeckling(4)
      , m_maxColors(50)
      , m_toneTol(128) {}
};

}; // END of namespace studio

/* === E N D =============================================================== */


