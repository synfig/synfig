# $Header: /opt/voria/cvs/studio/src/synfigapp/Makefile.am,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $

MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/libltdl

LAYER_ACTION_HH=actions/layerremove.h actions/layermove.h actions/layerraise.h actions/layerlower.h actions/layeradd.h actions/layeractivate.h actions/layerparamset.h actions/layerparamconnect.h actions/layerparamdisconnect.h actions/layerencapsulate.h actions/layerduplicate.h actions/layersetdesc.h
LAYER_ACTION_CC=actions/layerremove.cpp actions/layermove.cpp actions/layerraise.cpp actions/layerlower.cpp actions/layeradd.cpp actions/layeractivate.cpp actions/layerparamset.cpp actions/layerparamconnect.cpp actions/layerparamdisconnect.cpp actions/layerencapsulate.cpp actions/layerduplicate.cpp actions/layersetdesc.cpp

VALUEDESC_ACTION_HH=actions/valuedescset.h actions/valuedescexport.h actions/valuedescconvert.h actions/valuedescconnect.h actions/valuedescdisconnect.h actions/valuedesclink.h
VALUEDESC_ACTION_CC=actions/valuedescset.cpp actions/valuedescexport.cpp actions/valuedescconvert.cpp actions/valuedescconnect.cpp actions/valuedescdisconnect.cpp actions/valuedesclink.cpp

VALUENODE_ACTION_HH=actions/valuenodeconstset.h actions/valuenodeadd.h actions/valuenodereplace.h actions/valuenodelinkconnect.h actions/valuenodelinkdisconnect.h actions/valuenodedynamiclistinsert.h actions/valuenodedynamiclistremove.h actions/valuenoderename.h actions/valuenoderemove.h actions/valuenodedynamiclistinsertsmart.h actions/valuenodedynamiclistremovesmart.h actions/valuenodedynamiclistloop.h actions/valuenodedynamiclistunloop.h actions/valuenodedynamiclistrotateorder.h
VALUENODE_ACTION_CC=actions/valuenodeconstset.cpp actions/valuenodeadd.cpp actions/valuenodereplace.cpp actions/valuenodelinkconnect.cpp actions/valuenodelinkdisconnect.cpp actions/valuenodedynamiclistinsert.cpp actions/valuenodedynamiclistremove.cpp actions/valuenoderename.cpp actions/valuenoderemove.cpp actions/valuenodedynamiclistinsertsmart.cpp actions/valuenodedynamiclistremovesmart.cpp actions/valuenodedynamiclistloop.cpp actions/valuenodedynamiclistunloop.cpp actions/valuenodedynamiclistrotateorder.cpp

WAYPOINT_ACTION_HH=actions/waypointadd.h actions/waypointset.h actions/waypointsetsmart.h actions/waypointremove.h actions/waypointsimpleadd.h
WAYPOINT_ACTION_CC=actions/waypointadd.cpp actions/waypointset.cpp actions/waypointsetsmart.cpp actions/waypointremove.cpp actions/waypointsimpleadd.cpp

TIMEPOINT_ACTION_HH=actions/timepointsmove.h actions/timepointscopy.h actions/timepointsdelete.h timegather.h
TIMEPOINT_ACTION_CC=actions/timepointsmove.cpp actions/timepointscopy.cpp actions/timepointsdelete.cpp timegather.cpp

ACTIVEPOINT_ACTION_HH=actions/activepointadd.h actions/activepointset.h actions/activepointsetsmart.h actions/activepointsetoff.h actions/activepointseton.h actions/activepointremove.h actions/activepointsimpleadd.h
ACTIVEPOINT_ACTION_CC=actions/activepointadd.cpp actions/activepointset.cpp actions/activepointsetsmart.cpp actions/activepointsetoff.cpp actions/activepointseton.cpp actions/activepointremove.cpp actions/activepointsimpleadd.cpp

KEYFRAME_ACTION_HH=actions/keyframesetdelta.h actions/keyframeadd.h actions/keyframeset.h actions/keyframeremove.h actions/keyframeduplicate.h actions/keyframewaypointset.h
KEYFRAME_ACTION_CC=actions/keyframesetdelta.cpp actions/keyframeadd.cpp actions/keyframeset.cpp actions/keyframeremove.cpp actions/keyframeduplicate.cpp actions/keyframewaypointset.cpp

CANVAS_ACTION_HH=actions/canvasrenddescset.h actions/canvasadd.h actions/canvasremove.h
CANVAS_ACTION_CC=actions/canvasrenddescset.cpp actions/canvasadd.cpp actions/canvasremove.cpp

GROUP_ACTION_HH=actions/groupaddlayers.h actions/groupremovelayers.h actions/groupremove.h actions/grouprename.h
GROUP_ACTION_CC=actions/groupaddlayers.cpp actions/groupremovelayers.cpp actions/groupremove.cpp actions/grouprename.cpp

OTHER_ACTION_HH=actions/colorset.h actions/editmodeset.h actions/blinepointtangentmerge.h actions/blinepointtangentsplit.h actions/gradientset.h
OTHER_ACTION_CC=actions/colorset.cpp actions/editmodeset.cpp actions/blinepointtangentmerge.cpp actions/blinepointtangentsplit.cpp actions/gradientset.cpp

ACTIONHH=$(GROUP_ACTION_HH) $(LAYER_ACTION_HH) $(VALUEDSEC_ACTION_HH) $(VALUENODE_ACTION_HH) $(WAYPOINT_ACTION_HH) $(KEYFRAME_ACTION_HH) $(OTHER_ACTION_HH) $(CANVAS_ACTION_HH) $(ACTIVEPOINT_ACTION_HH) $(TIMEPOINT_ACTION_HH)
ACTIONCC=$(GROUP_ACTION_CC) $(LAYER_ACTION_CC) $(VALUEDESC_ACTION_CC) $(VALUENODE_ACTION_CC) $(WAYPOINT_ACTION_CC) $(KEYFRAME_ACTION_CC) $(OTHER_ACTION_CC) $(CANVAS_ACTION_CC) $(ACTIVEPOINT_ACTION_CC) $(TIMEPOINT_ACTION_CC)

SYNFIGAPPHH=$(ACTIONHH) blineconvert.h cvs.h editmode.h action.h action_param.h action_system.h canvasinterface.h instance.h selectionmanager.h uimanager.h value_desc.h main.h inputdevice.h settings.h

synfiglibdir=@synfiglibdir@

lib_LTLIBRARIES = libsynfigapp.la
libsynfigapp_la_SOURCES =  $(SYNFIGAPPHH) $(ACTIONCC) blineconvert.cpp cvs.cpp main.cpp action_param.cpp action.cpp action_system.cpp canvasinterface.cpp instance.cpp uimanager.cpp inputdevice.cpp settings.cpp
libsynfigapp_la_LIBADD = @SYNFIG_LIBS@
libsynfigapp_la_CXXFLAGS = @SYNFIG_CFLAGS@
libsynfigapp_la_LDFLAGS = -export-dynamic -no-undefined

include_synfigappdir=$(prefix)/include/synfigapp-0.0/synfigapp
include_synfigapp_HEADERS = $(SYNFIGAPPHH)
