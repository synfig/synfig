/* === S Y N F I G ========================================================= */
/*!	\file vectorization.cpp
**	
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
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

#include <synfig/general.h>
#include <synfig/layer.h>
#include <ETL/handle>
#include "vectorization.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/main.h>
#include <synfigapp/localization.h>
#include <synfig/layers/layer_bitmap.h>
#include <synfig/layers/layer_pastecanvas.h>


#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::Vectorization);
ACTION_SET_NAME(Action::Vectorization,"Vectorization");
ACTION_SET_LOCAL_NAME(Action::Vectorization,N_("Vectorize the Image"));
ACTION_SET_TASK(Action::Vectorization,"vectorize");
ACTION_SET_CATEGORY(Action::Vectorization,Action::CATEGORY_LAYER);
ACTION_SET_PRIORITY(Action::Vectorization,0);
ACTION_SET_VERSION(Action::Vectorization,"0.0");
ACTION_SET_CVS_ID(Action::Vectorization,"$Id$");

/* === G L O B A L S ======================================================= */
/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */
Vectorization::Vectorization()
{
    isOutline = false;
    new_layer = synfig::Layer::create("group");

}

// this function return centerline configuration after setting the config using params
studio::CenterlineConfiguration Vectorization::getCenterlineConfiguration( ) const 
{
  studio::CenterlineConfiguration conf;

  conf.m_outline      = false;
  conf.m_threshold    = threshold;
  conf.m_penalty      = penalty;  // adjustment_accuracy in [1,10]
  conf.m_despeckling  = despeckling;
  conf.m_maxThickness = maxthickness;
  conf.m_thicknessRatio = 1.0;
  conf.m_leaveUnpainted = pparea;
  conf.m_makeFrame      = addborder;
  conf.m_naaSource      = false;//currently not in use

  return conf;
}

// this function return outline configuration after setting the config using params
studio::NewOutlineConfiguration Vectorization::getOutlineConfiguration() const 
{
  studio::NewOutlineConfiguration conf;

   conf.m_outline          = true;
//   conf.m_despeckling      = m_oDespeckling;
//   conf.m_adherenceTol     = m_oAdherence * 0.01;
//   conf.m_angleTol         = m_oAngle / 180.0;
//   conf.m_relativeTol      = m_oRelative * 0.01;
//   conf.m_mergeTol         = 5.0 - m_oAccuracy * 0.5;
//   conf.m_leaveUnpainted   = !m_oPaintFill;
//   conf.m_maxColors        = m_oMaxColors;
//   conf.m_transparentColor = m_oTransparentColor;
//   conf.m_toneTol          = m_oToneThreshold;

  return conf;
}

Action::ParamVocab
Action::Vectorization::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("image",Param::TYPE_LAYER)
		.set_local_name(_("Image Layer"))
	);

	ret.push_back(ParamDesc("mode",Param::TYPE_STRING)
		.set_local_name(_("Vectorization mode"))
		.set_desc(_("Mode for Vectorization"))
	);
    ret.push_back(ParamDesc("threshold", Param::TYPE_INTEGER)
		.set_local_name(_("Threshold Value"))
		.set_desc(_("Mode for Vectorization"))
	);
    ret.push_back(ParamDesc("penalty", Param::TYPE_INTEGER)
		.set_local_name(_("Penalty"))
		.set_desc(_("Penalty based on accuracy"))
	);
    ret.push_back(ParamDesc("despeckling", Param::TYPE_INTEGER)
		.set_local_name(_("Despeckling value"))
		.set_desc(_("Despeckling Value for process"))
	);
    ret.push_back(ParamDesc("maxthickness", Param::TYPE_INTEGER)
		.set_local_name(_("Max thickness"))
		.set_desc(_("Max thickness of outline"))
	);
    ret.push_back(ParamDesc("pparea", Param::TYPE_BOOL)
		.set_local_name(_("Preserve painted area"))
		.set_desc(_("To preserve painted area"))
	);
    ret.push_back(ParamDesc("addborder", Param::TYPE_BOOL)
		.set_local_name(_("Add border"))
		.set_desc(_("Add border in final outlines"))
	);
    
    return ret;
}

bool
Action::Vectorization::is_candidate(const ParamList &x)
{
	return (candidate_check(get_param_vocab(),x) &&
			synfig::Layer_Bitmap::Handle::cast_dynamic(x.find("image")->second.get_value_node()));
}

bool
Action::Vectorization::is_ready() const
{
    return get_param_vocab().size() >= 8 && 
        synfig::Layer_Bitmap::Handle::cast_dynamic(layer) && 
        Action::CanvasSpecific::is_ready();
 }

bool
Action::Vectorization::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="image" && param.get_type()==Param::TYPE_LAYER)
	{
        layer = param.get_layer();
		return true;	
    }
    if(name=="mode" && param.get_type() == Param::TYPE_STRING)
    {
        v_mode = param.get_string();
        if(v_mode=="outline"||v_mode == "Outline")
        isOutline=true;
        return true;
    }
    if(name=="threshold" && param.get_type() == Param::TYPE_INTEGER)
    {
        threshold = param.get_integer();
        return true;
    }
    if(name=="penalty" && param.get_type() == Param::TYPE_INTEGER)
    {
        penalty = param.get_integer();
        return true;
    }
    if(name=="despeckling" && param.get_type() == Param::TYPE_INTEGER)
    {
        despeckling = param.get_integer();
        return true;
    }
    if(name=="maxthickness" && param.get_type() == Param::TYPE_INTEGER)
    {
        maxthickness = param.get_integer();
        std::cout<<"vectorization action maxthickness :"<<maxthickness<<"\n";
        return true;
    }
    if(name=="pparea" && param.get_type() == Param::TYPE_BOOL)
    {
        pparea = param.get_bool();
        return true;
    }
    if(name=="addborder" && param.get_type() == Param::TYPE_BOOL)
    {
        addborder = param.get_bool();
        return true;
    }
    if(name=="reference_layer" && param.get_type() == Param::TYPE_LAYER)
    {
        reference_layer = param.get_layer();
        return true;
    }
    return Action::CanvasSpecific::set_param(name,param);
}

void
Action::Vectorization::perform()
{
    studio::CenterlineConfiguration m_cConf;
  	studio::NewOutlineConfiguration m_oConf;

    // set the configuration to pass to vectorizer
	studio::VectorizerConfiguration &configuration = isOutline ? static_cast<studio::VectorizerConfiguration &>(m_oConf)
        									  : static_cast<studio::VectorizerConfiguration &>(m_cConf);

    // set config to centerline or outline as per v_mode
    if (v_mode=="outline"||v_mode == "Outline")
        m_oConf = getOutlineConfiguration();
    else if(v_mode=="centerline"||v_mode=="Centerline")
        m_cConf = getCenterlineConfiguration();

    studio::VectorizerCore vCore;
    synfig::Layer_Bitmap::Handle image_layer = synfig::Layer_Bitmap::Handle::cast_dynamic(layer);

    // result of vectorization (vector of outline layers)
    std::vector< etl::handle<synfig::Layer> > Result = vCore.vectorize(image_layer, configuration);

    synfig::Canvas::Handle child_canvas;
    child_canvas=synfig::Canvas::create_inline(layer->get_canvas());

    new_layer->set_description("Vectorized "+layer->get_description());
	new_layer->set_param("canvas",child_canvas);
    int move_depth = 0;

    // if the vectorizer option was selected for switch group
    if(etl::handle<synfig::Layer_PasteCanvas> paste = etl::handle<synfig::Layer_PasteCanvas>::cast_dynamic(reference_layer))
    {
        // apply the transformation from switch group to result group
        new_layer->set_param("transformation",paste->get_param("transformation"));
        auto iter = std::find(get_canvas()->begin(), get_canvas()->end(), reference_layer);
        get_canvas()->insert(iter,new_layer);
        // how much layer is moved if there are layer above it in the layer panel
        move_depth = std::distance(get_canvas()->begin(), iter);
        
    }
    //synfig(17889) [11:06:00  IST] error: LayerTreeStore::on_layer_inserted():Unable to achieve desired depth, forced to rebuild...

    else
    {
        auto iter = std::find(get_canvas()->begin(), get_canvas()->end(), layer);
        get_canvas()->insert(iter,new_layer);
        move_depth = std::distance(get_canvas()->begin(), iter);
    }
    new_layer->set_canvas(get_canvas());
    for(int i=0;i < Result.size();i++)
    {
      Result[i]->set_canvas(child_canvas);
      child_canvas->push_front(Result[i]);
    }
    if(get_canvas_interface()) 
    { 
 	    get_canvas_interface()->signal_layer_inserted()(new_layer,0); 
        get_canvas_interface()->signal_layer_moved()(new_layer,move_depth-1,get_canvas());
    } 

}

void
Action::Vectorization::undo()
{
    // Find the iterator for the layer
	Canvas::iterator iter=find(get_canvas()->begin(),get_canvas()->end(),new_layer);

	if(*iter!=new_layer)
		throw Error(_("This layer doesn't exist anymore."));

	// Remove the layer from the canvas
	get_canvas()->erase(iter);

	// Signal that a layer has been inserted
	if(get_canvas_interface())
	{
		get_canvas_interface()->signal_layer_removed()(new_layer);
	}
	else synfig::warning("CanvasInterface not set on action");

}
