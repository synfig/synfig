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
ACTION_SET_CATEGORY(Action::Vectorization,Action::CATEGORY_LAYER|Action::CATEGORY_VALUENODE);
ACTION_SET_PRIORITY(Action::Vectorization,0);
ACTION_SET_VERSION(Action::Vectorization,"0.0");
ACTION_SET_CVS_ID(Action::Vectorization,"$Id$");

/* === G L O B A L S ======================================================= */
bool isOutline = false;
/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */
CenterlineConfiguration VectorizerSettings::getCenterlineConfiguration( ) const 
{
  CenterlineConfiguration conf;

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

NewOutlineConfiguration VectorizerSettings::getOutlineConfiguration(
    double frame) const {
  NewOutlineConfiguration conf;

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

bool
Action::Vectorization::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="image" && param.get_type()==Param::TYPE_LAYER)
	{
        layer = param.get_layer();
		return true;	
    }
    if(name=="mode" && param.get_type == Param::TYPE_STRING)
    {
        v_mode = param.get_string();
        if(v_mode=="outline"||v_mode=="Outline")
        isOutline=true;
        return true;
    }
    if(name=="threshold" && param.get_type == Param::TYPE_INTEGER)
    {
        threshold = param.get_integer();
        return true;
    }
    if(name=="penalty" && param.get_type == Param::TYPE_INTEGER)
    {
        penalty = param.get_integer();
        return true;
    }
    if(name=="despeckling" && param.get_type == Param::TYPE_INTEGER)
    {
        despeckling = param.get_integer();
        return true;
    }
    if(name=="maxthickness" && param.get_type == Param::TYPE_INTEGER)
    {
        maxthickness = param.get_integer();
        return true;
    }
    if(name=="pparea" && param.get_type == Param::TYPE_BOOL)
    {
        pparea = param.get_bool();
        return true;
    }
    if(name=="addborder" && param.get_type == Param::TYPE_BOOL)
    {
        addborder = param.get_bool();
        return true;
    }
    return Action::CanvasSpecific::set_param(name,param);
}

void
Action::Vectorization::perform()
{
    studio::CenterlineConfiguration m_cConf;
  	studio::NewOutlineConfiguration m_oConf;
	studio::VectorizerConfiguration &configuration = isOutline ? static_cast<VectorizerConfiguration &>(m_oConf)
        									  : static_cast<VectorizerConfiguration &>(m_cConf);

    if (v_mode=="outline"||v_mode == "Outline")
        m_oConf = getOutlineConfiguration(0.0);
    else if(v_mode=="centerline"||v_mode=="Centerline")
        m_cConf = getCenterlineConfiguration();

    studio::VectorizerCore vCore;
    std::vector< etl::handle<synfig::Layer> > Result = 
    vCore.vectorize(layer, configuration);//Todo change function to return outlines
}
