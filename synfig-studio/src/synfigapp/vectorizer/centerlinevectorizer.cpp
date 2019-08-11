/* === S Y N F I G ========================================================= */
/*!	\file centerlinevectorizer.cpp
**	\brief This is the entry point of centerline vectorization
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

#include "centerlinevectorizer.h"
#include "polygonizerclasses.h"
#include <synfig/layer.h>
#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

inline void deleteSkeletonList(SkeletonList *skeleton) {
  unsigned int i;
  for (i = 0; i < skeleton->size(); ++i) delete (*skeleton)[i];

  delete skeleton;
}


/* === E N T R Y P O I N T ================================================= */

//************************
//*    Vectorizer Main   *
//************************

// takes two arguments ( image layer handle, config )
std::vector< etl::handle<synfig::Layer> > VectorizerCore::centerlineVectorize(etl::handle<synfig::Layer_Bitmap> &image, const CenterlineConfiguration &configuration)
 {
  std::cout<<"Inside CenterlineVectorize\n";
  VectorizerCoreGlobals globals;
  globals.currConfig = &configuration;

  // step 2 
  // Extracts a polygonal, minimal yet faithful representation of image contours
  Contours polygons;
  studio::polygonize(image, polygons, globals);
//   for (int i = 0; i < polygons.size(); ++i) 
//   {
//     for (int j = 0; j < polygons[i].size(); ++j) 
//     {
//       for (int k = 0; k < polygons[i][j].size(); ++k)
//       {
//       std::cout<<"i= "<<i<<" j= "<<j<<" k="<<k<<" Data: ";
//       std::cout<< "[(" << polygons[i][j][k].m_position[0]<<", "<<polygons[i][j][k].m_position[1]<<", "<<polygons[i][j][k].m_position[2] <<")("
//       << polygons[i][j][k].m_direction[0]<<", "<<polygons[i][j][k].m_direction[1]<<", "<<polygons[i][j][k].m_direction[2] <<")("
//       << polygons[i][j][k].m_AngularMomentum[0]<<", "<<polygons[i][j][k].m_AngularMomentum[1]<<", "<<polygons[i][j][k].m_AngularMomentum[2] <<")("
//       << polygons[i][j][k].m_AuxiliaryMomentum1[0]<<", "<<polygons[i][j][k].m_AuxiliaryMomentum1[1]<<", "<<polygons[i][j][k].m_AuxiliaryMomentum1[2] <<")("
//       << polygons[i][j][k].m_AuxiliaryMomentum2[0]<<", "<<polygons[i][j][k].m_AuxiliaryMomentum2[1]<<", "<<polygons[i][j][k].m_AuxiliaryMomentum2[2] <<")"
//       <<" Concave: "<< polygons[i][j][k].m_concave << " Attr: "
//       << polygons[i][j][k].m_attributes <<" Time: "<< polygons[i][j][k].m_updateTime
//       <<" Ancestor"<< polygons[i][j][k].m_ancestor 
//       <<" AncC: "<< polygons[i][j][k].m_ancestorContour << "]\n";
//       }
//     }
// }
  // step 3
  // The process of skeletonization reduces all objects in an image to lines, 
  //  without changing the essential structure of the image.

  // Most time-consuming part of vectorization, 'this' is passed to inform of
  // partial progresses
  SkeletonList *skeletons = studio::skeletonize(polygons, this, globals);

  // for (SkeletonList::iterator currGraphPtr = skeletons->begin(); currGraphPtr != skeletons->end(); ++currGraphPtr) 
  // {
  //   std::cout<<"Skeleton data\n";
  //   SkeletonGraph &currGraph   = **currGraphPtr;
  //   std::cout<<"Link Count :"<<currGraph.getLinksCount()<<"Node Count :"<<currGraph.getNodesCount()<<"\n";
  //   for (int i = 0; i < currGraph.getNodesCount(); ++i)
  //   {

  //     std::cout<<"m_content :"<<currGraph.getNode(i)->operator[](0) <<", "<<currGraph.getNode(i)->operator[](1)<<", "
  //     <<currGraph.getNode(i)->operator[](2)<<"\n";

  //   }
  // }

  if (isCanceled()) 
  {
    // Clean and return 0 at cancel command
    deleteSkeletonList(skeletons);
    std::cout<<"CenterlineVectorize cancelled\n";
  }

  // step 4
  // The raw skeleton data obtained from StraightSkeletonizer
  // class need to be grouped in joints and sequences before proceding further
  studio::organizeGraphs(skeletons, globals);

//   // junctionRecovery(polygons);   //Da' problemi per maxThickness<inf...
//   // sarebbe da rendere compatibile

  std::vector< etl::handle<synfig::Layer> > sortibleResult;
//   TVectorImageP result;
  
  // step 5
  // Take samples of image colors to associate each sequence to its corresponding
  // palette color
  //studio::calculateSequenceColors(image, globals);  // Extract stroke colors here

//   // step 6
//   // Converts each forward or single Sequence of the image in its corresponding Stroke.
  studio::conversionToStrokes(sortibleResult, globals, image);
  deleteSkeletonList(skeletons);
  return sortibleResult;
}
std::vector< etl::handle<synfig::Layer> > VectorizerCore::vectorize(const etl::handle<synfig::Layer_Bitmap> &img, const VectorizerConfiguration &c) 
{
  std::vector< etl::handle<synfig::Layer> > result;

  if (c.m_outline)
  {
    std::cout<<"newOutlineVectorize called/n";
    // vi = newOutlineVectorize(img, static_cast<const NewOutlineConfiguration &>(c), plt);
    return result;
  }
  else 
  {
    Handle img2(img);
    result = centerlineVectorize(img2, static_cast<const CenterlineConfiguration &>(c));
    std::cout<<"After centerlineVectorize result.size(): "<<result.size()<<"\n";
    return result;
    
  }

}
