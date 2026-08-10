import sys
#from lxml import etree
import settings
sys.path.append("..")

class Param:
    def __init__(self, param, parent):
        self.parent = parent
        self.param = param
        self.subparams = {} # dictionary to hold sub-parameters keyed by their tag names
        self.expression_controllers = [] # holds any transformation expressions (if needed)
        self.expression = "" # a string representation of the computed expression
        self.dimension = 1 # 1 for scalar, 2 for vector values

        # For bone-related nodes, we extract their sub parameters immediately.
        if self.param.tag in settings.BONES or (len(self.param) and self.param[0].tag in settings.CONVERT_METHODS):
            self.extract_subparams()

    def extract_subparams(self):
        """
        Extract all direct sub-parameters, wrapping each child element as a Param
        object keyed by its tag name.
        """
        for child in self.param:
            # Use the child node's tag as the key.
            self.subparams[child.tag] = Param(child, self)

    def get_canvas(self):
        """
        Traverse upward to get the Canvas object.
        This function relies on the parent having a get_canvas() method.
        """
        if hasattr(self.parent, 'get_canvas'):
            return self.parent.get_canvas()
        return None

    def recur_animate(self, anim_type):
        """
        Recursively computes the transformation for a bone node.
        For spine-export we want to extract the origin (position), angle (rotation)
        and scale factors.
        Depending on the node type, this function:
        - For a "bone": extracts 'origin', 'angle', 'scalelx' (local scale) and 'scalex' (recursive scale).
        - For a "bone_root": returns default values.
        - For a "bone_link": lookups a referenced bone and applies a base offset.
        This implementation currently returns transformation expressions as strings;
        these may later be evaluated or directly computed if you choose a fixed frame.
        """
        # Check for bone-specific tags using settings (e.g., settings.BONES).
        if self.param.tag in settings.BONES or (len(self.param) and self.param[0].tag in settings.CONVERT_METHODS):

            # CASE 1: bone node
            if self.param.tag == "bone":
                # Retrieve subparameters if they exist.
                origin_param  = self.subparams.get("origin")
                angle_param   = self.subparams.get("angle")
                scalelx_param = self.subparams.get("scalelx")
                scalex_param  = self.subparams.get("scalex")

                # For basic exporting, if a subparameter is missing, use a default value.
                bone_origin = origin_param.recur_animate("vector")[0] if origin_param else "[0,0]"
                bone_angle  = angle_param.recur_animate("angle")[0]  if angle_param  else "0"
                lls         = scalelx_param.recur_animate("scalar")[0] if scalelx_param else "1"
                rls         = scalex_param.recur_animate("scalar")[0]  if scalex_param  else "1"

                # Check if there is a defined parent
                parent_param = self.subparams.get("parent")
                if parent_param:
                    # Expect parent to have a reference guid attribute.
                    guid = parent_param.param.attrib.get("guid", "")
                    canvas = self.get_canvas()
                    parent_bone = canvas.get_bone(guid)
                    if parent_bone is not None:
                        # Recursively get the parent's transformation values.
                        par_origin, par_angle, par_lls, par_rls = parent_bone.recur_animate("vector")
                        # Combine the parent's values and the current bone's own values.
                        # For a real implementation, use proper vector math
                        # here we simply form an expression string.)
                        combined_origin = "add({par_origin}, mul({bone_origin}, {par_lls}, {par_rls}))".format(
                            par_origin=par_origin, bone_origin=bone_origin, par_lls=par_lls, par_rls=par_rls
                        )
                        combined_angle = "add({par_angle},{bone_angle})".format(par_angle=par_angle, bone_angle=bone_angle)
                        return combined_origin, combined_angle, lls, rls, self.expression_controllers
                # If no parent is defined, return the bone’s own values.
                return bone_origin, bone_angle, lls, rls, self.expression_controllers

            # CASE 2: bone_root node; the root uses default transformation.
            elif self.param.tag == "bone_root":
                return "[0, 0]", "0", "1", "1", []

            # CASE 3: bone_link node: lookup a referenced bone and add a base offset.
            elif len(self.param) and self.param[0].tag == "bone_link":
                bone_link_param = self.subparams.get("bone_link")
                if bone_link_param:
                    guid_param = bone_link_param.subparams.get("bone")
                    if guid_param:
                        guid_val = guid_param.param.attrib.get("guid", "")
                        canvas = self.get_canvas()
                        linked_bone = canvas.get_bone(guid_val)
                        base_value_param = bone_link_param.subparams.get("base_value")
                        base_value = base_value_param.recur_animate("vector")[0] if base_value_param else "[0,0]"
                        # Combine the linked bone's origin with the base offset.
                        linked_origin, linked_angle, _, _, _ = linked_bone.recur_animate("vector")
                        ret_origin = "add({linked_origin},{base_value})".format(
                            linked_origin=linked_origin, base_value=base_value
                        )
                        return ret_origin, linked_angle, "1", "1", self.expression_controllers
                return "[0, 0]", "0", "1", "1", []

        # If the parameter is not bone-related, return its basic text.
        return self.param.text, "", "","", []

    def __get_value(self, frame):
        """
        Computes a concrete set of transformation values at a given frame.
        For a bone node, this method returns:
        (origin, angle, local scale, recursive scale)
        If a bone has a parent or is linked, the values are computed by
        combining the parent's values.
        """
        if self.param.tag in settings.BONES or (len(self.param) and self.param[0].tag in settings.CONVERT_METHODS):
            if self.param.tag == "bone":
                # Retrieve numeric values for each subparameter.
                origin_value = self.subparams.get("origin").__get_value(frame) if "origin" in self.subparams else [0, 0]
                angle_value  = self.subparams.get("angle").__get_value(frame)  if "angle" in self.subparams else 0
                lls_value    = self.subparams.get("scalelx").__get_value(frame) if "scalelx" in self.subparams else 1
                rls_value    = self.subparams.get("scalex").__get_value(frame)  if "scalex" in self.subparams else 1
                parent_param = self.subparams.get("parent")
                if parent_param:
                    guid = parent_param.param.attrib.get("guid", "")
                    canvas = self.get_canvas()
                    parent_bone = canvas.get_bone(guid)
                    if parent_bone is not None:
                        parent_origin, parent_angle, _, _ = parent_bone.__get_value(frame)
                        # Combine the parent's origin with the current origin.
                        final_origin = [parent_origin[0] + origin_value[0], parent_origin[1] + origin_value[1]]
                        final_angle  = parent_angle + angle_value
                        return final_origin, final_angle, lls_value, rls_value
                return origin_value, angle_value, lls_value, rls_value

            elif self.param.tag == "bone_root":
                return [0, 0], 0, 1, [1, 1]

            elif len(self.param) and self.param[0].tag == "bone_link":
                bone_link_param = self.subparams.get("bone_link")
                if bone_link_param:
                    guid_param = bone_link_param.subparams.get("bone")
                    if guid_param:
                        guid_val = guid_param.param.attrib.get("guid", "")
                        canvas = self.get_canvas()
                        linked_bone = canvas.get_bone(guid_val)
                        parent_origin, parent_angle, _, _ = linked_bone.__get_value(frame)
                        base_value = bone_link_param.subparams.get("base_value").__get_value(frame) if "base_value" in bone_link_param.subparams else [0, 0]
                        final_origin = [parent_origin[0] + base_value[0], parent_origin[1] + base_value[1]]
                        return final_origin, parent_angle, 1, [1, 1]
                return [0, 0], 0, 1, [1, 1]

        # For non-bone parameters, attempt to return a numeric value.
        try:
            return float(self.param.text)
        except (ValueError, TypeError):
            return self.param.text