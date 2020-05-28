"""
Param.py
Will store the Parameters class for Synfig parameters 
"""

import sys
import copy
import math
from lxml import etree
import settings
import common
import synfig.group
from synfig.animation import modify_bool_animation, to_Synfig_axis, is_animated, get_bool_at_frame, get_vector_at_frame
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

        self.is_group_child = 0

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
        This function will be called when any of the entities of the parameter
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
        val = Param(child, self)

        if self.is_group_child:
            val.is_group_child = True

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

                if self.is_group_child:
                    self.subparams[key].is_group_child = True

    def animate_without_path(self, anim_type):
        """
        If this parameter is not animated, it generates dummy waypoints and
        animates this parameter
        """
        if anim_type in {"vector", "group_layer_scale", "stretch_layer_scale", "circle_radius"}:   # This will never happen, can remove this latter
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
        if anim_type in {"vector", "group_layer_scale", "stretch_layer_scale", "circle_radius"}:
            self.dimension = 2

        # Check if we are dealing with convert methods
        if self.param.tag in settings.BONES or self.param[0].tag in settings.CONVERT_METHODS:
            self.extract_subparams()

            if self.param.tag == "bone":  # Carefull about param[0] and param here

                # Get the animation of this origin
                bone_origin, eff_1 = self.subparams["origin"].recur_animate("vector")

                # Get the animation of angle
                prop_name = "angle"
                if self.is_group_child:
                    prop_name = "rotate_layer_angle"
                bone_angle, ang_eff = self.subparams["angle"].recur_animate(prop_name)

                # Get the local length scale of the bone
                lls, lls_eff = self.subparams["scalelx"].recur_animate("scalar_multiply")

                # Get the recursive length scale of the bone
                rls, rls_eff = self.subparams["scalex"].recur_animate("scalar_multiply")

                # Adding to expression controller
                self.expression_controllers.extend(eff_1)
                self.expression_controllers.extend(ang_eff)
                self.expression_controllers.extend(lls_eff)
                self.expression_controllers.extend(rls_eff)

                # Now adding the parent's effects in this bone
                guid = self.subparams["parent"][0].attrib["guid"]
                canvas = self.get_canvas()
                bone = canvas.get_bone(guid)

                # Store the previous state
                prev_state = bone.is_group_child
                if self.is_group_child:
                    bone.is_group_child = True

                par_origin, par_angle, par_lls, par_rls, par_eff = bone.recur_animate("vector")

                # Restore the previous state
                bone.is_group_child = prev_state

                if par_eff is not None: # checking it is not the bone_root
                    # Adding expression to the expression controller
                    self.expression_controllers.extend(par_eff)

                    # Modifying the bone_origin according to the parent's local scale
                    bone_origin = "mul(" + bone_origin + ", " + par_lls + ")"

                    # Modifying the bone_origin according to the parent's recursive scale
                    bone_origin = "mul(" + bone_origin + ", " + par_rls + ")"

                    # Forming the expression for new origin
                    a1 = "degreesToRadians({angle})"
                    a2 = "degreesToRadians(sum({angle}, 90))"
                    a1, a2 = a1.format(angle=par_angle), a2.format(angle=par_angle)

                    ret_x = "sum({par_origin}[0], mul({bone_origin}[0], Math.cos({a1})), mul({bone_origin}[1], Math.cos({a2})))"
                    ret_y = "sum({par_origin}[1], mul({bone_origin}[0], Math.sin({a1})), mul({bone_origin}[1], Math.sin({a2})))"
                    ret_origin = "[" + ret_x + ", " + ret_y + "]"
                    ret_origin = ret_origin.format(par_origin=par_origin, bone_origin=bone_origin, a1=a1, a2=a2)

                    # Forming the expression for new angle
                    ret_angle = "sum({par_angle}, {bone_angle})"
                    ret_angle = ret_angle.format(par_angle=par_angle, bone_angle=bone_angle)
                else:
                    ret_origin = bone_origin
                    ret_angle = bone_angle

                ############# REMOVE AFTER DEBUGGING rls
                rls = "1"
                return ret_origin, ret_angle, lls, rls, self.expression_controllers

            elif self.param.tag == "bone_root": # No animation to be added as this being the root
                return "[0, 0]", "0", "1", "1", None

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

            elif self.param[0].tag == "exp":
                self.subparams["exp"].extract_subparams()
                exp, effects_1 = self.subparams["exp"].subparams["exp"].recur_animate("scalar_multiply")
                scale, effects_2 = self.subparams["exp"].subparams["scale"].recur_animate(anim_type)
                self.expression_controllers.extend(effects_1)
                self.expression_controllers.extend(effects_2)
                ret = "mul(Math.exp({exp}), {scale})"
                ret = ret.format(exp=exp, scale=scale)
                self.expression = ret
                return ret, self.expression_controllers

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
                prop_name = "angle"
                sett = settings.GROUP_LAYER.copy()
                sett.update(settings.PRE_COMP_LAYER)
                if self.get_layer().get_type() in sett: # depending upon the layer type, we use the angle is positive/negative value
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
                bone = self.get_bone_from_canvas(guid)

                base_value, bv_eff = self.subparams["bone_link"].subparams["base_value"].recur_animate("vector")

                # Storing previous state
                prev_state = bone.is_group_child
                if self.is_group_child:
                    bone.is_group_child = True
                origin, angle, lls, rls, eff = bone.recur_animate("vector")

                # Adding lls effect to the bone
                base_value = "mul(" + base_value + ", " + lls + ")"

                # Adding rls effect to the bone
                base_value = "mul(" + base_value + ", " + rls + ")"

                # Adding effect of base_value here
                a1 = "degreesToRadians({angle})"
                a2 = "degreesToRadians(sum({angle}, 90))"
                a1, a2 = a1.format(angle=angle), a2.format(angle=angle)

                ret_x = "sub(sum({origin}[0], mul({base_value}[0], Math.cos({a1}))), mul({base_value}[1], Math.cos({a2})))"
                ret_y = "sub(sum({origin}[1], mul({base_value}[0], Math.sin({a1}))), mul({base_value}[1], Math.sin({a2})))"
                ret_origin = "[" + ret_x + ", -" + ret_y + "]"
                ret_origin = ret_origin.format(origin=origin, base_value=base_value, a1=a1, a2=a2)

                # Restore previous state
                bone.is_group_child = prev_state

                if eff is not None:
                    self.expression_controllers.extend(eff)
                self.expression_controllers.extend(bv_eff)

                self.expression = ret_origin
                return ret_origin, self.expression_controllers

            elif self.param[0].tag == "sine":
                self.subparams["sine"].extract_subparams()
                angle, eff_1 = self.subparams["sine"].subparams["angle"].recur_animate("region_angle")
                amp, eff_2 = self.subparams["sine"].subparams["amp"].recur_animate("real")
                self.expression_controllers.extend(eff_1)
                self.expression_controllers.extend(eff_2)
                
                if self.dimension == 2:
                    ret = "mul(Math.sin(degreesToRadians({angle})), [{amp}, {amp}])"
                else:
                    ret = "mul(Math.sin(degreesToRadians({angle})),{amp})"
                ret = ret.format(angle=angle,amp=amp)

                self.expression = ret
                return ret, self.expression_controllers

            elif self.param[0].tag == "cos":
                self.subparams["cos"].extract_subparams()
                angle, eff_1 = self.subparams["cos"].subparams["angle"].recur_animate("region_angle")
                amp, eff_2 = self.subparams["cos"].subparams["amp"].recur_animate("real")
                self.expression_controllers.extend(eff_1)
                self.expression_controllers.extend(eff_2)
                
                if self.dimension == 2:
                    ret = "mul(Math.cos(degreesToRadians({angle})), [{amp}, {amp}])"
                else:
                    ret = "mul(Math.cos(degreesToRadians({angle})),{amp})"
                ret = ret.format(angle=angle,amp=amp)

                self.expression = ret
                return ret, self.expression_controllers

            elif self.param[0].tag == "fromint":
                self.subparams["fromint"].extract_subparams()
                if self.dimension == 2:
                    link, eff_1 = self.subparams["fromint"].subparams["link"].recur_animate("scalar_multiply")
                    self.expression_controllers.extend(eff_1)
                    ret = "[mul(Math.round({link}), {PIX_PER_UNIT}), mul(Math.round({link}), {PIX_PER_UNIT})]"
                    ret = ret.format(link=link, PIX_PER_UNIT=settings.PIX_PER_UNIT)
                else:
                    link, eff_1 = self.subparams["fromint"].subparams["link"].recur_animate("scalar_multiply")
                    self.expression_controllers.extend(eff_1)
                    ret = "mul(Math.round({link}), {PIX_PER_UNIT})"
                    ret = ret.format(link=link, PIX_PER_UNIT=settings.PIX_PER_UNIT)
                self.expression = ret
                return ret, self.expression_controllers

            elif self.param[0].tag == "atan2":
                self.subparams["atan2"].extract_subparams()
                y, eff_1 = self.subparams["atan2"].subparams["y"].recur_animate(anim_type)
                x, eff_2 = self.subparams["atan2"].subparams["x"].recur_animate(anim_type)
                self.expression_controllers.extend(eff_1)
                self.expression_controllers.extend(eff_2)
                
                ret = "sub(180,radiansToDegrees(Math.atan2({y}, {x})))"
                ret = ret.format(y=y,x=x)
                self.expression = ret
                return ret, self.expression_controllers

            elif self.param[0].tag == "vectorangle":
                self.subparams["vectorangle"].extract_subparams()
                vector, eff_1 = self.subparams["vectorangle"].subparams["vector"].recur_animate("vector")
                self.expression_controllers.extend(eff_1)
                ret = "radiansToDegrees(Math.atan2({y}[1], {x}[0]))"
                ret = ret.format(y=vector,x=vector)

                self.expression = ret
                return ret, self.expression_controllers

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
        Public method to get the value of the parameter at a given frame
        """
        ret = self.__get_value(frame)
        # Convert into Lottie format
        if isinstance(ret, list):
            ret = [ret[0], -ret[1]]
        return ret
    
    def __get_value(self, frame):
        """
        Returns the value of the parameter at a given frame
        """
        if self.param.tag in settings.BONES or self.param[0].tag in settings.CONVERT_METHODS:
            if self.param.tag == "bone":
                cur_origin = self.subparams["origin"].__get_value(frame)

                # Now adding the parent's effects in this bone
                guid = self.subparams["parent"][0].attrib["guid"]
                canvas = self.get_canvas()
                bone = canvas.get_bone(guid)
                shifted_origin, shifted_angle, lls, rls = bone.__get_value(frame)
                a1, a2 = math.radians(shifted_angle), math.radians(shifted_angle+90)

                # Calculating this bones angle with respect to parent bone's
                # angle
                if "angle" in self.subparams.keys():
                    angle = to_Synfig_axis(self.subparams["angle"].__get_value(frame), "angle")
                else:
                    angle = 0

                # Calculating the local length scale
                local_length_scale = self.subparams["scalelx"].__get_value(frame)

                # Calculating the recursive length scale
                this_rls = self.subparams["scalex"].__get_value(frame)    # In current angle's direction
                absolute_angle = shifted_angle+angle
                aa1 = math.radians(absolute_angle)
                this_rls = [this_rls * math.cos(aa1), this_rls * math.sin(aa1)]

                # Calculate returning recursive length
                ret_rls = [this_rls[0]*rls[0], this_rls[1]*rls[1]]
                ##### REMOVE AFTER DEBUGGING
                ret_rls = [1, 1]

                # Multiplying the current bone origin with the scale
                cur_origin = [i*lls for i in cur_origin]

                ret = shifted_origin
                # Adding effect of x component
                ret[0] = ret[0] + (cur_origin[0] * math.cos(a1) + cur_origin[1] * math.cos(a2)) * rls[0]
                ret[1] = ret[1] + (cur_origin[0] * math.sin(a1) + cur_origin[1] * math.sin(a2)) * rls[1]

                return ret, absolute_angle, local_length_scale, ret_rls

            elif self.param.tag == "bone_root":
                origin = [0, 0]
                angle = 0
                local_length_scale = 1
                recursive_length_scale = [1, 1] # x and y axis
                return origin, angle, local_length_scale, recursive_length_scale

            elif self.param[0].tag == "add":
                ret = self.subparams["add"].subparams["lhs"].__get_value(frame)
                ret2 = self.subparams["add"].subparams["rhs"].__get_value(frame)
                mul = self.subparams["add"].subparams["scalar"].__get_value(frame)
                if isinstance(ret, list):
                    ret[0] += ret2[0]
                    ret[1] += ret2[1]
                    ret = [it*mul for it in ret]
                else:
                    ret += ret2
                    ret *= mul

            elif self.param[0].tag == "exp":
                exp = self.subparams["exp"].subparams["exp"].__get_value(frame)
                scale = self.subparams["exp"].subparams["scale"].__get_value(frame)
                ret = scale * math.exp(exp)

            elif self.param[0].tag == "average":
                lst = self.subparams["average"].subparams["entry"]
                if not isinstance(lst, list):
                    lst = [lst]

                ret = [0, 0]
                if not isinstance(lst[0].__get_value(frame), list):
                    ret = 0
                for it in lst:
                    val = it.__get_value(frame)
                    if isinstance(val, list):
                        ret[0], ret[1] = ret[0] + val[0], ret[1] + val[1]
                    else:
                        ret += val
                if isinstance(ret, list):
                    ret[0], ret[1] = ret[0] / len(lst), ret[1] / len(lst)
                else:
                    ret /= float(len(lst))

            elif self.param[0].tag == "weighted_average":
                self.subparams["weighted_average"].extract_subparams()
                lst = self.subparams["weighted_average"].subparams["entry"]
                if not isinstance(lst, list):   # When only one entry is present
                    lst = [lst]

                ret = [0, 0]
                den = 0
                if not isinstance(lst[0].subparams["weighted_vector"].subparams["value"].__get_value(frame), list):
                    ret = 0
                for it in lst:
                    weight = it.subparams["weighted_vector"].subparams["weight"].__get_value(frame)
                    value = it.subparams["weighted_vector"].subparams["value"].__get_value(frame)
                    den += weight
                    if isinstance(value, list):
                        ret[0], ret[1] = ret[0] + value[0]*weight, ret[1] + value[1]*weight
                    else:
                        ret += value*weight
                if isinstance(ret, list):
                    ret[0], ret[1] = ret[0] / den, ret[1] / den
                else:
                    ret /= float(den)

            elif self.param[0].tag == "composite":  # Only available for vectors
                x = self.subparams["composite"].subparams["x"].__get_value(frame)
                y = self.subparams["composite"].subparams["y"].__get_value(frame)
                ret = [x, y]

            elif self.param[0].tag == "linear":
                slope = self.subparams["linear"].subparams["slope"].__get_value(frame)
                offset = self.subparams["linear"].subparams["offset"].__get_value(frame)
                if isinstance(slope, list):
                    ret = [0, 0]
                    ret[0] = offset[0] + slope[0]*(frame/settings.lottie_format["fr"])
                    ret[1] = offset[1] + slope[1]*(frame/settings.lottie_format["fr"])
                else:
                    ret = offset + slope*(frame/settings.lottie_format["fr"])

            elif self.param[0].tag == "radial_composite":   # Only for vectors
                rad = self.subparams["radial_composite"].subparams["radius"].__get_value(frame)
                angle = to_Synfig_axis(self.subparams["radial_composite"].subparams["theta"].__get_value(frame), "angle")
                angle = math.radians(angle)
                x = rad * math.cos(angle)
                y = rad * math.sin(angle)
                ret = [x, y]

            elif self.param[0].tag == "scale":
                link = self.subparams["scale"].subparams["link"].__get_value(frame)
                scalar = self.subparams["scale"].subparams["scalar"].__get_value(frame)
                if isinstance(link, list):
                    link[0] *= scalar
                    link[1] *= scalar
                else:
                    link *= scalar
                ret = link

            elif self.param[0].tag == "subtract":
                lhs = self.subparams["subtract"].subparams["lhs"].__get_value(frame)
                rhs = self.subparams["subtract"].subparams["rhs"].__get_value(frame)
                scalar = self.subparams["subtract"].subparams["scalar"].__get_value(frame)
                if isinstance(lhs, list):
                    ret = [0, 0]
                    ret[0] = (lhs[0] - rhs[0]) * scalar
                    ret[1] = (lhs[1] - rhs[1]) * scalar
                else:
                    ret = (lhs - rhs) * scalar

            elif self.param[0].tag == "switch":
                link_off = self.subparams["switch"].subparams["link_off"].__get_value(frame)
                link_on = self.subparams["switch"].subparams["link_on"].__get_value(frame)
                switch = self.subparams["switch"].subparams["switch"].__get_value(frame)
                if isinstance(link_on, list):
                    ret = [0, 0]
                    ret[0] = link_on[0] * switch + link_off[0] * (1 - switch)
                    ret[1] = link_on[1] * switch + link_off[1] * (1 - switch)
                else:
                    ret = link_on * switch + link_off * (1 - switch)

            elif self.param[0].tag == "bone_link":
                guid = self.subparams["bone_link"].subparams["bone"][0].attrib["guid"]
                bone = self.get_bone_from_canvas(guid)
                ret_origin, ret_angle, lls, rls = bone.__get_value(frame)

                # Adding the base value effect here
                base_value = self.subparams["bone_link"].subparams["base_value"].__get_value(frame)
                a1, a2 = math.radians(ret_angle), math.radians(ret_angle+90)
                ret = ret_origin

                # base_value to be arranged according to the local scale
                base_value = [lls*i for i in base_value]

                ret[0] = ret[0] + (base_value[0] * math.cos(a1) - base_value[1] * math.cos(a2)) * rls[0]
                ret[1] = ret[1] + (base_value[0] * math.sin(a1) - base_value[1] * math.sin(a2)) * rls[1]

                ret = [ret[0], ret[1]]

            elif self.param[0].tag == "sine":
                angle = self.subparams["sine"].subparams["angle"].__get_value(frame)
                amp = self.subparams["sine"].subparams["amp"].__get_value(frame)
                angle = math.radians(angle)
                
                if isinstance(amp, list):
                    ret = [0, 0]

                    ret[0] = math.sin(angle) * amp[0]
                    ret[1] = math.sin(angle) * amp[1]
                else:
                    ret = math.sin(angle)*amp

            elif self.param[0].tag == "cos":
                angle = self.subparams["cos"].subparams["angle"].__get_value(frame)
                amp = self.subparams["cos"].subparams["amp"].__get_value(frame)
                angle = math.radians(angle)
                if isinstance(amp, list):
                    ret = [0, 0]
                    ret[0] = math.cos(angle) * amp[0]
                    ret[1] = math.cos(angle) * amp[1]
                else:
                    ret = math.cos(angle)*amp

            elif self.param[0].tag == "fromint":
                link = self.subparams["fromint"].subparams["link"].__get_value(frame)
                if isinstance(link, list):
                    ret = [0, 0]
                    ret[0] = round(link[0])*settings.PIX_PER_UNIT
                    ret[1] = round(link[1])*settings.PIX_PER_UNIT
                else:
                    ret = round(link)*settings.PIX_PER_UNIT

            elif self.param[0].tag == "atan2":
                y = self.subparams["atan2"].subparams["y"].__get_value(frame)
                x = self.subparams["atan2"].subparams["x"].__get_value(frame)
                rad = math.pi/180
                ret = math.atan2(y,x)/rad

            elif self.param[0].tag == "vectorangle":
                vector = self.subparams["vectorangle"].subparams["vector"].__get_value(frame)
                rad = math.pi/180
                ret = math.atan2(vector[1],vector[0])/rad

        else:
            ret = self.get_single_value(frame)
            if isinstance(ret, list):
                # Need to change the calculation inside get_single_value, this
                # is just a hack
                ret = [ret[0], -ret[1]]
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

    def scale_convert_link(self, val):
        """
        Private method for inserting <scale></scale>, given the multiplication value
        """
        st = "<scale type='real'><link></link><scalar><real value='1.00'/></scalar></scale>"
        root = etree.fromstring(st)
        first = copy.deepcopy(self.param[0])
        root[0].append(first)
        root[1][0].attrib["value"] = str(val)
        self.param[0].getparent().remove(self.param[0])
        self.param.append(root)
        self.SUBPARAMS_EXTRACTED = 0

    def update_frame_window(self, window):
        """
        Given an animation, find the minimum and maximum frame at which the
        waypoints are located
        """
        self.extract_subparams()
        if self.param.tag in {"bone", "bone_root"}:
            node = self.param
        else:
            node = self.param[0]

        if node.tag in settings.BONES:
            if node.tag == "bone":
                self.subparams["origin"].update_frame_window(window)
                self.subparams["angle"].update_frame_window(window)
                self.subparams["scalelx"].update_frame_window(window)
                parent_guid = self.subparams["parent"][0].attrib["guid"]
                canvas = self.get_canvas()
                bone = canvas.get_bone(parent_guid)
                bone.update_frame_window(window)

        # Time updation for converted nodes:
        if node.tag in settings.CONVERT_METHODS:
            if node.tag in {"add", "subtract"}:
                key = node.tag
                self.subparams[key].extract_subparams()
                self.subparams[key].subparams["lhs"].update_frame_window(window)
                self.subparams[key].subparams["rhs"].update_frame_window(window)
                self.subparams[key].subparams["scalar"].update_frame_window(window)

            elif node.tag == "switch":
                key = node.tag
                self.subparams[key].extract_subparams()
                self.subparams[key].subparams["link_off"].update_frame_window(window)
                self.subparams[key].subparams["link_on"].update_frame_window(window)
                self.subparams[key].subparams["switch"].update_frame_window(window)

            elif node.tag == "average":
                if not isinstance(self.subparams["average"], list):
                    self.subparams["average"] = [self.subparams["average"]]
                for it in self.subparams["average"]:
                    it.update_frame_window(window)

            elif node.tag == "exp":
                self.subparams["exp"].extract_subparams()
                self.subparams["exp"].subparams["exp"].update_frame_window(window)
                self.subparams["exp"].subparams["scale"].update_frame_window(window)

            elif node.tag == "weighted_average":
                self.subparams["weighted_average"].extract_subparams()
                for it in self.subparams["weighted_average"].subparams["entry"]:
                    it.extract_subparams()
                    it.subparams["weighted_vector"].extract_subparams()
                    ti = it.subparams["weighted_vector"].subparams
                    ti["weight"].update_frame_window(window)
                    ti["value"].update_frame_window(window)

            elif node.tag == "composite":
                self.subparams["composite"].extract_subparams()
                self.subparams["composite"].subparams["x"].update_frame_window(window)
                self.subparams["composite"].subparams["y"].update_frame_window(window)

            elif node.tag == "linear":
                window["first"] = settings.lottie_format["ip"]
                window["last"] = settings.lottie_format["op"]

            elif node.tag == "radial_composite":
                self.subparams["radial_composite"].extract_subparams()
                self.subparams["radial_composite"].subparams["radius"].update_frame_window(window)
                self.subparams["radial_composite"].subparams["theta"].update_frame_window(window)

            elif node.tag == "scale":
                self.subparams["scale"].extract_subparams()
                self.subparams["scale"].subparams["link"].update_frame_window(window)
                self.subparams["scale"].subparams["scalar"].update_frame_window(window)

            elif node.tag == "bone_link":
                self.subparams["bone_link"].extract_subparams()
                guid = self.subparams["bone_link"].subparams["bone"][0].attrib["guid"]
                bone = self.get_bone_from_canvas(guid)
                bone.update_frame_window(window)
                self.subparams["bone_link"].subparams["base_value"].update_frame_window(window)

            elif node.tag == "sine":
                self.subparams["sine"].extract_subparams()
                self.subparams["sine"].subparams["angle"].update_frame_window(window)
                self.subparams["sine"].subparams["amp"].update_frame_window(window)

            elif node.tag == "cos":
                self.subparams["cos"].extract_subparams()
                self.subparams["cos"].subparams["angle"].update_frame_window(window)
                self.subparams["cos"].subparams["amp"].update_frame_window(window)

            elif node.tag == "fromint":
                self.subparams["fromint"].extract_subparams()
                self.subparams["fromint"].subparams["link"].update_frame_window(window)

            elif node.tag == "atan2":
                self.subparams["atan2"].extract_subparams()
                self.subparams["atan2"].subparams["y"].update_frame_window(window)
                self.subparams["atan2"].subparams["x"].update_frame_window(window)

            elif node.tag == "vectorangle":
                self.subparams["vectorangle"].extract_subparams()
                self.subparams["vectorangle"].subparams["vector"].update_frame_window(window)

        if is_animated(node) == 2:
            for waypoint in node:
                fr = common.misc.get_frame(waypoint)
                if fr > window["last"]:
                    window["last"] = fr
                if fr < window["first"]:
                    window["first"] = fr

    def get_bone_from_canvas(self, guid):
        """
        Given a canvas, recursively travel to the ancestor canvas's and find the
        bone associated with the guid
        """
        now = self
        while True:
            layer = now.get_layer()
            canvas = layer.getparent()
            bone = canvas.get_bone(guid)
            if bone is not None:
                break
            else:
                now = canvas.getparent_param()
        return bone
