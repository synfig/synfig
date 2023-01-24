/* === S Y N F I G ========================================================= */
/*!	\file centerlineadjustments.cpp
**	\brief centerlineadjustments File
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

/* === H E A D E R S ======================================================= */


#include "polygonizerclasses.h"



/* === U S I N G =========================================================== */

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
// class need to be grouped in joints and sequences before proceeding with
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



