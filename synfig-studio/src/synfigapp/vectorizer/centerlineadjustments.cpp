/* === S Y N F I G ========================================================= */
/*!	\file centerlineadjustments.cpp
**	\brief centerlineadjustments File
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
VectorizerCoreGlobals *globals;
std::vector<unsigned int> contourFamilyOfOrganized;
JointSequenceGraph *currJSGraph;
ContourFamily *currContourFamily;


/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */
//--------------------------------------
//      Skeleton re-organization
//--------------------------------------

// EXPLANATION:  The raw skeleton data obtained from StraightSkeletonizer
// class need to be grouped in joints and sequences before proceding with
// conversion in quadratics - which works on single sequences.

// NOTE: Due to maxHeight, we have to assume that a single SkeletonGraph can
// hold
// more connected graphs at once.

// a) Isolate graph-like part of skeleton
// b) Retrieve remaining single sequences.

typedef std::map<UINT, UINT, std::less<UINT>> uintMap;

// void organizeGraphs(SkeletonList* skeleton)
void studio::organizeGraphs(SkeletonList *skeleton, VectorizerCoreGlobals &g) 
{
  globals = &g;

  SkeletonList::iterator currGraphPtr;
  Sequence currSequence;
  uintMap jointsMap;
  UINT i, j;

  UINT counter = 0;  // We also count current graph number, to associate
  // organized graphs to their contour family

  contourFamilyOfOrganized.clear();

  for (currGraphPtr = skeleton->begin(); currGraphPtr != skeleton->end();++currGraphPtr) 
  {
    SkeletonGraph &currGraph   = **currGraphPtr;
    currSequence.m_graphHolder = &currGraph;

    // Separate single Points - can happen only when a single node gets stored
    // in a SkeletonGraph.
    if (currGraph.getNodesCount() == 1) 
    {
      globals->singlePoints.push_back(*currGraph.getNode(0));
      ++counter;
      continue;
    }

    // Discriminate between graphs, two-endpoint single sequences, and circular
    // ones
    bool has1DegreePoint = 0;
    for (i = 0; i < currGraph.getNodesCount(); ++i)
    {
        if (currGraph.getNode(i).degree() != 2)
        {
            if (currGraph.getNode(i).degree() == 1)
                has1DegreePoint = 1;
            else
                goto _graph;
        }
    }
      
    if (has1DegreePoint)
      goto _two_endpoint;
    else
      goto _circulars;

  _two_endpoint : {
    // Find head
    for (i = 0; currGraph.getNode(i).degree() != 1; ++i);

    currSequence.m_head     = i;
    currSequence.m_headLink = 0;

    // Find tail
    for (++i; i < currGraph.getNodesCount() && currGraph.getNode(i).degree() == 2;  ++i);

    currSequence.m_tail     = i;
    currSequence.m_tailLink = 0;

    globals->singleSequences.push_back(currSequence);

    ++counter;
    continue;
  }

  _graph : {
    // Organize Graph-like part
    globals->organizedGraphs.push_back(JointSequenceGraph());
    JointSequenceGraph &JSGraph = globals->organizedGraphs.back();
    contourFamilyOfOrganized.push_back(counter);

    jointsMap.clear();

    // Gather all sequence extremities
    for (i = 0; i < currGraph.getNodesCount(); ++i) 
    {
      if (currGraph.getNode(i).degree() != 2) 
      {
        j = JSGraph.newNode(i);
        // Using a map to keep one-to-one relation between j and i
        jointsMap.insert(uintMap::value_type(i, j));
      }
    }

    // Extract Sequences
    for (i = 0; i < JSGraph.getNodesCount(); ++i) 
    {
      UINT joint = *JSGraph.getNode(i);
      for (j = 0; j < currGraph.getNode(joint).getLinksCount(); ++j) 
      {
        currSequence.m_head     = joint;
        currSequence.m_headLink = j;

        // Seek tail
        UINT oldNode  = joint,
             thisNode = currGraph.getNode(joint).getLink(j).getNext();
        while (currGraph.getNode(thisNode).degree() == 2) 
        {
          currGraph.node(thisNode).setAttribute(
              ORGANIZEGRAPHS_SIGN);  // Sign thisNode as part of a JSGraph
          currSequence.advance(oldNode, thisNode);
        }

        currSequence.m_tail = thisNode;
        currSequence.m_tailLink = currGraph.getNode(thisNode).linkOfNode(oldNode);

        JSGraph.newLink(i, jointsMap.find(thisNode)->second, currSequence);
      }
    }
  }

  // NOTE: The following may seem uncommon - you must observe that, *WHEN
  // maxThickness<INF*,
  //      more isolated sequence groups may arise in the SAME SkeletonGraph; so,
  //      an organized
  //      graph may contain different unconnected basic graph-structures.
  //      Further, remaining circular sequences may still exist. Therefore:

  // Proceed with remaining circulars extraction

  _circulars : {
    // Extract all circular sequences
    // Find a sequence point
    for (i = 0; i < currGraph.getNodesCount(); ++i)
    {
      if (!currGraph.getNode(i).hasAttribute(ORGANIZEGRAPHS_SIGN) &&
          currGraph.getNode(i).degree() == 2) {
        unsigned int curr = i, currLink = 0;
        currSequence.next(curr, currLink);

        for (; curr != i; currSequence.next(curr, currLink))
          currGraph.node(curr).setAttribute(ORGANIZEGRAPHS_SIGN);

        // Add sequence
        currSequence.m_head = currSequence.m_tail = i;
        currSequence.m_headLink                   = 0;
        currSequence.m_tailLink                   = 1;

        globals->singleSequences.push_back(currSequence);
      }
    }
  }
  }
}

//==========================================================================



