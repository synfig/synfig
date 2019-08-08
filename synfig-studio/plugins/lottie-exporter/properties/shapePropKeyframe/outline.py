# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of outline layer
in Lottie format
"""

import sys
import math
import settings
from common.Bline import Bline
from common.Param import Param
from common.Vector import Vector
from common.Hermite import Hermite
from synfig.animation import to_Synfig_axis
from properties.shapePropKeyframe.helper import add_reverse, add, move_to, get_tangent_at_frame, insert_dict_at, animate_tangents
sys.path.append("../../")


def gen_bline_outline(lottie, bline_point):
    """
    Generates the bline corresponding to outline layer by adding some vertices
    to bline and converting it to region layer

    Some parts are common with gen_bline_shapePropKeyframe(), which will be
    placed in a common function latter

    Args:
        lottie (dict) : Lottie format outline layer will be stored in this
        bline_point (common.Param.Param) : Synfig format bline points

    Returns:
        (None)
    """
    ################### SECTION 1 #########################
    # Inserting waypoints if not animated and finding the first and last frame
    window = {}
    window["first"] = sys.maxsize
    window["last"] = -1

    bline = Bline(bline_point[0], bline_point)

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

    layer = bline.get_layer().get_layer()
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

    # Animaing the homogenous width
    homo_width.update_frame_window(window)
    homo_width.animate_without_path("bool")

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window


    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)  # This loop needs to be considered somewhere down

        synfig_outline(bline, st_val, origin, outer_width, sharp_cusps, expand, r_tip0, r_tip1, homo_width, fr)
        synfig_outline(bline, en_val, origin, outer_width, sharp_cusps, expand, r_tip0, r_tip1, homo_width, fr + 1)

        fr += 1
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr


def get_outline_grow(fr):
    """
    Gives the value of outline grow parameter at a particular frame
    """
    ret = 0
    for og in settings.OUTLINE_GROW:
        if isinstance(og, (float, int)):
            ret += og
        else:   # should be dict
            val = to_Synfig_axis(og.get_value(fr), "real")
            ret += val
    ret = math.e ** ret
    return ret


def get_outline_param_at_frame(entry, fr):
    """
    Given a entry and frame, returns the parameters of the outline layer at
    that frame

    Args:
        entry (dict) : Vertex of outline layer in Synfig format
        fr        (int)                 : frame number

    Returns:
        (common.Vector.Vector) : position of the vertex
        (float)       : width of the vertex
        (common.Vector.Vector) : Tangent 1 of the vertex
        (common.Vector.Vector) : Tangent 2 of the vertex
        (bool)        : True if radius split is ticked at this frame
        (bool)        : True if tangent split is ticked at this frame
    """
    pos = entry["point"].get_value(fr)
    # Convert pos back to Synfig coordinates
    pos = to_Synfig_axis(pos, "vector")
    pos_ret = Vector(pos[0], pos[1])

    width = entry["width"].get_value(fr)
    width = to_Synfig_axis(width, "real")
    t1 = entry["t1"]
    t2 = entry["t2"]
    split_r = entry["split_radius"]
    split_a = entry["split_angle"]


    t1, t2 = get_tangent_at_frame(t1, t2, split_r, split_a, fr)
    # Convert to Synfig units
    t1 /= settings.PIX_PER_UNIT
    t2 /= settings.PIX_PER_UNIT

    split_r_val = split_r.get_value(fr)
    split_a_val = split_a.get_value(fr)

    return pos_ret, width, t1, t2, split_r_val, split_a_val


def synfig_outline(bline, st_val, origin_p, outer_width_p, sharp_cusps_p, expand_p, r_tip0_p, r_tip1_p, homo_width_p, fr):
    """
    Calculates the points for the outline layer as in Synfig:
    https://github.com/synfig/synfig/blob/678cc3a7b1208fcca18c8b54a29a20576c499927/synfig-core/src/modules/mod_geometry/outline.cpp

    Args:
        bline_point (common.Bline.Bline) : Synfig format bline points of outline layer
        st_val (dict) : Lottie format outline stored in this
        origin_p (common.Param.Param) : Lottie format origin of outline layer
        outer_width_p (common.Param.Param) : Lottie format outer width
        sharp_cusps_p (common.Param.Param) : sharp cusps in Synfig format
        expand_p (common.Param.Param) : Lottie format expand parameter
        r_tip0_p (common.Param.Param) : Round tip[0] in Synfig format
        r_tip1_p (common.Param.Param) : Round tip[1] in Synfig format
        homo_width_p (common.Param.Param) : Homogeneous width in Synfig format
        fr (int) : Frame number

    Returns:
        (None)
    """

    EPSILON = 0.000000001
    SAMPLES = 50
    CUSP_TANGENT_ADJUST = 0.025
    CUSP_THRESHOLD = 0.40
    SPIKE_AMOUNT = 4
    ROUND_END_FACTOR = 4

    outer_width = to_Synfig_axis(outer_width_p.get_value(fr), "real")
    expand = to_Synfig_axis(expand_p.get_value(fr), "real")
    sharp_cusps = sharp_cusps_p.get_value(fr)
    r_tip0 = r_tip0_p.get_value(fr)
    r_tip1 = r_tip1_p.get_value(fr)
    homo_width = homo_width_p.get_value(fr)

    gv = get_outline_grow(fr)

    # Setup chunk list
    side_a, side_b = [], []

    # Check if looped
    loop = bline.get_loop()

    # Iterators
    end_it = bline.get_len()
    next_it = 0
    if loop:
        iter_it = end_it - 1
    else:
        iter_it = next_it
        next_it += 1

    first_point = bline[iter_it]
    first_tangent = get_outline_param_at_frame(bline[0], fr)[3]
    last_tangent = get_outline_param_at_frame(first_point, fr)[2]

    # If we are looped and drawing sharp cusps, we 'll need a value
    # for the incoming tangent. This code fails if we have a "degraded" spline
    # with just one vertex, so we avoid such case.
    if loop and sharp_cusps and last_tangent.is_equal_to(Vector(0, 0)) and bline.get_len() > 1:
        prev_it = iter_it
        prev_it -= 1
        prev_it %= bline.get_len()
        prev_point = bline[prev_it]
        curve = Hermite(get_outline_param_at_frame(prev_point, fr)[0],
                        get_outline_param_at_frame(first_point, fr)[0],
                        get_outline_param_at_frame(prev_point, fr)[3],
                        get_outline_param_at_frame(first_point, fr)[2])
        last_tangent = curve.derivative(1.0 - CUSP_TANGENT_ADJUST)

    first = not loop
    while next_it != end_it:
        bp1 = bline[iter_it]
        bp2 = bline[next_it]

        # Calculate current vertex and next vertex parameters
        pos_c, width_c, t1_c, t2_c, split_r_c, split_a_c = get_outline_param_at_frame(bp1, fr)
        pos_n, width_n, t1_n, t2_n, split_r_n, split_a_n = get_outline_param_at_frame(bp2, fr)

        # Setup tangents
        prev_t = t1_c
        iter_t = t2_c
        next_t = t1_n

        split_flag = split_a_c or split_r_c

        # If iter_it.t2 == 0 and next.t1 == 0, this is a straight line
        if iter_t.is_equal_to(Vector(0, 0)) and next_t.is_equal_to(Vector(0, 0)):
            iter_t = next_t = pos_n - pos_c

            # If the 2 points are on top of each other, ignore this segment
            # leave 'first' true if was before
            if iter_t.is_equal_to(Vector(0, 0)):
                iter_it = next_it
                iter_it %= bline.get_len()
                next_it += 1
                continue

        # Setup the curve
        curve = Hermite(pos_c, pos_n, iter_t, next_t)

        # Setup width's
        iter_w = gv*(width_c * outer_width * 0.5 + expand)
        next_w = gv*(width_n * outer_width * 0.5 + expand)

        if first:
            first_tangent = curve.derivative(CUSP_TANGENT_ADJUST)

        # Make cusps as necassary
        if not first and \
           sharp_cusps and \
           split_flag and \
           ((not prev_t.is_equal_to(iter_t)) or (iter_t.is_equal_to(Vector(0, 0)))) and \
           (not last_tangent.is_equal_to(Vector(0, 0))):

            curr_tangent = curve.derivative(CUSP_TANGENT_ADJUST)
            t1 = last_tangent.perp().norm()
            t2 = curr_tangent.perp().norm()

            cross = t1*t2.perp()
            perp = (t1 - t2).mag()
            if cross > CUSP_THRESHOLD:
                p1 = pos_c + t1*iter_w
                p2 = pos_c + t2*iter_w
                side_a.append([line_intersection(p1, last_tangent, p2, curr_tangent), Vector(0, 0), Vector(0, 0)])
            elif cross < -CUSP_THRESHOLD:
                p1 = pos_c - t1*iter_w
                p2 = pos_c - t2*iter_w
                side_b.append([line_intersection(p1, last_tangent, p2, curr_tangent), Vector(0, 0), Vector(0, 0)])
            elif cross > 0.0 and perp > 1.0:
                amount = max(0.0, cross/CUSP_THRESHOLD) * (SPIKE_AMOUNT - 1.0) + 1.0
                side_a.append([pos_c + (t1 + t2).norm()*iter_w*amount, Vector(0, 0), Vector(0, 0)])
            elif cross < 0 and perp > 1:
                amount = max(0.0, -cross/CUSP_THRESHOLD) * (SPIKE_AMOUNT - 1.0) + 1.0
                side_b.append([pos_c - (t1 + t2).norm()*iter_w*amount, Vector(0, 0), Vector(0, 0)])

        # Precalculate positions and coefficients
        length = 0.0
        points = []
        dists = []
        n = 0.0
        itr = 0
        while n < 1.000001:
            points.append(curve.value(n))
            if n:
                length += (points[itr] - points[itr-1]).mag()
            dists.append(length)

            n += 1.0/SAMPLES
            itr += 1
        length += (curve.value(1) - points[itr-1]).mag()

        div_length = 1
        if length > EPSILON:
            div_length = 1.0 / length

        # Might not need /3 for the tangents genereated finally - VERY IMPORTANT
        # Make the outline
        pt = curve.derivative(CUSP_TANGENT_ADJUST) / 3
        n = 0.0
        itr = 0
        while n < 1.000001:
            t = curve.derivative(min(max(n, CUSP_TANGENT_ADJUST), 1.0 - CUSP_TANGENT_ADJUST)) / 3
            d = t.perp().norm()
            k = dists[itr] * div_length
            if not homo_width:
                k = n
            w = (next_w - iter_w)*k + iter_w
            if False and n:
                # create curve
                a = points[itr-1] + d*w
                b = points[itr] + d*w
                tk = (b - a).mag() * div_length
                side_a.append([b, pt*tk, -t*tk])

                a = points[itr-1] - d*w
                b = points[itr] - d*w
                tk = (b - a).mag() * div_length
                side_b.append([b, pt*tk, -t*tk])
            else:
                side_a.append([points[itr] + d*w, Vector(0, 0), Vector(0, 0)])
                side_b.append([points[itr] - d*w, Vector(0, 0), Vector(0, 0)])
            pt = t
            itr += 1
            n += 1.0/SAMPLES

        last_tangent = curve.derivative(1.0 - CUSP_TANGENT_ADJUST)
        side_a.append([curve.value(1.0) + last_tangent.perp().norm()*next_w, Vector(0, 0), Vector(0, 0)])
        side_b.append([curve.value(1.0) - last_tangent.perp().norm()*next_w, Vector(0, 0), Vector(0, 0)])
        first = False

        iter_it = next_it
        iter_it %= bline.get_len()
        next_it += 1

    if len(side_a) < 2 or len(side_b) < 2:
        return

    origin_cur = origin_p.get_value(fr)
    move_to(side_a[0][0], st_val, origin_cur)

    if loop:
        add(side_a, st_val, origin_cur)
        add_reverse(side_b, st_val, origin_cur)
    else:
        # Insert code for adding end tip
        if r_tip1:
            bp = bline[-1]
            vertex = get_outline_param_at_frame(bp, fr)[0]
            tangent = last_tangent.norm()
            w = gv*(get_outline_param_at_frame(bp, fr)[1] * outer_width * 0.5 + expand)

            a = vertex + tangent.perp()*w
            b = vertex - tangent.perp()*w
            p1 = a + tangent*w*(ROUND_END_FACTOR/3.0)
            p2 = b + tangent*w*(ROUND_END_FACTOR/3.0)
            tan = tangent*w*(ROUND_END_FACTOR/3.0)

            # replace the last point
            side_a[-1] = [a, Vector(0, 0), tan]
            add(side_a, st_val, origin_cur)
            add([[b, -tan, Vector(0, 0)]], st_val, origin_cur)
        else:
            add(side_a, st_val, origin_cur)

        # Insert code for adding beginning tip
        if r_tip0:
            bp = bline[0]
            vertex = get_outline_param_at_frame(bp, fr)[0]
            tangent = first_tangent.norm()
            w = gv*(get_outline_param_at_frame(bp, fr)[1] * outer_width * 0.5 + expand)

            a = vertex - tangent.perp()*w
            b = vertex + tangent.perp()*w
            p1 = a - tangent*w*(ROUND_END_FACTOR/3.0)
            p2 = b - tangent*w*(ROUND_END_FACTOR/3.0)
            tan = -tangent*w*(ROUND_END_FACTOR/3.0)

            # replace the first point
            side_b[0] = [a, Vector(0, 0), tan]
            add_reverse(side_b, st_val, origin_cur)
            add([[b, -tan, Vector(0, 0)]], st_val, origin_cur)
        else:
            add_reverse(side_b, st_val, origin_cur)


def line_intersection(p1, t1, p2, t2):
    """
    This function was adapted from what was
    described on http://www.whisqu.se/per/docs/math28.htm

    Args:
        p1 (common.Vector.Vector) : First point
        t1 (common.Vector.Vector) : First tangent
        p2 (common.Vector.Vector) : Second point
        t2 (common.Vector.Vector) : Second tangent

    Returns:
        (common.Vector.Vector) : intersection of the both the lines
    """
    x0 = p1[0]
    y0 = p1[1]

    x1 = p1[0] + t1[0]
    y1 = p1[1] + t1[1]

    x2 = p2[0]
    y2 = p2[1]

    x3 = p2[0] + t2[0]
    y3 = p2[1] + t2[1]

    near_infinity = 1e+10

    # compute slopes, not the kluge for infinity, however, this will
    # be close enough

    if (x1 - x0) != 0:
        m1 = (y1 - y0) / (x1 - x0)
    else:
        m1 = near_infinity

    if (x3 - x2) != 0:
        m2 = (y3 - y2) / (x3 - x2)
    else:
        m2 = near_infinity

    a1 = m1
    a2 = m2
    b1 = -1.0
    b2 = -1.0
    c1 = y0 - m1*x0
    c2 = y2 - m2*x2

    # compute the inverse of the determinate
    det_inv = 1.0 / (a1*b2 - a2*b1)

    # Use kramer's rule to compute intersection
    return Vector((b1*c2 - b2*c1)*det_inv, (a2*c1 - a1*c2)*det_inv)
