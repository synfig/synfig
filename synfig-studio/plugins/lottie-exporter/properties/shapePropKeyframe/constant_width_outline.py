# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of outline layer
in Lottie format without variable widths
"""

import sys
import settings
from common.Bline import Bline
from common.misc import is_animated
from common.Count import Count
from properties.value import gen_properties_value
from properties.valueKeyframed import gen_value_Keyframed
from properties.shapePropKeyframe.helper import insert_dict_at, animate_tangents, cubic_to
from properties.shapePropKeyframe.outline import get_outline_param_at_frame,synfig_outline

def gen_bline_outline_constant(lottie, bline_point, layer, transformation, idx):
	"""
	"""
	index = Count()
	lottie["ty"]  = "gr"
	lottie["nm"]  = "Shape "+ str(idx)
	lottie["np"]  = 3
	lottie["cix"] = 2
	lottie["bm"]  = 0
	lottie["ix"]  = idx
	lottie["mn"]  = "ADBE Vector Group " +str(idx)
	lottie["hd"]  = "false"
	lottie["it"]  = []
	lottie["it"].append({})
	lottie["it"].append({})
	lottie["it"].append({})

	bline = Bline(bline_point[0], bline_point)

	#Creating transformation dictionary
	lottie["it"][2]["ty"] = "tr"
	lottie["it"][2]["nm"] = "Transform"
	lottie["it"][2].update(transformation)
	#Creating stroke dictionary
	lottie["it"][1]["ty"] = "st"
	lottie["it"][1]["lc"] = 2
	lottie["it"][1]["lj"] = 1
	lottie["it"][1]["ml"] = 37
	lottie["it"][1]["bm"] = 0
	lottie["it"][1]["nm"] = "Stroke " + str(idx)
	lottie["it"][1]["mn"] = "ADBE Vector Graphic - Stroke " +str(idx)
	lottie["it"][1]["hd"] = "false"
	lottie["it"][1]["c"]  = {}
	lottie["it"][1]["o"]  = {}
	lottie["it"][1]["w"]  = {}

	#Color
	color = layer.get_param("color").get()
	is_animate = is_animated(color[0])
	if is_animate == settings.ANIMATED:
		gen_value_Keyframed(lottie["it"][1]["c"], color[0], index.inc())

	else:
		if is_animate == settings.NOT_ANIMATED:
			val = color[0]
		else:
			val = color[0][0][0]
		red = float(val[0].text)
		green = float(val[1].text)
		blue = float(val[2].text)
		red, green, blue = red ** (1/settings.GAMMA[0]), green ** (1/settings.GAMMA[1]), blue ** (1/ settings.GAMMA[2])
		alpha = float(val[3].text)
		gen_properties_value(lottie["it"][1]["c"],
							[red, green, blue, alpha],
							index.inc(),
							settings.DEFAULT_ANIMATED,
							settings.NO_INFO)

	#Opacity
	opacity = layer.get_param("amount")
	opacity.animate("opacity")
	opacity.fill_path(lottie["it"][1],"o")

	#Constant width
	loop = bline.get_loop()
	width = layer.get_param("width")
	if loop:
		width.scale_convert_link(1)
	else:
		width.scale_convert_link(4)

	width.animate("real")
	width.fill_path(lottie["it"][1],"w")

	#Creating shape dictionary
	lottie["it"][0]["ind"] =  0
	lottie["it"][0]["ty"] = "sh"
	lottie["it"][0]["ix"] = 1
	lottie["it"][0]["ks"] = {}
	lottie["it"][0]["nm"] =  "Path 1",
	lottie["it"][0]["mn"] =  "ADBE Vector Shape - Group",
	lottie["it"][0]["hd"] =  "false"
	lottie["it"][0]["ks"]["a"] = 1 
	lottie["it"][0]["ks"]["ix"] = lottie["it"][0]["ix"] + 1 
	lottie["it"][0]["ks"]["k"] = []

	window = {}
	window["first"] = sys.maxsize
	window["last"] = -1

	for entry in bline.get_entry_list():
		pos = entry["point"]
		width = entry["width"]
		t1 = entry["t1"]
		t2 = entry["t2"]
		split_r = entry["split_radius"]
		split_a = entry["split_angle"]

		pos.update_frame_window(window)
		# Empty the pos and fill in the new animated pos
		pos.animate("vector")

		width.update_frame_window(window)
		width.animate("real")

		split_r.update_frame_window(window)
		split_r.animate_without_path("bool")

		split_a.update_frame_window(window)
		split_a.animate_without_path("bool")

		animate_tangents(t1, window)
		animate_tangents(t2, window)


	outer_width = layer.get_param("width")
	sharp_cusps = layer.get_param("sharp_cusps")
	expand = layer.get_param("expand")
	r_tip0 = layer.get_param("round_tip[0]")
	r_tip1 = layer.get_param("round_tip[1]")
	homo_width = layer.get_param("homogeneous_width")
	origin = layer.get_param("origin")

	# Animating the origin
	origin.update_frame_window(window)
	origin.animate("vector")

	# Animating the outer width
	outer_width.update_frame_window(window)
	outer_width.animate("real")

	# Animating the sharp_cusps
	sharp_cusps.update_frame_window(window)
	sharp_cusps.animate_without_path("bool")

	# Animating the expand param
	expand.update_frame_window(window)
	expand.animate("real")

	# Animating the round tip 0
	r_tip0.update_frame_window(window)
	r_tip0.animate_without_path("bool")

	# Animating the round tip 1
	r_tip1.update_frame_window(window)
	r_tip1.animate_without_path("bool")

	# Animating the homogeneous width
	homo_width.update_frame_window(window)
	homo_width.animate_without_path("bool")
	# Minimizing the window size
	if window["first"] == sys.maxsize and window["last"] == -1:
		window["first"] = window["last"] = 0
	
	frames = list(set(settings.WAYPOINTS_LIST))
	length = bline.get_len()

	flag = False
	if loop:
		flag = True

	for fr in frames:
		st_val = insert_dict_at(lottie["it"][0]["ks"]["k"], -1, fr, flag,True)
		cur_origin = origin.get_value(fr)
		for point in range(0,length):
			pos_ret, width, t1, t2, split_r_val, split_a_val = get_outline_param_at_frame(bline[point],fr)
			cubic_to(pos_ret,t1,t2,st_val,cur_origin,False,True)


