"""
"""

import sys
import ast
import settings
from misc import change_axis, get_vector, is_animated
from shapes.rectangle import gen_dummy_waypoint, get_vector_at_frame
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
sys.path.append("../")


def animate_radial_composite(radial_composite, window):
    """
    Animates the radial composite and updates the window of frame if radial
    composite's parameters are already animated
    Also generate the Lottie path and stores in radial_composite
    """
    for child in radial_composite:
        if child.tag == "radius":
            radius = child
        elif child.tag == "theta":
            theta = child
    update_frame_window(radius[0], window)
    update_frame_window(theta[0], window)

    radius = gen_dummy_waypoint(radius, is_animated(radius[0]), "real")
    theta = gen_dummy_waypoint(theta, is_animated(theta[0]), "real")

    # Empty the radial_composite and fill in new values
    for child in radial_composite:
        child.getparent().remove(child)
    radial_composite.insert(0, radius)
    radial_composite.insert(1, theta)

    # Generating the radial path and store in the lxml element
    radius_dict = {}
    gen_value_Keyframed(radius_dict, radius[0], 0)
    # Store in lxml element
    radius_lxml = etree.Element("radius_path")
    radius_lxml.text = str(radius_dict)
    radial_composite.append(radius_lxml)

    # Generating the theta path and store in the lxml element
    theta_dict = {}
    gen_value_Keyframed(theta_dict, theta[0], 0)
    # Store in lxml element
    theta_lxml = etree.Element("theta_path")
    theta_lxml.text = str(theta_dic)
    radial_composite.append(theta_lxml)


def update_frame_window(node, window):
    if is_animated(node) == 2:
        for waypoint in node:
            fr = get_frame(waypoint)
            if fr > window["last"]:
                window["last"] = fr
            if fr < window["first"]:
                window["first"] = fr


def gen_properties_shapePropKeyframe(lottie, bline_point):
    """
    """
    # Assuming split angle and split radius both are ticked for now

    ################### SECTION 2 #########################
    # Inserting waypoints if not animated and finding the first and last frame
    # AFter that, there path will be calculated in lottie format which can
    # latter be used in get_vector_at_frame() function
    window = {}
    window["first"] = sys.maxsize
    window["last"] = -1

    for entry in bline_point:
        composite = entry[0]
        for child in composite:
            if child.tag == "point":
                pos = child
            elif child.tag == "t1":
                t1 = child
            elif child.tag == "t2":
                t2 = child
                
        # Necassary to update this before inserting new waypoints, as new
        # waypoints might include there on time: 0 seconds
        update_frame_window(pos[0], window)

        # Empty the pos and fill in the new animated pos
        pos = gen_dummy_waypoint(pos, is_animated(pos[0]), "vector")
        for child in composite:
            if child.tag == "point":
                child.getparent().remove(child)
        composite.insert(0, pos)

        # Generate path for Lottie format
        path_dict = {}
        gen_properties_multi_dimensional_keyframed(path_dict, pos[0], 0)
        # Store in lxml element
        path_lxml = etree.Element("point_path")
        path_lxml.text = str(path_dict)
        composite.append(path_dict)

        animate_radial_composite(t1[0], window)
        animate_radial_composite(t2[0], window)

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window 
    fr = window["first"]
    while fr <= window["last"]:
        lottie.append({})
        lottie[-1]["i"], lottie[-1]["o"] = {}, {}
        lottie[-1]["i"]["x"] = lottie[-1]["i"]["y"] = 0.5   # Does not matter because frames are adjacent
        lottie[-1]["o"]["x"] = lottie[-1]["o"]["y"] = 0.5   # Does not matter because frames are adjacent
        lottie[-1]["t"] = fr
        lottie[-1]["s"], lottie[-1]["e"] = [], []           # Start and end value of the path
        st_val, en_val = lottie[-1]["s"], lottie[-1]["e"]
        st_val.append({})
        en_val.append({})
        st_val, en_val = st_val[0], en_val[0]
        st_val["i"], st_val["o"], st_val["v"], st_val["c"] = [], [], [], False
        en_val["i"], en_val["o"], en_val["v"], en_val["c"] = [], [], [], False
        for entry in bline_points:
            composite = entry[0]
            for child in composite:
                if child.tag == "point_path":
                    pos = get_vector_at_frame(ast.literal_eval(child.text), fr) 
                    st_val["v"].append(pos)
                elif child.tag == "t1":
                    pass
                elif child.tag == "t2":
                    pass
