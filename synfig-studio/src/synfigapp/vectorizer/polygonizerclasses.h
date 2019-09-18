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

#ifndef __SYNFIG_APP_POLYGONIZERCLASSES_H
#define __SYNFIG_APP_POLYGONIZERCLASSES_H

/* === H E A D E R S ======================================================= */
#include <synfig/vector.h>
#include "centerlinevectorizer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

//*********************************
//       Traversable Graphs
//*********************************

typedef unsigned int UINT;

/*!
  \brief    Graph class used by the centerline vectorization process.

  \details  Introducing a directed graph structure that allows local access:
  main feature is that a graph edge
            physically belongs to the node that emitted it, by storing it in a
  'link vector' inside the node.
            No full-scale edge search method is therefore needed to find node
  neighbours.

            No specific iterator class is needed, just use unsigned ints to
  perform random access to nodes and
            links vectors.
*/
template <class T, class I>
void append(T &cont1, T &cont2) 
{
  I i, j;

  cont1.resize(cont1.size() + cont2.size());
  for (i = cont2.rbegin(), j = cont1.rbegin(); i != cont2.rend(); ++i, ++j)
    *j = *i;
}

template <typename NodeContentType, typename ArcType>
class Graph {
public:
  class Link {
    UINT m_next;    //!< Index of the node pointed by this link.
    ArcType m_arc;  //!< Edge data associated to this link.
    int m_access;   //!< Whether access to a node is allowed
                    //!  through this link.
  public:
    Link() : m_access(1) {}
    Link(UINT _next) : m_next(_next), m_access(1) {}
    Link(UINT _next, ArcType _arc) : m_next(_next), m_arc(_arc), m_access(1) {}
    ~Link() {}

    ArcType &operator*() { return m_arc; }
    const ArcType &operator*() const { return m_arc; }

    ArcType *operator->() { return &m_arc; }
    const ArcType *operator->() const { return &m_arc; }

    UINT getNext() const { return m_next; }
    void setNext(UINT _next) { m_next = _next; }

    int getAccess() const { return m_access; }
    void setAccess(int acc) { m_access = acc; }
  };

  //--------------------------------------------------------------------------

  class Node {
    friend class Graph;  // Grant Graph access to m_links

    std::vector<Link> m_links;  //!< Links to neighbouring nodes.
    NodeContentType m_content;  //!< The node's content.
    int m_attributes;           //!< Node attributes.

  public:
    Node() : m_attributes(0) {}
    Node(const NodeContentType &_cont) : m_content(_cont), m_attributes(0) {}
    ~Node() {}

    Link &link(UINT i) { return m_links[i]; }
    const Link &getLink(UINT i) const { return m_links[i]; }
    UINT getLinksCount() const { return m_links.size(); }

    NodeContentType &operator*() { return m_content; }
    const NodeContentType &operator*() const { return m_content; }

    NodeContentType *operator->() { return &m_content; }
    const NodeContentType *operator->() const { return &m_content; }

    // Attributes
    int hasAttribute(int attr) const { return m_attributes & attr; }
    void setAttribute(int attr) { m_attributes |= attr; }
    void clearAttribute(int attr) { m_attributes &= ~attr; }

    // Others
    int degree() const { return int(m_links.size()); }

    /*!
\warning    If more links can be set between the same nodes, the
      returned link index will be ambiguous.
*/
    UINT linkOfNode(UINT next) const {
      UINT i = 0;
      for (; i < m_links.size() && m_links[i].getNext() != next; ++i)
        ;
      return i;
    }
  };

public:
  std::vector<Node> m_nodes;  //!< Nodes container.
  UINT m_linksCount;          //!< Links counter.

public:
  Graph() : m_linksCount(0) {}
  virtual ~Graph() {}

  Node &node(UINT i) { return m_nodes[i]; }
  const Node &getNode(UINT i) const { return m_nodes[i]; }

  UINT getNodesCount() const { return m_nodes.size(); }
  UINT getLinksCount() const { return m_linksCount; }

  // Nodes/Links insertions
  UINT newNode() {
    m_nodes.push_back(Node());
    return m_nodes.size() - 1;
  }

  UINT newNode(const NodeContentType &content) {
    m_nodes.push_back(Node(content));
    return m_nodes.size() - 1;
  }

  UINT newLink(UINT first, UINT last) {
    assert(first < m_nodes.size() && last < m_nodes.size());
    m_nodes[first].m_links.push_back(Link(last));
    ++m_linksCount;
    return m_nodes[first].m_links.size() - 1;
  }

  UINT newLink(UINT first, UINT last, const ArcType &arc) {
    assert(first < m_nodes.size() && last < m_nodes.size());
    m_nodes[first].m_links.push_back(Link(last, arc));
    ++m_linksCount;
    return m_nodes[first].m_links.size() - 1;
  }

  void insert(UINT inserted, UINT afterNode, UINT onLink) {
    newLink(inserted, getNode(afterNode).getLink(onLink).getNext());
    node(afterNode).link(onLink).setNext(inserted);
  }
};

//********************************
//*    Polygonization classes    *
//********************************

//--------------------------------------------------------------------------

// Of course we don't want RawBorders to be entirely copied whenever STL
// requires to resize a BorderFamily...
class RawBorder;

typedef std::vector<RawBorder *> BorderFamily;
typedef std::vector<BorderFamily> BorderList;

//--------------------------------------------------------------------------

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
//-----------------------------------
//    Straight Skeleton Classes
//-----------------------------------

class SkeletonArc {
  double m_slope;
  unsigned int m_leftGeneratingNode, m_leftContour, m_rightGeneratingNode,
      m_rightContour;
  int m_attributes;

  // NOTE:  Typically an arc is generated by a couple of *edges* of the original
  //        contours; but we store instead the *nodes* which address those
  //        edges.

public:
  SkeletonArc() : m_attributes(0) {}
  SkeletonArc(ContourNode *node)
      : m_slope(node->m_direction[2])
      , m_leftGeneratingNode(node->m_ancestor)
      , m_leftContour(node->m_ancestorContour)
      , m_rightGeneratingNode(node->m_prev->m_ancestor)
      , m_rightContour(node->m_prev->m_ancestorContour)
      , m_attributes(0) {}

  enum { ROAD = 0x1 };

  double getSlope() const { return m_slope; }

  unsigned int getLeftGenerator() const { return m_leftGeneratingNode; }
  unsigned int getRightGenerator() const { return m_rightGeneratingNode; }
  unsigned int getLeftContour() const { return m_leftContour; }
  unsigned int getRightContour() const { return m_rightContour; }

  enum { SS_OUTLINE = 0x10, SS_OUTLINE_REVERSED = 0x20 };

  int hasAttribute(int attr) const { return m_attributes & attr; }
  void setAttribute(int attr) { m_attributes |= attr; }

  void turn() {
    m_slope = -m_slope;

    std::swap(m_leftGeneratingNode, m_rightGeneratingNode);
    std::swap(m_leftContour, m_rightContour);
  }
};

//--------------------------------------------------------------------------

typedef Graph<synfig::Point3, SkeletonArc> SkeletonGraph;
typedef std::vector<SkeletonGraph *> SkeletonList;

//==========================================================================

//----------------------------------------
//    Joints and Sequences definition
//----------------------------------------

class Sequence 
{
public:
  UINT m_head;
  UINT m_headLink;
  UINT m_tail;
  UINT m_tailLink;
  SkeletonGraph *m_graphHolder;

  // Stroke color-sensible data
  synfig::Color m_color;
  int m_strokeIndex;
  int m_strokeHeight;

public:
  Sequence() : m_graphHolder(0) {}
  ~Sequence() {}

  // Impose a property dependant only on the extremity we consider first
  // - so that the same sequence is not considered twice when head and tail
  // are exchanged
  bool isForward() const 
  {
    return (m_head < m_tail) || (m_head == m_tail && m_headLink < m_tailLink);
  }

  // Advances a couple (old, current) of sequence nodes
  void advance(UINT &old, UINT &current) const 
  {
    UINT temp = current;
    current   = m_graphHolder->getNode(current).getLink(0).getNext() == old
                  ? m_graphHolder->getNode(current).getLink(1).getNext()
                  : m_graphHolder->getNode(current).getLink(0).getNext();
    old = temp;
  }

  // Advances a couple (current, link) of a sequence node plus its link
  // direction
  void next(UINT &current, UINT &link) const 
  {
    UINT temp = current;
    current   = m_graphHolder->getNode(current).getLink(link).getNext();
    link = m_graphHolder->getNode(current).getLink(0).getNext() == temp ? 1 : 0;
  }
};

//--------------------------------------------------------------------------

class JointSequenceGraph final : public Graph<UINT, Sequence> {
public:
  JointSequenceGraph() {}
  ~JointSequenceGraph() {}

  enum { REACHED = 0x1, ELIMINATED = 0x2 };

  // Extracts JSG tail link of input node-link
  inline UINT tailLinkOf(UINT node, UINT link) 
  {
    UINT i, next = getNode(node).getLink(link).getNext();

    for (i = 0; getNode(next).getLink(i)->m_tail !=
                    getNode(node).getLink(link)->m_head ||
                getNode(next).getLink(i)->m_tailLink !=
                    getNode(node).getLink(link)->m_headLink;
         ++i)
      ;
    return i;
  }
};

typedef std::vector<JointSequenceGraph> JointSequenceGraphList;

typedef std::vector<Sequence> SequenceList;
typedef std::vector<synfig::Point3> PointList;


//==========================================================================



class VectorizerCoreGlobals {
public:
  const CenterlineConfiguration *currConfig;

  JointSequenceGraphList organizedGraphs;
  SequenceList singleSequences;
  PointList singlePoints;

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

SkeletonList *skeletonize(Contours &contours, VectorizerCore *thisVectorizer, VectorizerCoreGlobals &g);

void organizeGraphs(SkeletonList *skeleton, VectorizerCoreGlobals &g);

// void junctionRecovery(Contours *polygons, VectorizerCoreGlobals &g);

void conversionToStrokes(std::vector< etl::handle<synfig::Layer> > &strokes, VectorizerCoreGlobals &g, const etl::handle<synfig::Layer_Bitmap> &image) ;

 void calculateSequenceColors(const etl::handle<synfig::Layer_Bitmap> &ras, VectorizerCoreGlobals &g);

// void applyStrokeColors(std::vector<TStroke *> &strokes, const TRasterP &ras,
//                        TPalette *palette, VectorizerCoreGlobals &g);


}; // END of namespace studio

/* === E N D =============================================================== */

#endif
