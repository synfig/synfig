
/* === S Y N F I G ========================================================= */
/*!	\file vectorizer/vectorizerparameters.h
**
**	\legal
**	This file uses code from OpenToonz open-source animation software  (https://github.com/opentoonz/opentoonz/), which is developed from Toonz, a software originally created by Digital Video, S.p.A., Rome Italy Digital Video, S.p.A., Rome Italy.
**
**	Copyright (c) 2016 - 2019, DWANGO Co., Ltd.
**	Copyright (c) 2016 Toshihiro Shimizu - https://github.com/meso
**	Copyright (c) 2016 Shinya Kitaoka - https://github.com/skitaoka
**	Copyright (c) 2016 shun-iwasawa - https://github.com/shun-iwasawa
**	Copyright (c) 2016 Campbell Barton - https://github.com/ideasman42
**	Copyright (c) 2019 luzpaz - https://github.com/luzpaz
**
**	Copyright (c) 2019 - 2020, Ankit Kumar Dwivedi - https://github.com/ankit-kumar-dwivedi
**
**	LICENSE
** 
**	BSD 3-Clause "New" or "Revised" License
** 
**	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
**
**	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
**
**	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
**
**	3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
**
**	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

  \details  All vectorization options and information are passed from higher
            (like \p VectorizerPopup) to lower layers (\p VectorizerCore) of
            the vectorization process inside a \p VectorizerConfiguration
  variable.
            This typically includes vectorization modes, various sensitivity and
            accuracy parameters, and post-processing information. This class
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


