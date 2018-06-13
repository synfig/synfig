/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/renderersw.cpp
**	\brief RendererSW
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <synfig/localization.h>

#include "renderersw.h"

#include  "task/tasksw.h"

#include "../common/optimizer/optimizerblendassociative.h"
#include "../common/optimizer/optimizerblendmerge.h"
#include "../common/optimizer/optimizerblendtotarget.h"
#include "../common/optimizer/optimizerblendseparate.h"
#include "../common/optimizer/optimizerblendsplit.h"
#include "../common/optimizer/optimizerblendzero.h"
#include "../common/optimizer/optimizerlist.h"
#include "../common/optimizer/optimizerpixelprocessorsplit.h"
#include "../common/optimizer/optimizersplit.h"
#include "../common/optimizer/optimizertransformation.h"

#include "function/fft.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

RendererSW::RendererSW()
{
	register_mode(TaskSW::mode_token.handle());

	// register optimizers
	register_optimizer(new OptimizerTransformation());

	register_optimizer(new OptimizerBlendZero());
	register_optimizer(new OptimizerBlendMerge());
	register_optimizer(new OptimizerBlendToTarget());
	register_optimizer(new OptimizerList());
	register_optimizer(new OptimizerBlendAssociative());
	register_optimizer(new OptimizerBlendSeparate());
	register_optimizer(new OptimizerBlendSplit());
	register_optimizer(new OptimizerPixelProcessorSplit());

	//register_optimizer(new OptimizerSplit());
}

RendererSW::~RendererSW() { }

String RendererSW::get_name() const { return _("Cobra (software)"); }

void RendererSW::initialize()
{
	software::FFT::initialize();

}

void RendererSW::deinitialize()
{
	software::FFT::deinitialize();
}

/* === E N T R Y P O I N T ================================================= */
