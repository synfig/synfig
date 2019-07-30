/* === S Y N F I G ========================================================= */
/*!	\file template.cpp
**	\brief Template File
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
// #include "modules/mod_geometry/outline.h"
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

// takes three arguments ( TimageP, config and palette )
std::vector< etl::handle<synfig::Layer> > VectorizerCore::centerlineVectorize(etl::handle<synfig::Layer_Bitmap> &image, const CenterlineConfiguration &configuration)
 {
  std::cout<<"Inside CenterlineVectorize\n";
  VectorizerCoreGlobals globals;
  globals.currConfig = &configuration;

  // step 2 
  // Extracts a polygonal, minimal yet faithful representation of image contours
  Contours polygons;
  studio::polygonize(image, polygons, globals);
  
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

  return sortibleResult;

	// synfig::Canvas::Handle child_canvas;
  // child_canvas=synfig::Canvas::create_inline(image->get_canvas());
	// synfig::Layer::Handle new_layer(synfig::Layer::create("group"));
  // new_layer->set_description("description");
	// new_layer->set_param("canvas",child_canvas);
  // image->get_canvas()->parent()->push_front(new_layer);
  // for(int i=0;i < sortibleResult.size();i++)
  // {
  //     sortibleResult[i]->set_canvas(child_canvas);
  //     child_canvas->push_front(sortibleResult[i]);
  // }
//   // step 7

//   // Take samples of image colors to associate each stroke to its corresponding
//   // palette color.
//   applyStrokeColors(sortibleResult, ras, palette,
//                     globals);  // Strokes get sorted here
  
//   // step 8
//   // copy/apply the strokes to an empty vector image
//   // and return the vector image after further adjustments
//   result = copyStrokes(sortibleResult);

//   // Further misc adjustments
//   if (globals.currConfig->m_thicknessRatio < 100)
//     reduceThickness(result, configuration.m_thicknessRatio);
//   if (globals.currConfig->m_maxThickness == 0.0)
//     for (unsigned int i = 0; i < result->getStrokeCount(); ++i)
//       result->getStroke(i)->setSelfLoop(true);
//   if (globals.currConfig->m_makeFrame) addFrameStrokes(result, ras, palette);
//   // randomizeExtremities(result);   //Cuccio random - non serve...

//   deleteSkeletonList(skeletons);

//   return result;
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
    return result;
    // if (vi) {
    //   for (int i = 0; i < (int)vi->getStrokeCount(); ++i) {
    //     TStroke *stroke = vi->getStroke(i);

    //     for (int j = 0; j < stroke->getControlPointCount(); ++j) {
    //       TThickPoint p = stroke->getControlPoint(j);
    //       p             = TThickPoint(c.m_affine * p, c.m_thickScale * p.thick);

    //       stroke->setControlPoint(j, p);
    //     }
    //   }

    //   applyFillColors(vi, img2, plt, c);
    // }
  }

}
