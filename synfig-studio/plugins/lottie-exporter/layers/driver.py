# pylint: disable=line-too-long
"""
Main driver functions to generate layers
"""

import sys
import logging
import settings
from layers.shape import gen_layer_shape
from layers.solid import gen_layer_solid
from layers.image import gen_layer_image
from layers.shape_solid import gen_layer_shape_solid
from layers.preComp import gen_layer_precomp
from layers.group import gen_layer_group
from layers.blur import gen_layer_blur
from layers.text import gen_layer_text
sys.path.append("..")

def blur_test(lottie):
	"""
	This function will test if this layer has already been blurred or not

	Args:
		lottie (dict) : Lottie dictionary of effects in a layer

	Returns:
		(True if layer has already been blurred)
	"""

	for effects in lottie:
		if effects["ty"] == 29:
			return True

	return False

def calculate_blurs_needed(itr):
	"""
	This function will be called for each non blur layer to calculate all blur layers above it.

	Args:
		itr (int) : Position of layer in canvas

	Returns:
		(None)
	"""
	settings.non_blur_dictionary[itr] = [layer for layer in settings.blur_dictionary.keys() if layer < itr]

def append_blur_dict(layer,itr,group_flag):
	"""
	This function will generate and assign the gaussian blur dictionary required for different layers

	Args:
		layer (common.Layer.Layer) : Synfig format layer
		itr   (int)                : Position of layer in canvas
		group_flag (boolean)	   : True if layer belongs to a Group Layer

	Returns:
		(None)
	"""
	blur_dict = []
	layers = [settings.blur_dictionary[layer_var] for layer_var in settings.non_blur_dictionary[itr]]
	gen_layer_blur(blur_dict,layers)
	if len(layers)!=0:
		if group_flag:
			for asset_index,_ in enumerate(settings.lottie_format["assets"]):
				if "layers" in _.keys():
					for index,val in enumerate(settings.lottie_format["assets"][asset_index]["layers"]):
						blur_flag = blur_test(settings.lottie_format["assets"][asset_index]["layers"][index]["ef"])
						if not blur_flag:
							for blur in blur_dict:
								settings.lottie_format["assets"][asset_index]["layers"][index]["ef"].append(blur)

		else:
			for index,val in enumerate(settings.lottie_format["layers"]):
				if settings.lottie_format["layers"][index]["nm"] == layer.get_description():
					for blur in blur_dict:
						settings.lottie_format["layers"][index]["ef"].append(blur)

def gen_layers(lottie, canvas, layer_itr):
	"""
	This function will be called for each canvas/composition. Main function to
	generate all the layers

	Args:
		lottie (dict) : Layers in Lottie format
		canvas (common.Canvas.Canvas) : Synfig format canvas
		layer_itr (int) : position of layer in canvas

	Returns:
		(None)
	"""
	itr = layer_itr
	shape = settings.SHAPE_LAYER
	solid = settings.SOLID_LAYER
	shape_solid = settings.SHAPE_SOLID_LAYER
	image = settings.IMAGE_LAYER
	pre_comp = settings.PRE_COMP_LAYER
	group = settings.GROUP_LAYER
	skeleton = settings.SKELETON_LAYER
	blur = settings.BLUR_LAYER
	supported_layers = set.union(shape, solid, shape_solid, image, pre_comp, group, skeleton,blur)
	if settings.WITHOUT_VARIABLE_WIDTH:
		shape.add("outline")
		settings.WITHOUT_VARIABLE_WIDTH = False
		
	while itr >= 0:
		layer = canvas[itr]
		if layer.get_type() not in supported_layers:  # Only supported layers
			logging.warning(settings.NOT_SUPPORTED_TEXT, layer.get_type())
			itr -= 1
			continue
		elif not layer.is_active():   # Only render the active layers
			logging.info(settings.NOT_ACTIVE_TEXT, layer.get_type())
			itr -= 1
			continue
		elif not layer.to_render():   # If we don't have to render the layer
			logging.info(settings.EXCLUDE_FROM_RENDERING, layer.get_type())
			itr -= 1
			continue

		if layer.get_type() != "blur":
			lottie.append({})
			layer.set_lottie_layer(lottie[-1])

		if layer.get_type() in shape:           # Goto shape layer
			gen_layer_shape(lottie[-1],
							layer,
							itr)
			calculate_blurs_needed(settings.LEVEL)
			append_blur_dict(layer,settings.LEVEL,settings.INSIDE_PRECOMP)

		elif layer.get_type() in solid:         # Goto solid layer
			gen_layer_solid(lottie[-1],
							layer,
							itr)
			calculate_blurs_needed(settings.LEVEL)
			append_blur_dict(layer,settings.LEVEL,settings.INSIDE_PRECOMP)

		elif layer.get_type() in shape_solid:   # Goto shape_solid layer
			gen_layer_shape_solid(lottie[-1],
								  layer,
								  itr)
			calculate_blurs_needed(settings.LEVEL)
			append_blur_dict(layer,settings.LEVEL,settings.INSIDE_PRECOMP)

		elif layer.get_type() in image:   # Goto image layer
			gen_layer_image(lottie[-1],
							layer,
							itr)
			calculate_blurs_needed(settings.LEVEL)
			append_blur_dict(layer,settings.LEVEL,settings.INSIDE_PRECOMP)

		elif layer.get_type() in blur:
			settings.blur_dictionary[settings.LEVEL] = layer

		elif layer.get_type() in pre_comp:      # Goto precomp layer
			gen_layer_precomp(lottie[-1],
							  layer,
							  itr)
			return  # other layers will be generated inside the precomp
		elif layer.get_type() in group:       # Goto group layer
			gen_layer_group(lottie[-1],
							layer,
							itr)
			# No return statement here

		elif layer.get_type() in text:
			gen_layer_text(lottie[-1],
						   layer,
						   itr)

		elif layer.get_type() in skeleton:
			pass
			# skeletons are just for linking purposes which is served by bones

		settings.LEVEL += 1
		itr -= 1
