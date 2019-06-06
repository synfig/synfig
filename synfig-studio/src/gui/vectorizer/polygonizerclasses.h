/* === S Y N F I G ========================================================= */
/*!	\file template.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
#include <synfig/vector.h>


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class ContourEdge;

// NOTE: The following class is mainly used in the later 'straight skeleton
// computation'
//       - for polygonization purposes, consider it like a TPointD class.

class ContourNode {
public:
  enum Attributes           //! Node attributes
  { HEAD            = 0x1,  //!< Node is the 'first' of a nodes ring.
    ELIMINATED      = 0x4,  //!< Node was eliminated by the SS process.
    SK_NODE_DROPPED = 0x8,
    AMBIGUOUS_LEFT  = 0x10,  //!< Node represents an ambiguous \a left turn in
                             //!  the original image.
    AMBIGUOUS_RIGHT = 0x20,  //!< Node represents an ambiguous \a right turn in
                             //!  the original image.
    JR_RESERVED  = 0x40,     //!< Reserved for joints recovery.
    LINEAR_ADDED = 0x80  //!< Node was added by the linear skeleton technique.
  };

public:
  // Node kinematics infos
  synfig::Point3 m_position,      //!< Node's position.
      m_direction,           //!< Node's direction.
      m_AngularMomentum,     //!< Angular momentum with the next node's edge.
      m_AuxiliaryMomentum1,  // Used only when this vertex is convex
      m_AuxiliaryMomentum2;  // Used only when this vertex is convex

  // Further node properties
  bool m_concave;             //!< Whether the node represents a concave angle.
  unsigned int m_attributes,  //!< Bitwise signatures of this node
      m_updateTime,  //!< \a Algoritmic time in which the node was updated.
      m_ancestor,  //!< Index of the original node from which this one evolved.
      m_ancestorContour;  //!< Contour index of the original node from which
                          //! this one evolved.
  std::vector<ContourEdge *> m_notOpposites;  //!< List of edges \a not to be
                                              //! used as possible opposites.
  int m_outputNode;  //!< Skeleton node produced by this ContourNode.

  // Connective data
  ContourEdge *m_edge;  //!< Edge departing from this, keeping adjacent black
                        //!  region on the right
  // Node neighbours
  ContourNode *m_next;  //!< Next node on the contour.
  ContourNode *m_prev;  //!< Previous node on the contour.

public:
  ContourNode() : m_attributes(0) {}
  ContourNode(double x, double y) : m_position(x, y, 0), m_attributes(0) {}
  ContourNode(const synfig::Point &P) : m_position(P[0], P[1], 0), m_attributes(0) {}
  ContourNode(double x, double y, unsigned short attrib)
      : m_position(x, y, 0), m_attributes(attrib) {}

  int hasAttribute(int attr) const { return m_attributes & attr; }
  void setAttribute(int attr) { m_attributes |= attr; }
  void clearAttribute(int attr) { m_attributes &= ~attr; }

public:
  // Private Node Methods
  inline void buildNodeInfos(bool forceConvex = false);
};

//--------------------------------------------------------------------------
// basically creating 1D, 2D and 3D matrix of ContourNode
// 1D matrix being called Contour
// 2D matrix is ContourFamily and 
// 3D is Contours
typedef std::vector<ContourNode> Contour;
typedef std::vector<Contour> ContourFamily;
typedef std::vector<ContourFamily> Contours;

class VectorizerCoreGlobals {
public:
  const CenterlineConfiguration *currConfig;

  // JointSequenceGraphList organizedGraphs;
  // SequenceList singleSequences;
  // PointList singlePoints;

  VectorizerCoreGlobals() {}
  ~VectorizerCoreGlobals() {}
};

namespace {
// SkeletonGraph nodes global signatures - used for various purposes
enum {
  ORGANIZEGRAPHS_SIGN = 0x10,
  SAMPLECOLOR_SIGN    = 0x20,
  COLORORDERING_SIGN  = 0x40
};
const int infinity = 1000000;  // just a great enough number
};

//--------------------------------------------------------------------------

//===============================
//    Function prototypes
//===============================

void polygonize(const etl::handle<synfig::Layer_Bitmap> &ras, Contours &polygons,VectorizerCoreGlobals &g);

// SkeletonList *skeletonize(Contours &contours, VectorizerCore *thisVectorizer,
//                           VectorizerCoreGlobals &g);

// void organizeGraphs(SkeletonList *skeleton, VectorizerCoreGlobals &g);

// void junctionRecovery(Contours *polygons, VectorizerCoreGlobals &g);

// void conversionToStrokes(std::vector<TStroke *> &strokes,
//                          VectorizerCoreGlobals &g);

// void calculateSequenceColors(const TRasterP &ras, VectorizerCoreGlobals &g);

// void applyStrokeColors(std::vector<TStroke *> &strokes, const TRasterP &ras,
//                        TPalette *palette, VectorizerCoreGlobals &g);


}; // END of namespace studio

/* === E N D =============================================================== */
