/* === S Y N F I G ========================================================= */
/*!	\file skeletonizer.cpp
**	\brief skeletonizer File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "polygonizerclasses.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */


// Forward declarations

struct VectorizationContext;

class ContourEdge {
public:
  enum { NOT_OPPOSITE = 0x1 };

public:
  Point m_direction;
  unsigned short m_attributes;

public:
  ContourEdge() : m_attributes(0) {}
  ContourEdge(Point dir) : m_direction(dir), m_attributes(0) {}

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
    {
    special,           //!< A vertex event that is also an edge event (V case).
    edge,              //!< An edge shrinks to 0 length.
    vertex,            //!< Two contour nodes clash.
    split_regenerate,  //!< Placeholder type for split events that must be regenerated.
    split,             //!< An edge is split by a clashing contour node.
    failure 
    };

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