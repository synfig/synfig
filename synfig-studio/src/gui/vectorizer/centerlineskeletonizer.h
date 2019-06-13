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

#ifndef __SYNFIG_STUDIO_TEMPLATE_H
#define __SYNFIG_STUDIO_TEMPLATE_H

/* === H E A D E R S ======================================================= */
#include "polygonizerclasses.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

struct VectorizationContext;

class ContourEdge {
public:
  enum { NOT_OPPOSITE = 0x1 };

public:
  synfig::Point m_direction;
  unsigned short m_attributes;

public:
  ContourEdge() : m_attributes(0) {}
  ContourEdge(synfig::Point dir) : m_direction(dir), m_attributes(0) {}

  int hasAttribute(int attr) const { return m_attributes & attr; }
  void setAttribute(int attr) { m_attributes |= attr; }
  void clearAttribute(int attr) { m_attributes &= ~attr; }
};

//--------------------------------------------------------------------------

class IndexTable {
public:
  typedef std::list<ContourNode *> IndexColumn;

  std::vector<IndexColumn>
      m_columns;  //!< Countours set by 'column identifier'.
  std::vector<int>
      m_identifiers;  //!< Column identifiers by original contour index.

  // NOTE: Contours are stored in 'comb' structure (vector of lists) since
  // contours may both
  // be SPLIT (new entry in a list) and MERGED (two lists merge).

public:
  IndexTable() {}

  IndexColumn *operator[](int i) { return &m_columns[i]; }
  IndexColumn &columnOfId(int id) { return m_columns[m_identifiers[id]]; }

  // Initialization
  void build(ContourFamily &family);
  void clear();

  // Specific handlers
  IndexColumn::iterator find(ContourNode *index);
  void merge(IndexColumn::iterator index1, IndexColumn::iterator index2);
  void remove(IndexColumn::iterator index);
};

//--------------------------------------------------------------------------

class Event {
public:
  /*! \remark  Values are sorted by preference at simultaneous events.    */

  enum Type            //! An event's possible types.
  { special,           //!< A vertex event that is also an edge event (V case).
    edge,              //!< An edge shrinks to 0 length.
    vertex,            //!< Two contour nodes clash.
    split_regenerate,  //!< Placeholder type for split events that must be
                       //! regenerated.
    split,             //!< An edge is split by a clashing contour node.
    failure };

public:
  double m_height;
  double m_displacement;
  ContourNode *m_generator;
  ContourNode *m_coGenerator;
  Type m_type;
  unsigned int m_algoritmicTime;

  VectorizationContext *m_context;

public:
  // In-builder event constructor
  Event(ContourNode *generator, VectorizationContext *context);

  // Event calculators
  inline void calculateEdgeEvent();
  inline void calculateSplitEvent();

  // Auxiliary event calculators
  inline double splitDisplacementWith(ContourNode *plane);
  inline bool tryRayEdgeCollisionWith(ContourNode *edge);

  // Event handlers
  inline bool process();
  inline void processEdgeEvent();
  inline void processMaxEvent();
  inline void processSplitEvent();
  inline void processVertexEvent();
  inline void processSpecialEvent();

private:
  inline bool testRayEdgeCollision(ContourNode *opposite, double &displacement,
                                   double &height, double &side1,
                                   double &side2);
};

//--------------------------------------------------------------------------

struct EventGreater {
  bool operator()(const Event &event1, const Event &event2) const {
    return event1.m_height > event2.m_height ||
           (event1.m_height == event2.m_height &&
            event1.m_type > event2.m_type);
  }
};

class Timeline final
    : public std::priority_queue<Event, std::vector<Event>, EventGreater> {
public:
  Timeline() {}

  // NOTE: Timeline construction contains the most complex part of
  // vectorization;
  // progress bar partial notification happens there, so thisVectorizer's signal
  // emission methods must be passed and used.
  void build(ContourFamily &polygons, VectorizationContext &context,
             VectorizerCore *thisVectorizer);
};

struct VectorizationContext 
{
  VectorizerCoreGlobals *m_globals;

  // Globals
  unsigned int m_totalNodes;      // Number of original contour nodes
  unsigned int m_contoursCount;   // Number of contours in input region
  IndexTable m_activeTable;       // Index table of active contours
  SkeletonGraph *m_output;        // Output skeleton of input region
  double m_currentHeight;         // Height of our 'roof-flooding' process
  Timeline m_timeline;            // Ordered queue of all possible events
  unsigned int m_algoritmicTime;  // Number of events precessed up to now

  // Containers
  std::vector<ContourEdge> m_edgesHeap;
  std::vector<ContourNode> m_nodesHeap;  // of *non-original* nodes only
  unsigned int m_nodesHeapCount;         // number of nodes used in nodesHeap

  //'Linear Axis-added' *pseudo-original* nodes and edges
  std::vector<ContourNode> m_linearNodesHeap;
  std::vector<ContourEdge> m_linearEdgesHeap;
  unsigned int m_linearNodesHeapCount;

public:
  VectorizationContext(VectorizerCoreGlobals *globals) : m_globals(globals) {}

  ContourNode *getNode() { return &m_nodesHeap[m_nodesHeapCount++]; }
  ContourNode *getLinearNode() {
    return &m_linearNodesHeap[m_linearNodesHeapCount];
  }
  ContourEdge *getLinearEdge() {
    return &m_linearEdgesHeap[m_linearNodesHeapCount++];
  }

  inline void addLinearNodeBefore(ContourNode *node);
  inline void repairDegenerations(
      const std::vector<ContourNode *> &degenerates);

  inline void prepareGlobals();
  inline void prepareContours(ContourFamily &family);

  inline void newSkeletonLink(unsigned int cur, ContourNode *node);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
