/* === S Y N F I G ========================================================= */
/*!	\file action.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "action.h"
#include "instance.h"


#include "actions/layerremove.h"
#include "actions/layermove.h"
#include "actions/layerraise.h"
#include "actions/layerlower.h"
#include "actions/layeradd.h"
#include "actions/layeraddframe.h"
#include "actions/layeractivate.h"
#include "actions/layercopy.h"
#include "actions/layerfit.h"
#include "actions/layermakebline.h"
#include "actions/layerparamset.h"
#include "actions/layerparamsetstatic.h"
#include "actions/layerparamunsetstatic.h"
#include "actions/layerparamconnect.h"
#include "actions/layerparamdisconnect.h"
#include "actions/layerencapsulate.h"
#include "actions/layerencapsulatefilter.h"
#include "actions/layerencapsulateswitch.h"
#include "actions/layerduplicate.h"
#include "actions/layersetdesc.h"
#include "actions/layersetexcludefromrendering.h"
#include "actions/layerzdepthrangeset.h"
#include "actions/layerembed.h"
#include "actions/layerextract.h"
#include "actions/layerresetpose.h"

#include "actions/valuenodeconstset.h"
#include "actions/valuenodeconstsetstatic.h"
#include "actions/valuenodeconstunsetstatic.h"
#include "actions/valuenodeadd.h"
#include "actions/valuenodereplace.h"
#include "actions/valuenodelinkconnect.h"
#include "actions/valuenodelinkdisconnect.h"
#include "actions/valuenodedynamiclistinsert.h"
#include "actions/valuenodedynamiclistremove.h"
#include "actions/valuenodedynamiclistinsertsmart.h"
#include "actions/valuenodedynamiclistremovesmart.h"
#include "actions/valuenodedynamiclistloop.h"
#include "actions/valuenodedynamiclistunloop.h"
#include "actions/valuenodedynamiclistrotateorder.h"
#include "actions/valuenodesetactivebone.h"
#include "actions/valuenodestaticlistinsert.h"
#include "actions/valuenodestaticlistremove.h"
#include "actions/valuenodestaticlistinsertsmart.h"
#include "actions/valuenodestaticlistremovesmart.h"
#include "actions/valuenodestaticlistloop.h"
#include "actions/valuenodestaticlistunloop.h"
#include "actions/valuenodestaticlistrotateorder.h"
#include "actions/valuenoderename.h"
#include "actions/valuenoderemove.h"

#include "actions/valuedescset.h"
#include "actions/valuedescsetinterpolation.h"
#include "actions/valuedescexport.h"
#include "actions/valuedescconvert.h"
#include "actions/valuedescconnect.h"
#include "actions/valuedescdisconnect.h"
#include "actions/valuedesclink.h"
#include "actions/valuedescblinelink.h"
#include "actions/valuedescremovesmart.h"
#include "actions/valuedescbonelink.h"
#include "actions/valuedescskeletonlink.h"
#include "actions/valuedesccreatechildbone.h"
#include "actions/valuedescresetpose.h"
#include "actions/valuedescbake.h"
#include "actions/valuedescbonesetparent.h"
#include "actions/vectorization.h"
#include "actions/waypointadd.h"
#include "actions/waypointset.h"
#include "actions/waypointsetsmart.h"
#include "actions/waypointremove.h"
#include "actions/waypointconnect.h"
#include "actions/waypointdisconnect.h"
// #include "actions/waypointsimpleadd.h"

#include "actions/activepointadd.h"
#include "actions/activepointset.h"
#include "actions/activepointsetsmart.h"
#include "actions/activepointsetoff.h"
#include "actions/activepointseton.h"
#include "actions/activepointremove.h"
// #include "actions/activepointsimpleadd.h"

#include "actions/keyframeadd.h"
#include "actions/keyframeset.h"
#include "actions/keyframeremove.h"
#include "actions/keyframetoggle.h"
#include "actions/keyframeduplicate.h"
#include "actions/keyframewaypointset.h"
#include "actions/keyframesetdelta.h"

#include "actions/timepointsmove.h"
#include "actions/timepointscopy.h"
#include "actions/timepointsdelete.h"

#include "actions/canvasdescriptionset.h"
#include "actions/canvasidset.h"
#include "actions/canvasmetadataerase.h"
#include "actions/canvasmetadataset.h"
#include "actions/canvasnameset.h"
#include "actions/canvasrenddescset.h"
#include "actions/canvasadd.h"
#include "actions/canvasremove.h"

#include "actions/editmodeset.h"

#include "actions/blinepointtangentmerge.h"
#include "actions/blinepointtangentsplit.h"

#include "actions/gradientset.h"
#include "actions/colorset.h"

#include "actions/groupaddlayers.h"
#include "actions/groupremovelayers.h"
#include "actions/groupremove.h"
#include "actions/grouprename.h"

#include "canvasinterface.h"

#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === P R O C E D U R E S ================================================= */

/* === S T A T I C S ======================================================= */

synfigapp::Action::Book *book_;

/* === M E T H O D S ======================================================= */

#define ADD_ACTION(x) { BookEntry &be(book()[x::name__]); \
	be.name=x::name__; \
	be.local_name=_(x::local_name__); \
	be.version=x::version__; \
	be.task=x::task__; \
	be.priority=x::priority__; \
	be.category=x::category__; \
	be.factory=x::create; \
	be.get_param_vocab=x::get_param_vocab; \
	be.is_candidate=x::is_candidate; \
	}


Action::Main::Main()
{
	book_=new synfigapp::Action::Book();

	ADD_ACTION(Action::LayerRemove);
	ADD_ACTION(Action::LayerMove);
	ADD_ACTION(Action::LayerRaise);
	ADD_ACTION(Action::LayerLower);
	ADD_ACTION(Action::LayerAdd);
	ADD_ACTION(Action::LayerActivate);
	ADD_ACTION(Action::LayerFit);
	ADD_ACTION(Action::LayerMakeOutline);
	ADD_ACTION(Action::LayerMakeAdvancedOutline);
	ADD_ACTION(Action::LayerMakeRegion);
	auto layer_iter = synfig::Layer::book().find("curve_gradient");
	if (layer_iter != synfig::Layer::book().end())
		ADD_ACTION(Action::LayerMakeCurveGradient)
	layer_iter = synfig::Layer::book().find("plant");
	if (layer_iter != synfig::Layer::book().end())
		ADD_ACTION(Action::LayerMakePlant)
	ADD_ACTION(Action::LayerParamSet);
	ADD_ACTION(Action::LayerParamSetStatic);
	ADD_ACTION(Action::LayerParamUnSetStatic);
	ADD_ACTION(Action::LayerParamConnect);
	ADD_ACTION(Action::LayerParamDisconnect);
	ADD_ACTION(Action::LayerEncapsulate);
	ADD_ACTION(Action::LayerEncapsulateFilter);
	ADD_ACTION(Action::LayerEncapsulateSwitch);
	ADD_ACTION(Action::LayerDuplicate);
	ADD_ACTION(Action::LayerSetDesc);
	ADD_ACTION(Action::LayerZDepthRangeSet);
	ADD_ACTION(Action::LayerEmbed);
	ADD_ACTION(Action::LayerExtract);
	ADD_ACTION(Action::LayerSetExcludeFromRenderingOn);
	ADD_ACTION(Action::LayerSetExcludeFromRenderingOff);
	ADD_ACTION(Action::LayerAddFrame);
	ADD_ACTION(Action::LayerCopy);
	ADD_ACTION(Action::LayerResetPose);

	ADD_ACTION(Action::ValueNodeConstSet);
	ADD_ACTION(Action::ValueNodeConstSetStatic);
	ADD_ACTION(Action::ValueNodeConstUnSetStatic);
	ADD_ACTION(Action::ValueNodeAdd);
	ADD_ACTION(Action::ValueNodeReplace);
	ADD_ACTION(Action::ValueNodeLinkConnect);
	ADD_ACTION(Action::ValueNodeLinkDisconnect);
	ADD_ACTION(Action::ValueNodeDynamicListInsert);
	ADD_ACTION(Action::ValueNodeDynamicListRemove);
	ADD_ACTION(Action::ValueNodeDynamicListInsertSmart);
	ADD_ACTION(Action::ValueNodeDynamicListInsertSmartKeepShape);
	ADD_ACTION(Action::ValueNodeDynamicListRemoveSmart);
	ADD_ACTION(Action::ValueNodeDynamicListLoop);
	ADD_ACTION(Action::ValueNodeDynamicListUnLoop);
	ADD_ACTION(Action::ValueNodeDynamicListRotateOrder);
	ADD_ACTION(Action::ValueNodeSetActiveBone);
	ADD_ACTION(Action::ValueNodeStaticListInsert);
	ADD_ACTION(Action::ValueNodeStaticListRemove);
	ADD_ACTION(Action::ValueNodeStaticListInsertSmart);
	ADD_ACTION(Action::ValueNodeStaticListRemoveSmart);
	ADD_ACTION(Action::ValueNodeStaticListLoop);
	ADD_ACTION(Action::ValueNodeStaticListUnLoop);
	ADD_ACTION(Action::ValueNodeStaticListRotateOrder);
	ADD_ACTION(Action::ValueNodeRename);
	ADD_ACTION(Action::ValueNodeRemove);

	ADD_ACTION(Action::ValueDescSet);
	ADD_ACTION(Action::ValueDescSetInterpolation);
	ADD_ACTION(Action::ValueDescExport);
	ADD_ACTION(Action::ValueDescConvert);
	ADD_ACTION(Action::ValueDescConnect);
	ADD_ACTION(Action::ValueDescDisconnect);
	ADD_ACTION(Action::ValueDescLink);
	ADD_ACTION(Action::ValueDescLinkOpposite);
	ADD_ACTION(Action::ValueDescBLineLink);
	ADD_ACTION(Action::ValueDescRemoveSmart);
	ADD_ACTION(Action::ValueDescBoneLink);
	ADD_ACTION(Action::ValueDescSkeletonLink);
	ADD_ACTION(Action::ValueDescCreateChildBone);
	ADD_ACTION(Action::ValueDescBoneSetParent);
	ADD_ACTION(Action::ValueDescResetPose);
	ADD_ACTION(Action::ValueDescBake);
	ADD_ACTION(Action::Vectorization);
	ADD_ACTION(Action::WaypointAdd);
	ADD_ACTION(Action::WaypointSet);
	ADD_ACTION(Action::WaypointSetSmart);
	ADD_ACTION(Action::WaypointRemove);
	ADD_ACTION(Action::WaypointConnect);
	ADD_ACTION(Action::WaypointDisconnect);
//	ADD_ACTION(Action::WaypointSimpleAdd);

	ADD_ACTION(Action::ActivepointAdd);
	ADD_ACTION(Action::ActivepointSet);
	ADD_ACTION(Action::ActivepointSetSmart);
	ADD_ACTION(Action::ActivepointSetOn);
	ADD_ACTION(Action::ActivepointSetOff);
	ADD_ACTION(Action::ActivepointRemove);
//	ADD_ACTION(Action::ActivepointSimpleAdd);

	ADD_ACTION(Action::KeyframeAdd);
	ADD_ACTION(Action::KeyframeSet);
	ADD_ACTION(Action::KeyframeRemove);
	ADD_ACTION(Action::KeyframeToggl);
	ADD_ACTION(Action::KeyframeDuplicate);
	ADD_ACTION(Action::KeyframeWaypointSet);
	ADD_ACTION(Action::KeyframeSetDelta);

	ADD_ACTION(Action::CanvasDescriptionSet);
	ADD_ACTION(Action::CanvasIdSet);
	ADD_ACTION(Action::CanvasNameSet)
	ADD_ACTION(Action::CanvasMetadataErase);
	ADD_ACTION(Action::CanvasMetadataSet);
	ADD_ACTION(Action::CanvasRendDescSet);
	ADD_ACTION(Action::CanvasAdd);
	ADD_ACTION(Action::CanvasRemove);

	ADD_ACTION(Action::EditModeSet);

	ADD_ACTION(Action::BLinePointTangentMerge);
	ADD_ACTION(Action::BLinePointTangentMergeRadius);
	ADD_ACTION(Action::BLinePointTangentMergeAngle);
	ADD_ACTION(Action::BLinePointTangentSplit);
	ADD_ACTION(Action::BLinePointTangentSplitRadius);
	ADD_ACTION(Action::BLinePointTangentSplitAngle);

	ADD_ACTION(Action::GradientSet);
	ADD_ACTION(Action::ColorSetFromOC);
	ADD_ACTION(Action::ColorSetFromFC);

	ADD_ACTION(Action::TimepointsMove);
	ADD_ACTION(Action::TimepointsCopy);
	ADD_ACTION(Action::TimepointsDelete);

	ADD_ACTION(Action::GroupAddLayers);
	ADD_ACTION(Action::GroupRemoveLayers);
	ADD_ACTION(Action::GroupRemove);
	ADD_ACTION(Action::GroupRename);
}

Action::Main::~Main()
{
	delete book_;
}


Action::Book& Action::book() { return *book_; }


Action::Handle
Action::create(const synfig::String &name)
{
	if(!book().count(name))
		return 0; //! \todo perhaps we should throw something instead?
	return book()[name].factory();
}


Action::CandidateList
Action::compile_candidate_list(const ParamList& param_list, Category category)
{
	Action::CandidateList ret;

	Book::const_iterator iter;

	//synfig::info("param_list.size()=%d",param_list.size());

	for(iter=book().begin();iter!=book().end();++iter)
	{
		if((iter->second.category&category))
		{
			if(iter->second.is_candidate(param_list))
			{
				//synfig::info("Action \"%s\" is in",iter->second.name.c_str());
				ret.push_back(iter->second);
			}
			else
			{
				//synfig::info("Action \"%s\" is not a candidate",iter->second.name.c_str());
			}
		}
		else
		{
			//synfig::info("Action \"%s\" has unsuitable category",iter->second.name.c_str());
		}
	}

	return ret;
}

Action::CandidateList
Action::compile_visible_candidate_list(const ParamList& param_list, Category category)
{
	Action::CandidateList ret;

	for (const auto& item : book()) {
		const auto& entry = item.second;
		if (entry.category & category && !(entry.category & Action::CATEGORY_HIDDEN)) {
			if (entry.is_candidate(param_list)) {
				ret.push_back(entry);
			}
		}
	}

	return ret;
}

Action::CandidateList::iterator
Action::CandidateList::find(const synfig::String& x)
{
	iterator iter;
	for(iter=begin();iter!=end();++iter)
		if(iter->name==x)
			break;
	return iter;
}

void
Action::Base::set_param_list(const ParamList &param_list)
{
	ParamList::const_iterator iter;

	for(iter=param_list.begin();iter!=param_list.end();++iter)
		set_param(iter->first,iter->second);
}

synfig::String
Action::Base::get_layer_descriptions(const std::list<synfig::Layer::Handle> layers, synfig::String singular_prefix, synfig::String plural_prefix)
{
	String ret;
	bool first = true;

	if (plural_prefix.empty())
		plural_prefix = singular_prefix;

	if (layers.empty())
		return plural_prefix;

	if (layers.size() == 1)
		ret = singular_prefix;
	else
		ret = plural_prefix;

	if (!ret.empty())
		ret.push_back(' ');

	for(std::list<synfig::Layer::Handle>::const_iterator iter=layers.begin(); iter!=layers.end(); ++iter)
	{
		if (first)
			first = false;
		else
			ret += ", ";

		ret += strprintf("'%s'", (*iter)->get_non_empty_description().c_str());
	}

	return ret;
}

synfig::String
Action::Base::get_layer_descriptions(const std::list<std::pair<synfig::Layer::Handle,int> > layers, synfig::String singular_prefix, synfig::String plural_prefix)
{
	String ret;
	bool first = true;

	if (plural_prefix.empty())
		plural_prefix = singular_prefix;

	if (layers.empty())
		return plural_prefix;

	if (layers.size() == 1)
		ret = singular_prefix;
	else
		ret = plural_prefix;

	if (!ret.empty())
		ret.push_back(' ');

	for(std::list<std::pair<synfig::Layer::Handle,int> >::const_iterator iter=layers.begin(); iter!=layers.end(); ++iter)
	{
		if (first)
			first = false;
		else
			ret += ", ";

		ret += strprintf("'%s'", iter->first->get_non_empty_description().c_str());
	}

	return ret;
}

void
Super::perform()
{
	set_dirty(false);

	prepare();

	ActionList::const_iterator iter;
	for(iter=action_list_.begin();iter!=action_list_.end();++iter)
	{
		DEBUG_LOG("SYNFIG_DEBUG_ACTIONS",
			"%s:%d action: '%s'", __FILE__, __LINE__, (*iter)->get_name().c_str());

		try
		{
			try
			{
				(*iter)->perform();
				CanvasSpecific* canvas_specific(dynamic_cast<CanvasSpecific*>(iter->get()));
				if(canvas_specific && canvas_specific->is_dirty())
					set_dirty(true);
			}
			catch(...)
			{
				if(iter!=action_list_.begin())
				{
					for(--iter;iter!=action_list_.begin();--iter)
						(*iter)->undo();
					(*iter)->undo();
				}
				throw;
			}
		}
		catch(Error& x)
		{
			//synfig::info("%s:%d caught exception", __FILE__, __LINE__);
			throw Error(x.get_type(),((*iter)->get_local_name()+": "+x.get_desc()).c_str());
		}
		//synfig::info("%s:%d finished action: '%s'", __FILE__, __LINE__, (*iter)->get_name().c_str());
	}
}

void
Super::undo()
{
	set_dirty(false);

	ActionList::const_reverse_iterator iter;
	for(iter=const_cast<const ActionList &>(action_list_).rbegin();iter!=const_cast<const ActionList &>(action_list_).rend();++iter)
	{
		try {
			(*iter)->undo();
			CanvasSpecific* canvas_specific(dynamic_cast<CanvasSpecific*>(iter->get()));
			if(canvas_specific && canvas_specific->is_dirty())
				set_dirty(true);
		}
		catch(...)
		{
			if(iter!=const_cast<const ActionList &>(action_list_).rbegin())
			{
				for(--iter;iter!=const_cast<const ActionList &>(action_list_).rbegin();--iter)
					(*iter)->perform();
				(*iter)->perform();
			}
			throw;
		}
	}
}

void
Super::add_action(Undoable::Handle action)
{
	action_list_.push_back(action);
	CanvasSpecific *specific_action=dynamic_cast<CanvasSpecific *>(action.get());
	if(specific_action && !get_canvas())
		set_canvas(specific_action->get_canvas());
}

void
Super::add_action_front(Undoable::Handle action)
{
	assert(action);
	action_list_.push_front(action);
	CanvasSpecific *specific_action=dynamic_cast<CanvasSpecific *>(action.get());
	if(specific_action && !get_canvas())
		set_canvas(specific_action->get_canvas());
}


Group::Group(const synfig::String &str):
	name_(str),
	ready_(true)
{
}

Group::~Group()
{
}




Action::ParamVocab
Action::CanvasSpecific::get_param_vocab()
{
	ParamVocab ret;

	ret.push_back(ParamDesc("canvas",Param::TYPE_CANVAS)
		.set_local_name(_("Canvas"))
		.set_desc(_("Selected Canvas"))
	);

	ret.push_back(ParamDesc("canvas_interface",Param::TYPE_CANVASINTERFACE)
		.set_local_name(_("Canvas Interface"))
		.set_desc(_("Canvas Interface"))
		.set_optional(true)
	);


	return ret;
}

bool
CanvasSpecific::set_param(const synfig::String& name, const Param &param)
{
	if(name=="canvas" && param.get_type()==Param::TYPE_CANVAS)
	{
		if(!param.get_canvas())
			return false;
		set_canvas(param.get_canvas());

		return true;
	}
	if(name=="canvas_interface" && param.get_type()==Param::TYPE_CANVASINTERFACE)
	{
		if(!param.get_canvas_interface())
			return false;
		set_canvas_interface(param.get_canvas_interface());
		if(!get_canvas())
			set_canvas(get_canvas_interface()->get_canvas());

		return true;
	}
	if(name=="edit_mode" && param.get_type()==Param::TYPE_EDITMODE)
	{
		set_edit_mode(param.get_edit_mode());

		return true;
	}

	return false;
}

bool
CanvasSpecific::is_ready()const
{
	if(!get_canvas())
		return false;
	return true;
}

EditMode
CanvasSpecific::get_edit_mode()const
{
	if(mode_!=MODE_UNDEFINED)
		return mode_;

	if(get_canvas_interface())
		return get_canvas_interface()->get_mode();

	return MODE_NORMAL;
}

//DOO static int undoable_count = 0;

Undoable::Undoable():
	active_(true)
{
	//DOO printf("%s:%d Undoable::Undoable() (we have %d)\n", __FILE__, __LINE__, ++undoable_count);
}

#ifdef _DEBUG
Undoable::~Undoable() {
	//DOO printf("%s:%d Undoable::~Undoable() (we now have %d)\n", __FILE__, __LINE__, --undoable_count);
}

void
Undoable::ref() const noexcept
{
	DEBUG_LOG("SYNFIG_DEBUG_ACTION_REFCOUNT",
		"%s:%d %lx   ref undoable %*s -> %2d\n", __FILE__, __LINE__, uintptr_t(this), (count()*2), "", count()+1);

	Base::ref();
}

void
Undoable::unref()const
{
	DEBUG_LOG("SYNFIG_DEBUG_ACTION_REFCOUNT",
		"%s:%d %lx unref undoable %*s%2d <-\n", __FILE__, __LINE__, uintptr_t(this), ((count()-1)*2), "", count()-1);

	Base::unref();
}
#endif
