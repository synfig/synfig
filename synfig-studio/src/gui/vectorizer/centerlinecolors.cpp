/* === S Y N F I G ========================================================= */
/*!	\file centerlinecolors.cpp
**	\brief centerlinecolors File
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

void calculateSequenceColors(const etl::handle<synfig::Layer_Bitmap> &ras, VectorizerCoreGlobals &g) 
{
  int threshold                           = g.currConfig->m_threshold;
  SequenceList &singleSequences           = g.singleSequences;
  JointSequenceGraphList &organizedGraphs = g.organizedGraphs;

  //TRasterCM32P cm = ras;
  unsigned int i, j, k;
  int l;

  if (cm && g.currConfig->m_maxThickness > 0.0) 
  {
    // singleSequence is traversed back-to-front because new, possibly splitted
    // sequences
    // are inserted at back - and don't have to be re-sampled.
    for (l = singleSequences.size() - 1; l >= 0; --l) 
    {
      Sequence rear;
     // sampleColor(ras, threshold, singleSequences[l], rear, singleSequences);
      // If rear is built, a split occurred and the rear of this
      // single sequence has to be pushed back.
      if (rear.m_graphHolder) singleSequences.push_back(rear);
    }

    for (i = 0; i < organizedGraphs.size(); ++i)
    {
        for (j = 0; j < organizedGraphs[i].getNodesCount(); ++j)
        {
                  // due to junction recovery
        if (!organizedGraphs[i].getNode(j).hasAttribute(JointSequenceGraph::ELIMINATED))
        {
            for (k = 0; k < organizedGraphs[i].getNode(j).getLinksCount(); ++k) 
            {
                Sequence &s = *organizedGraphs[i].node(j).link(k);
                if (s.isForward() && !s.m_graphHolder->getNode(s.m_tail).hasAttribute(SAMPLECOLOR_SIGN))
                {
                    unsigned int next = organizedGraphs[i].node(j).link(k).getNext();
                    unsigned int nextLink = organizedGraphs[i].tailLinkOf(j, k);
                    Sequence &sOpposite = *organizedGraphs[i].node(next).link(nextLink);
                   // sampleColor(cm, threshold, s, sOpposite, singleSequences);
                }
            }
        }  

        }
    }
      
  }
}

//==========================================================================



