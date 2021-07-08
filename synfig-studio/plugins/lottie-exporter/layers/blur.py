# pylint: disable=line-too-long
"""
Will store all the functions corresponding to Blur Layer in lottie
"""

import settings
from properties.valueKeyframed import gen_value_Keyframed
from common.misc import is_animated
from properties.value import gen_properties_value
from common.Count import Count

def blurriness(lottie,parameter,idx,direction):
	"""
	This function will be called for blur layer direction to create the blurriness
	value dictionary

	Args:
		lottie    (dict)               : Lottie Dictionary for blurriness
		parameter (common.Param.Param) : Synfig format param for blur size
		idx       (int)                : Stores the index(number of) of blurriness
		direction (string)             : Indicates the direction of blurring

	Returns:
		(None)
	"""
	lottie["ty"] =  0
	lottie["nm"] = "Blurriness"
	lottie["mn"] = "Gaussian Blur 1"
	lottie["ix"] = idx
	lottie["v"]  = {}
	is_animate = is_animated(parameter[0])
	if is_animate == settings.ANIMATED:
		if direction == "horizontal":
			parameter[0].attrib['type'] = 'blur_anim_x'
			gen_value_Keyframed(lottie["v"],parameter[0],1)
		else:
			parameter[0].attrib['type'] = 'blur_anim_y'
			gen_value_Keyframed(lottie["v"],parameter[0],1)
	else:
		if is_animate == settings.NOT_ANIMATED:
			if direction == "horizontal":
				val = float(parameter[0][0].text) * 100
			else:
				val = float(parameter[0][1].text) * 100
		else:
			if direction == "horizontal":
				val = float(parameter[0][0][0].text) * 100
			else:
				val = float(parameter[0][0][1].text) * 100
		gen_properties_value(lottie["v"],
							 val,
							 1,
							 settings.DEFAULT_ANIMATED,
							 settings.NO_INFO)

def generate_dimensions_dict(lottie,idx,direction):
	"""
	This function will be called for filling blur layer properties

	Args:
		lottie    (dict)               : Lottie Dictionary for blur layer properties
		idx       (int)                : Stores the index(number of) of blur layer properties
		direction (string)             : Indicates the direction of blurring

	Returns:
		(None)
	"""
	lottie["ty"] =  7
	lottie["nm"] = "Blur Dimensions"
	lottie["mn"] = "Gaussian Blur 2"
	lottie["ix"] = idx
	lottie["v"]  = {}
	if direction == "horizontal":
		lottie["nm"] = "Blur Dimensions"
		lottie["mn"] = "Gaussian Blur 2"
		gen_properties_value(lottie["v"],
							 2,
							 idx,
							 settings.DEFAULT_ANIMATED,
							 settings.NO_INFO)
	elif direction == "vertical":
		lottie["nm"] = "Blur Dimensions"
		lottie["mn"] = "Gaussian Blur 2"
		gen_properties_value(lottie["v"],
							 3,
							 idx,
							 settings.DEFAULT_ANIMATED,
							 settings.NO_INFO)
	else:
		lottie["nm"] = "Repeat Edge Pixels"
		lottie["mn"] = "Gaussian Blur 3"
		gen_properties_value(lottie["v"],
							 0,
							 idx,
							 settings.DEFAULT_ANIMATED,
							 settings.NO_INFO)
def fill_blur_dict(lottie, layer, idx,direction):
	"""
	This function will be called for each blur layer separately for the two directions

	Args:
		lottie    (dict)               : Lottie Dictionary for blur layers
		layers    (common.Layer.Layer) : Synfig format layer
		idx       (int)                : Stores the index(number of) of blur layer
		direction (string)             : Indicates the direction of blurring

	Returns:
		(None)
	"""
	index = Count()
	lottie["ty"] = settings.BLUR_TYPE
	lottie["nm"] = layer.get_description()
	lottie["mn"] = lottie["nm"]
	lottie["np"] = 5
	lottie["en"] = 1
	lottie["ix"] = idx
	lottie["ef"] = []
	size = layer.get_param("size").get()
	temp_blurriness = {}
	blurriness(temp_blurriness,size,index.inc(),direction)
	temp_directions = {}
	generate_dimensions_dict(temp_directions,index.inc(),direction)
	temp_pixels = {}
	generate_dimensions_dict(temp_pixels,index.inc(),"edge_pixels")

	lottie["ef"].append(temp_blurriness)
	lottie["ef"].append(temp_directions)
	lottie["ef"].append(temp_pixels)

def gen_layer_blur(lottie, layers):
	"""
	This function will be called for each canvas/composition. Main function to
	generate all the layers

	Args:
		lottie (dict) : Lottie Dictionary for blur layers
		layers (List) : Dictionary of Synfig format layers

	Returns:
		(None)
	"""
	index = Count()
	for layer in layers:
		blur_dict_x = {}
		fill_blur_dict(blur_dict_x,layer,index.inc(),"horizontal")
		blur_dict_y = {}
		fill_blur_dict(blur_dict_y,layer,index.inc(),"vertical")
		lottie.append(blur_dict_x)
		lottie.append(blur_dict_y)
