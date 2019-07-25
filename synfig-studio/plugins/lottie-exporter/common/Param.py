"""
Param.py
Will store the Parameters class for Synfig parameters 
"""

import sys
import copy
from lxml import etree
import settings
import common
import synfig.group
from synfig.animation import to_Synfig_axis, is_animated, get_bool_at_frame, get_vector_at_frame, print_animation
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
sys.path.append("..")


class Param:
    """
    Class to keep Synfig format parameters
    """
    def __init__(self, param, parent):
        """
        This parent can be another parameter or a Layer
        """
        self.parent = parent
        self.param = param
        self.subparams = {}
        self.IS_ANIMATED = 0
        
    def reset(self):
        """
        This function will be called when any of the entities of the paramter
        are changed. This should be internally called, but for now compromising
        """
        self.PATH_GENERATED = 0
        self.TRANSFORM_PATH_GENERATED = 0
        self.path = {}

    def get(self):
        """
        Returns the original param
        """
        return self.param

    def get_text(self):
        """
        Returns the text stored inside
        """
        return self.param.text

    def __getitem__(self, itr):
        """
        Returns the child corresponding to itr
        """
        return self.param[itr]

    def __setitem__(self, itr, val):
        """
        Sets the value of child corresponding to itr
        """
        self.param[itr] = val

    def get_layer_type(self):
        """
        Recursively go till the top until layer is not reached
        """
        if isinstance(self.parent, common.Layer.Layer):
            return self.parent.get_type()
        return self.parent.get_layer_type()

    def get_layer(self):
        """
        Recursively find the layer
        """
        if isinstance(self.parent, common.Layer.Layer):
            return self.parent
        return self.parent.get_layer()

    def getparent(self):
        """
        Returns the parent of this parameter
        """
        return self.parent

    def add_subparam(self, key, val):
        """
        Adds sub parameters
        """
        self.subparams[key] = val

    def get_subparam_dict(self):
        """
        Returns the address of the dict
        """
        return self.subparams

    def get_subparam(self, key):
        """
        Given a key, returns it's dictionary
        """
        return self.subparams[key]

    def extract_subparams(self):
        """
        Extracts the subparameters of this parameter and stores with there tag
        as the key
        """
        for child in self.param:
            key = child.tag 
            self.subparams[key] = Param(child, self.param)

    def animate_without_path(self, anim_type, transform=False):
        """
        If this parameter is not animated, it generates dummy waypoints and
        animates this parameter
        """

        # Check if we are dealing with convert methods
        if self.param[0].tag in settings.CONVERT_METHODS:
            self.extract_subparams()
            if self.param[0].tag == "add":
                self.subparams["add"].extract_subparams()
                self.subparams["add"].subparams["lhs"].animate(anim_type, transform)
                self.subparams["add"].subparams["rhs"].animate(anim_type, transform)
                self.subparams["add"].subparams["scaler"].animate("real")
        else:
            self.single_animate(anim_type)


    def animate(self, anim_type, transform=False):
        """
        If this parameter is not animated, it generates dummy waypoints and
        animates this parameter
        """

        # Check if we are dealing with convert methods
        if self.param[0].tag in settings.CONVERT_METHODS:
            self.extract_subparams()
            if self.param[0].tag == "add":
                self.subparams["add"].extract_subparams()
                self.subparams["add"].subparams["lhs"].animate(anim_type, transform)
                self.subparams["add"].subparams["rhs"].animate(anim_type, transform)
                self.subparams["add"].subparams["scalar"].animate("real")
        else:
            self.single_animate(anim_type)
            if anim_type == "vector":
                self.gen_path("vector", transform)
            elif anim_type not in {"bool", "string"}:   # These do not need path generation
                self.gen_path("real", transform)   # "real" can be of various types as defined in common.misc.parse_position()

    def single_animate(self, anim_type):
        """
        For animating a single animation, convert methods will use this internal
        function
        """
        is_animate = common.misc.is_animated(self.param[0])
        if is_animate == 2:
            # If already animated, no need to add waypoints
            # Forcibly set it's animation type to the anim_type
            self.param[0].attrib["type"] = anim_type
            return
        elif is_animate == 0:
            st = '<animated type="{anim_type}"><waypoint time="0s" before="constant" after="constant"></waypoint></animated>'
            st = st.format(anim_type=anim_type)
            root = etree.fromstring(st)
            root[0].append(copy.deepcopy(self.param[0]))
            self.param[0] = root
        elif is_animate == 1:
            self.param[0].attrib["type"] = anim_type
            self.param[0][0].attrib["before"] = self.param[0][0].attrib["after"] = "constant"

        new_waypoint = copy.deepcopy(self.param[0][0])
        frame = common.misc.get_frame(self.param[0][0])
        frame += 1
        time = frame / settings.lottie_format["fr"]
        time = str(time) + 's'
        new_waypoint.attrib["time"] = time
        self.param[0].insert(1, new_waypoint)

    def gen_path(self, anim_type="real", transform=False, idx=0):
        """
        Generates the path for this parameter over time depending on the
        animation type of this parameter
        """
        self.path = {}
        if transform:
            self.param[0].attrib["transform_axis"] = "true"
        if anim_type == "real":
            gen_value_Keyframed(self.path, self.param[0], idx)
        else:
            gen_properties_multi_dimensional_keyframed(self.path, self.param[0], idx)
        self.param[0].attrib["transform_axis"] = "false"

    def get_path(self):
        """
        Returns the dictionary which stores the path of this parameter
        """
        return self.path

    def get_value(self, frame):
        """
        Returns the value of the parameter at a given frame
        """
        if self.param[0].tag in settings.CONVERT_METHODS:
            if self.param[0].tag == "add":
                ret = self.subparams["add"].subparams["lhs"].get_value(frame)
                ret2 = self.subparams["add"].subparams["rhs"].get_value(frame)
                mul = to_Synfig_axis(self.subparams["add"].subparams["scalar"].get_value(frame), "real")
                ret[0] += ret2[0]
                ret[1] += ret2[1]
                ret = [it*mul for it in ret]
        else:
            ret = self.get_single_value(frame)
        return ret

    def get_single_value(self, frame):
        """
        Returns the value of some parameter which is not a convert method
        """
        if self.param[0].attrib["type"] == "bool":  # No need of lottie format path here
            return get_bool_at_frame(self.param[0], frame)

        if not self.path:   # Empty dictionary
            raise KeyError("Please calculate the path of this parameter before getting value at a frame")
        return get_vector_at_frame(self.path, frame)


    def add_offset(self):
        """
        Updates the position parameter of Synfig format which has offset due to
        increase in widht and height of pre-comp layer(group layer)
        """
        # Only for 2-D animations
        offset = synfig.group.get_offset()
        is_animate = is_animated(self.param[0])
        if is_animate == 0:
            self.add(self.param[0], offset)
        else:
            for waypoint in self.param[0]:
                self.add(waypoint[0], offset)

    def add(self, vector, offset):
        """ 
        Helper function to modify Synfig xml

        Args:
            vector (lxml.etree._Element) : Position in Synfig format
            offset (common.Vector.Vector) : offset to be added to that position

        Returns:
            (None)
        """
        vector[0].text = str(float(vector[0].text) + offset[0])
        vector[1].text = str(float(vector[1].text) + offset[1])
