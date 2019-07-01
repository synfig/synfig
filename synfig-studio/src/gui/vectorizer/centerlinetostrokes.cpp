/* === S Y N F I G ========================================================= */
/*!	\file centerlinetostrokes.cpp
**	\brief centerlinetostrokes
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

#include "polygonizerclasses.h"

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */
//--------------------------------------------------------------------------

// Converts each forward or single Sequence of the image in its corresponding
// TStroke. Output is a vector<TStroke*>* whose ownership belongs to the user.
// This allow sorts on the TStroke vector *before* adding any stroke to the
// output TVectorImage.
// In synfig we will be using outline layer instead of TStroke  

void conversionToStrokes(std::vector< *> &strokes, VectorizerCoreGlobals &g) 
{
  SequenceList &singleSequences           = g.singleSequences;
  JointSequenceGraphList &organizedGraphs = g.organizedGraphs;
  double penalty                          = g.currConfig->m_penalty;

  unsigned int i, j, k;

  // Convert single sequences
  for (i = 0; i < singleSequences.size(); ++i) 
  {
    if (singleSequences[i].m_head == singleSequences[i].m_tail) 
    {
      // If the sequence is circular, move your endpoints to an edge middle, in
      // order
      // to allow a soft junction
      SkeletonGraph *currGraph = singleSequences[i].m_graphHolder;

      unsigned int head     = singleSequences[i].m_head;
      unsigned int headLink = singleSequences[i].m_headLink;
      unsigned int next = currGraph->getNode(head).getLink(headLink).getNext();
      unsigned int nextLink = currGraph->getNode(next).linkOfNode(head);

      unsigned int addedNode = singleSequences[i].m_graphHolder->newNode(
          (*currGraph->getNode(head) + *currGraph->getNode(next)) * 0.5);

      singleSequences[i].m_graphHolder->insert(addedNode, head, headLink);
      *singleSequences[i].m_graphHolder->node(addedNode).link(0) =
          *singleSequences[i].m_graphHolder->node(head).link(headLink);

      singleSequences[i].m_graphHolder->insert(addedNode, next, nextLink);
      *singleSequences[i].m_graphHolder->node(addedNode).link(1) =
          *singleSequences[i].m_graphHolder->node(next).link(nextLink);

      singleSequences[i].m_head     = addedNode;
      singleSequences[i].m_headLink = 0;
      singleSequences[i].m_tail     = addedNode;
      singleSequences[i].m_tailLink = 1;
    }

    strokes.push_back(convert(singleSequences[i], penalty));
  }

  // Convert graph sequences
  for (i = 0; i < organizedGraphs.size(); ++i)
    for (j = 0; j < organizedGraphs[i].getNodesCount(); ++j)
      if (!organizedGraphs[i].getNode(j).hasAttribute(
              JointSequenceGraph::ELIMINATED))
        // Otherwise eliminated by junction recovery
        for (k = 0; k < organizedGraphs[i].getNode(j).getLinksCount(); ++k) {
          // A sequence is taken at both extremities in our organized graphs
          if (organizedGraphs[i].getNode(j).getLink(k)->isForward())
            strokes.push_back(
                convert(*organizedGraphs[i].getNode(j).getLink(k), penalty));
        }
}


