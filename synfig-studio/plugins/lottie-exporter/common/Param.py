"""
Param.py
Will store the Parameters class for Synfig parameters 
"""

import sys
import copy
import math
import inspect
from lxml import etree
import settings
import common
import synfig.group
from synfig.animation import modify_bool_animation, to_Synfig_axis, is_animated, get_bool_at_frame, get_vector_at_frame, print_animation
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
from properties.value import gen_properties_value
from effects.controller import gen_effects_controller
sys.path.append("..")


class Param:
    """
    Class to keep Synfig format parameters
    """
    def __init__(self, param, parent):
        """
        This parent can be another parameter or a Layer or Canvas
        """
        self.parent = parent
        self.param = param
        self.subparams = {}
        self.SUBPARAMS_EXTRACTED = 0
        self.expression_controllers = [] # Effects will be stored in this
        self.expression = ""
        self.dimension = 1  # 1 represents real, 2 represents vector
        self.get_exported_valuenode()

    def get_exported_valuenode(self):
        """
        If the animation of this param is not directly linked with this param,
        then it must be preset in the canvas :use, :def
        """
        if self.param is None or "use" not in self.param.attrib.keys():
            return
        layer = self.get_layer()
        canvas = layer.getparent()
        key = self.param.attrib["use"]
        anim = canvas.get_def(key)
        assert(anim is not None)
        self.param.append(copy.deepcopy(anim))
        
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

    def get_canvas(self):
        """
        Recursively find the canvas
        """
        if isinstance(self.parent, common.Canvas.Canvas):
            return self.parent
        return self.parent.get_canvas()

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

    def append_same_key(self, key, child):
        """
        Helps append in the dict if same key is already present
        """
        val = Param(child, self.param)
        if isinstance(self.subparams[key], list):
            self.subparams[key].append(val)
        else:
            prev = self.subparams[key]
            self.subparams[key] = [prev, val]

    def extract_subparams(self):
        """
        Extracts the subparameters of this parameter and stores with there tag
        as the key
        """
        if self.SUBPARAMS_EXTRACTED:
            return
        self.SUBPARAMS_EXTRACTED = 1
        self.subparams.clear()
        for child in self.param:
            key = child.tag 
            if key in self.subparams.keys():
                self.append_same_key(key, child)
            else:
                self.subparams[key] = Param(child, self)

    def animate_without_path(self, anim_type):
        """
        If this parameter is not animated, it generates dummy waypoints and
        animates this parameter
        """
        if anim_type == "vector":   # This will never happen, can remove this latter
            self.dimension = 2

        # Check if we are dealing with convert methods
        if self.param[0].tag in settings.CONVERT_METHODS:
            self.extract_subparams()
            if self.param[0].tag == "add":
                self.subparams["add"].extract_subparams()
                self.subparams["add"].subparams["lhs"].animate(anim_type)
                self.subparams["add"].subparams["rhs"].animate(anim_type)
                self.subparams["add"].subparams["scaler"].animate("scalar_multiply")
        else:
            self.single_animate(anim_type)


    def animate(self, anim_type, transform=False):
        """
        If this parameter is not animated, it generates dummy waypoints and
        animates this parameter
        """
        if not transform and anim_type in {"vector"}:
            # Will add link to convert into lottie axis
            value = common.Vector.Vector()
            value[0] = (settings.lottie_format["w"]/2) / settings.PIX_PER_UNIT
            value[1] = -(settings.lottie_format["h"]/2) / settings.PIX_PER_UNIT
            self.add_convert_link(value)

        self.expression_controllers = []
        self.recur_animate(anim_type)

    def recur_animate(self, anim_type):
        """
        Internal private method for animating
        """
        if anim_type == "vector":
            self.dimension = 2

        # Check if we are dealing with convert methods
        if self.param.tag in settings.BONES or self.param[0].tag in settings.CONVERT_METHODS:
            self.extract_subparams()

            if self.param.tag == "bone":  # Carefull about param[0] and param here
                origin, eff_1 = self.subparams["origin"].recur_animate("vector")
                # Now adding the parent's effects in this bone
                guid = self.subparams["parent"][0].attrib["guid"]
                canvas = self.get_canvas()
                bone = canvas.get_bone(guid)
                b_origin, eff_2 = bone.recur_animate("vector")
                self.expression_controllers.extend(eff_1)
                if eff_2 is not None:
                    self.expression_controllers.extend(eff_2)
                    ret = "sum(" + origin + "," + b_origin + ")"
                else:
                    ret = origin
                return ret, self.expression_controllers

            elif self.param.tag == "bone_root": # No animation to be added as this being the root
                return "0", None

            elif self.param[0].tag == "add":
                self.subparams["add"].extract_subparams()
                lhs, effects_1 = self.subparams["add"].subparams["lhs"].recur_animate(anim_type)
                rhs, effects_2 = self.subparams["add"].subparams["rhs"].recur_animate(anim_type)   # Only one of the child should be converted to lottie axis
                scalar, effects_3 = self.subparams["add"].subparams["scalar"].recur_animate("scalar_multiply")
                self.expression_controllers.extend(effects_1)
                self.expression_controllers.extend(effects_2)
                self.expression_controllers.extend(effects_3)
                ret = "mul(sum({lhs}, {rhs}), {scalar})"
                ret = ret.format(lhs=lhs, rhs=rhs, scalar=scalar)
                self.expression = ret
                return ret, self.expression_controllers # Might have to return copy.deepcopy because these expressions are destroyed on animating this parameter again, and this param might be child to 2 params

            elif self.param[0].tag == "average":
                self.subparams["average"].extract_subparams()   # Entries will be extracted here
                lst = self.subparams["average"].subparams["entry"]
                if not isinstance(lst, list):   # When only one entry is present
                    lst = [lst]

                ret = "sum("    # Returning string
                for it in lst:
                    ent, eff = it.recur_animate(anim_type) 
                    self.expression_controllers.extend(eff)
                    ret += ent
                    ret += ","
                ret = ret[:-1]
                ret += ")"

                # Dividing by the length
                ret = "div(" + ret + "," + str(len(lst)) + ")"
                self.expression = ret
                return ret, self.expression_controllers

            elif self.param[0].tag == "composite":  # composite only on vectors
                self.subparams["composite"].extract_subparams()
                x, effect_1 = self.subparams["composite"].subparams["x"].recur_animate("real")
                y, effect_2 = self.subparams["composite"].subparams["y"].recur_animate("real")
                self.expression_controllers.extend(effect_1)
                self.expression_controllers.extend(effect_2)
                ret = "[{x}, -{y}]"
                ret = ret.format(x=x, y=y)
                self.expression = ret
                return ret, self.expression_controllers

            elif self.param[0].tag == "linear":
                self.subparams["linear"].extract_subparams()
                slope, effect_1 = self.subparams["linear"].subparams["slope"].recur_animate(anim_type)
                offset, effect_2 = self.subparams["linear"].subparams["offset"].recur_animate(anim_type)
                self.expression_controllers.extend(effect_1)
                self.expression_controllers.extend(effect_2)
                ret = "sum({offset}, mul({slope}, time))"
                ret = ret.format(offset=offset, slope=slope)
                self.expression = ret
                return ret, self.expression_controllers

            elif self.param[0].tag == "radial_composite":   # only for vectors
                dic = {"gen_layer_group", "gen_layer_rotate", "gen_layer_translate", "gen_layer_scale"}
                prop_name = "angle"
                if self.called_by(dic): # Should find some easier way of finding the caller function
                    prop_name = "rotate_layer_angle"
                self.subparams["radial_composite"].extract_subparams()
                rad, eff_1 = self.subparams["radial_composite"].subparams["radius"].recur_animate("real")
                ang, eff_2 = self.subparams["radial_composite"].subparams["theta"].recur_animate(prop_name)
                self.expression_controllers.extend(eff_1)
                self.expression_controllers.extend(eff_2)
                ret = "[mul({radius}, Math.cos(degreesToRadians({angle}))), mul({radius}, Math.sin(degreesToRadians({angle})))]"
                ret = ret.format(radius=rad, angle=ang)
                self.expression = ret
                return ret, self.expression_controllers

            elif self.param[0].tag == "scale":
                self.subparams["scale"].extract_subparams()
                link, eff_1 = self.subparams["scale"].subparams["link"].recur_animate(anim_type)
                scalar, eff_2 = self.subparams["scale"].subparams["scalar"].recur_animate("scalar_multiply")
                self.expression_controllers.extend(eff_1)
                self.expression_controllers.extend(eff_2)
                ret = "mul({link}, {scalar})"
                ret = ret.format(link=link, scalar=scalar)
                self.expression = ret
                return ret, self.expression_controllers

            elif self.param[0].tag == "subtract":
                self.subparams["subtract"].extract_subparams()
                scalar, eff_1 = self.subparams["subtract"].subparams["scalar"].recur_animate("scalar_multiply")
                lhs, eff_2 = self.subparams["subtract"].subparams["lhs"].recur_animate(anim_type)
                rhs, eff_3 = self.subparams["subtract"].subparams["rhs"].recur_animate(anim_type)
                self.expression_controllers.extend(eff_1)
                self.expression_controllers.extend(eff_2)
                self.expression_controllers.extend(eff_3)
                ret = "mul(sub({lhs}, {rhs}), {scalar})"
                ret = ret.format(lhs=lhs, rhs=rhs, scalar=scalar)
                self.expression = ret
                return ret, self.expression_controllers

            elif self.param[0].tag == "switch":
                self.subparams["switch"].extract_subparams()
                link_off, eff_1 = self.subparams["switch"].subparams["link_off"].recur_animate(anim_type)
                link_on, eff_2 = self.subparams["switch"].subparams["link_on"].recur_animate(anim_type)
                switch, eff_3 = self.subparams["switch"].subparams["switch"].recur_animate("bool")
                self.expression_controllers.extend(eff_1)
                self.expression_controllers.extend(eff_2)
                self.expression_controllers.extend(eff_3)
                ret = "sum(mul({eff_2}, {eff_3}), mul({eff_1}, sub(1, {eff_3})))"
                ret = ret.format(eff_1=link_off, eff_2=link_on, eff_3=switch)
                self.expression = ret
                return ret, self.expression_controllers

            elif self.param[0].tag == "weighted_average":
                self.subparams["weighted_average"].extract_subparams()
                lst = self.subparams["weighted_average"].subparams["entry"]
                if not isinstance(lst, list):   # When only one entry is present
                    lst = [lst]

                ret = "sum("    # Returning string
                den = "sum("    # Denominator string
                for it in lst:
                    it.extract_subparams()  # weighted_vector
                    it.subparams["weighted_vector"].extract_subparams()
                    weight, eff_1 = it.subparams["weighted_vector"].subparams["weight"].recur_animate("scalar_multiply")
                    value, eff_2 = it.subparams["weighted_vector"].subparams["value"].recur_animate(anim_type)
                    self.expression_controllers.extend(eff_1)
                    self.expression_controllers.extend(eff_2)
                    hell = "mul({weight}, {value}),"
                    hell = hell.format(weight=weight, value=value)
                    ret += hell

                    den += weight
                    den += ","
                ret = ret[:-1]
                den = den[:-1]
                den += ")"
                ret += ")"
                ret = "div(" + ret + "," + den + ")"
                self.expression = ret
                return ret, self.expression_controllers

            elif self.param[0].tag == "bone_link":
                self.subparams["bone_link"].extract_subparams()
                guid = self.subparams["bone_link"].subparams["bone"][0].attrib["guid"]
                layer = self.get_layer()
                canvas = layer.getparent()
                bone = canvas.get_bone(guid)
                ######## THIS IS NOT CORRECT WAY OF FORMING BONE PARAM
                ########################################################
                origin, eff_1 = bone.recur_animate("vector")  # animating only the origin now
                self.expression_controllers.extend(eff_1)
                self.expression = origin
                return origin, self.expression_controllers

        else:
            self.single_animate(anim_type)
            # Insert the animation into the effect
            self.expression_controllers.append({})

            if anim_type == "vector":
                self.gen_path("vector")
            elif anim_type not in {"bool", "string"}:   # These do not need path generation
                self.gen_path("real")   # "real" can be of various types as defined in common.misc.parse_position()
            elif anim_type in {"bool"}:     # This is the first time bool is needed
                self.gen_path("bool")

            
            gen_effects_controller(self.expression_controllers[-1], self.get_path(), anim_type)

            # Extract name from the effects
            ret = "effect('{effect_1}')('{effect_2}')"
            ret = ret.format(effect_1=self.expression_controllers[-1]["nm"], effect_2=self.expression_controllers[-1]["ef"][0]["nm"])
            self.expression = ret
            return ret, self.expression_controllers

    def called_by(self, dic):
        """
        Checks if this function's ancestor are in this dictionary
        """
        for it in inspect.stack():
            if it[3] in dic:
                return True
        return False

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
            if anim_type == "bool":
                modify_bool_animation(self.param[0])
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

        if anim_type == "bool":
            modify_bool_animation(self.param[0])

    def gen_path(self, anim_type="real", idx=0):
        """
        Generates the path for this parameter over time depending on the
        animation type of this parameter
        """
        self.path = {}
        if anim_type in {"real", "bool"}:
            gen_value_Keyframed(self.path, self.param[0], idx)
        else:
            gen_properties_multi_dimensional_keyframed(self.path, self.param[0], idx)

    def get_path(self):
        """
        Returns the dictionary which stores the path of this parameter
        """
        return self.path

    def fill_path(self, lottie, key):
        """
        Fill's the lottie dictionary with the path of the parameter
        """
        if self.param[0].tag in settings.CONVERT_METHODS:
            expression = "var $bm_rt; $bm_rt = {expr}"
            expression = expression.format(expr=self.expression)
            if self.dimension == 1:
                val = 1
            else:
                val = [1, 1]
            gen_properties_value(lottie[key],
                                 val,
                                 0,
                                 0,
                                 expression)
            if "ef" not in self.get_layer().get_lottie_layer().keys():
                self.get_layer().get_lottie_layer()["ef"] = []
            self.get_layer().get_lottie_layer()["ef"].extend(self.expression_controllers)
                
        else:
            lottie[key] = copy.deepcopy(self.path)

    def get_value(self, frame):
        """
        Returns the value of the parameter at a given frame
        """
        if self.param.tag in settings.BONES or self.param[0].tag in settings.CONVERT_METHODS:
            if self.param.tag == "bone":
                ret = self.subparams["origin"].get_value(frame)
                # Now adding the parent's effects in this bone
                guid = self.subparams["parent"][0].attrib["guid"]
                canvas = self.get_canvas()
                bone = canvas.get_bone(guid)
                ret2 = bone.get_value(frame)
                if ret2 is not None:
                    if isinstance(ret, list):
                        ret[0] += ret2[0]
                        ret[1] += ret2[1]
                    else:
                        ret += ret2

            elif self.param.tag == "bone_root":
                return None

            elif self.param[0].tag == "add":
                ret = self.subparams["add"].subparams["lhs"].get_value(frame)
                ret2 = self.subparams["add"].subparams["rhs"].get_value(frame)
                mul = self.subparams["add"].subparams["scalar"].get_value(frame)
                if isinstance(ret, list):
                    ret[0] += ret2[0]
                    ret[1] += ret2[1]
                    ret = [it*mul for it in ret]
                else:
                    ret += ret2
                    ret *= mul

            elif self.param[0].tag == "average":
                lst = self.subparams["average"].subparams["entry"]
                if not isinstance(lst, list):
                    lst = [lst]

                ret = [0, 0]
                if not isinstance(lst[0].get_value(frame), list):
                    ret = 0
                for it in lst:
                    val = it.get_value(frame)
                    if isinstance(val, list):
                        ret[0], ret[1] = ret[0] + val[0], ret[1] + val[1]
                    else:
                        ret += val
                if isinstance(ret, list):
                    ret[0], ret[1] = ret[0] / len(lst), ret[1] / len(lst)
                else:
                    ret /= len(lst)

            elif self.param[0].tag == "weighted_average":
                self.subparams["weighted_average"].extract_subparams()
                lst = self.subparams["weighted_average"].subparams["entry"]
                if not isinstance(lst, list):   # When only one entry is present
                    lst = [lst]

                ret = [0, 0]
                den = 0
                if not isinstance(lst[0].subparams["weighted_vector"].subparams["value"].get_value(frame), list):
                    ret = 0
                for it in lst:
                    weight = it.subparams["weighted_vector"].subparams["weight"].get_value(frame)
                    value = it.subparams["weighted_vector"].subparams["value"].get_value(frame)
                    den += weight
                    if isinstance(value, list):
                        ret[0], ret[1] = ret[0] + value[0]*weight, ret[1] + value[1]*weight
                    else:
                        ret += value*weight
                if isinstance(ret, list):
                    ret[0], ret[1] = ret[0] / den, ret[1] / den
                else:
                    ret /= den

            elif self.param[0].tag == "composite":  # Only available for vectors
                x = self.subparams["composite"].subparams["x"].get_value(frame)
                y = self.subparams["composite"].subparams["y"].get_value(frame)
                ret = [x, -y]   # -y as we are not using change_axis() function here

            elif self.param[0].tag == "linear":
                slope = self.subparams["linear"].subparams["slope"].get_value(frame)
                offset = self.subparams["linear"].subparams["offset"].get_value(frame)
                if isinstance(slope, list):
                    ret = [0, 0]
                    ret[0] = offset[0] + slope[0]*(frame/settings.lottie_format["fr"])
                    ret[1] = offset[1] + slope[1]*(frame/settings.lottie_format["fr"])
                else:
                    ret = offset + slope*(frame/settings.lottie_format["fr"])

            elif self.param[0].tag == "radial_composite":   # Only for vectors
                rad = self.subparams["radial_composite"].subparams["radius"].get_value(frame)
                angle = to_Synfig_axis(self.subparams["radial_composite"].subparams["theta"].get_value(frame), "angle")
                angle = math.radians(angle)
                x = rad * math.cos(angle)
                y = rad * math.sin(angle)
                ret = [x, -y]

            elif self.param[0].tag == "scale":
                link = self.subparams["scale"].subparams["link"].get_value(frame)
                scalar = self.subparams["scale"].subparams["scalar"].get_value(frame)
                if isinstance(link, list):
                    link[0] *= scalar
                    link[1] *= scalar
                else:
                    link *= scalar
                ret = link

            elif self.param[0].tag == "subtract":
                lhs = self.subparams["subtract"].subparams["lhs"].get_value(frame)
                rhs = self.subparams["subtract"].subparams["rhs"].get_value(frame)
                scalar = self.subparams["subtract"].subparams["scalar"].get_value(frame)
                if isinstance(lhs, list):
                    ret = [0, 0]
                    ret[0] = (lhs[0] - rhs[0]) * scalar
                    ret[1] = (lhs[1] - rhs[1]) * scalar
                else:
                    ret = (lhs - rhs) * scalar

            elif self.param[0].tag == "switch":
                link_off = self.subparams["switch"].subparams["link_off"].get_value(frame)
                link_on = self.subparams["switch"].subparams["link_on"].get_value(frame)
                switch = self.subparams["switch"].subparams["switch"].get_value(frame)
                if isinstance(link_on, list):
                    ret = [0, 0]
                    ret[0] = link_on[0] * switch + link_off[0] * (1 - switch)
                    ret[1] = link_on[1] * switch + link_off[1] * (1 - switch)
                else:
                    ret = link_on * switch + link_off * (1 - switch)

            elif self.param[0].tag == "bone_link":
                guid = self.subparams["bone_link"].subparams["bone"][0].attrib["guid"]
                layer = self.get_layer()
                canvas = layer.getparent()
                bone = canvas.get_bone(guid)
                ret = bone.get_value(frame)

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
        Will modify the animation by inserting <add></add> xml elements
        """
        offset = synfig.group.get_offset()
        self.add_convert_link(offset)


    def add_convert_link(self, val):
        """
        Private method for inserting <add></add>, given a offset
        """
        st = "<add type='vector'><lhs></lhs><rhs></rhs><scalar><real value='1.00'/></scalar></add>"
        root = etree.fromstring(st)
        first = copy.deepcopy(self.param[0])
        root[0].append(first)
        
        st = "<vector><x>{x_val}</x><y>{y_val}</y></vector>"
        st = st.format(x_val=val[0], y_val=val[1])
        second = etree.fromstring(st)
        root[1].append(second)

        self.param[0].getparent().remove(self.param[0])
        self.param.append(root)
        self.SUBPARAMS_EXTRACTED = 0
