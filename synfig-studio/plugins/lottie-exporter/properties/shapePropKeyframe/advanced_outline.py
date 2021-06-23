# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of Advanced outline layer
in Lottie format
"""

import sys
import math
import copy
import settings
from common.Bline import Bline
from common.WidthPoint import WidthPoint
from common.WidthPointList import WidthPointList
from common.DashItemList import DashItemList
from common.Vector import Vector
from common.Hermite import Hermite
from synfig.animation import to_Synfig_axis
from properties.shapePropKeyframe.helper import add_reverse, add, move_to, get_tangent_at_frame, insert_dict_at, animate_tangents
from properties.shapePropKeyframe.outline import get_outline_grow, get_outline_param_at_frame
sys.path.append("../../")


def gen_bline_advanced_outline(lottie, bline_point):
    """
    Generates the bline corresponding to advanced outline layer by adding some vertices
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

    # For width point list
    width_point_list = layer.get_param("wplist")
    width_point_list = WidthPointList(width_point_list[0], width_point_list)

    for entry in width_point_list.get_entry_list():
        pos = entry["position"]
        width = entry["width"]
        # All other remaining parameters are static

        pos.update_frame_window(window)
        pos.animate("real")

        width.update_frame_window(window)
        width.animate("real")

    # For Dash Item list
    dash_item_list = layer.get_param("dilist")
    dash_item_list = DashItemList(dash_item_list[0], dash_item_list)

    for entry in dash_item_list.get_entry_list():
        offset = entry["offset"]
        length = entry["length"]
        # All other remaining parameters are static

        offset.update_frame_window(window)
        offset.animate("real")

        length.update_frame_window(window)
        length.animate("real")

    outer_width = layer.get_param("width")
    expand = layer.get_param("expand")
    
    start_tip = layer.get_param("start_tip").get()  # Static parameter
    start_tip = int(start_tip[0].attrib["value"])
    end_tip = layer.get_param("end_tip").get()      # Static parameter: can't be animated
    end_tip = int(end_tip[0].attrib["value"])
    cusp_type = layer.get_param("cusp_type").get()  # Static parameter
    cusp_type = int(cusp_type[0].attrib["value"])
    homogeneous = layer.get_param("homogeneous")    # Static parameter
    homogeneous = True if homogeneous[0].attrib["value"] == "true" else False

    smoothness = layer.get_param("smoothness")
    dash_enabled = layer.get_param("dash_enabled")
    dash_offset = layer.get_param("dash_offset")
    origin = layer.get_param("origin")

    # Animating the origin
    origin.update_frame_window(window)
    origin.animate("vector")

    # Animating the outer width
    outer_width.update_frame_window(window)
    outer_width.animate("real")

    # Animating smoothness
    smoothness.update_frame_window(window)
    smoothness.animate("real")

    # Animating the expand param
    expand.update_frame_window(window)
    expand.animate("real")

    # Animating the dash enabled
    dash_enabled.update_frame_window(window)
    dash_enabled.animate_without_path("bool")

    # Animating the dash offset
    dash_offset.update_frame_window(window)
    dash_offset.animate("real")

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window


    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)  # This loop needs to be considered somewhere down

        synfig_advanced_outline(bline, st_val, origin, outer_width, expand,
                start_tip, end_tip, cusp_type, smoothness, homogeneous,
                dash_enabled, dash_offset, dash_item_list, width_point_list, fr)
        synfig_advanced_outline(bline, en_val, origin, outer_width, expand,
                start_tip, end_tip, cusp_type, smoothness, homogeneous,
                dash_enabled, dash_offset, dash_item_list, width_point_list, fr + 1)

        fr += 1
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr


def synfig_advanced_outline(bline, st_val, origin, outer_width_p, expand_p,
        start_tip, end_tip, cusp_type, smoothness_p, homogeneous,
        dash_enabled_p, dash_offset_p, dash_item_list, width_point_list, fr):
    """
    Calculates the points for the advanced outline layer as in Synfig:
    https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/modules/mod_geometry/advanced_outline.cpp

    Returns:
        (None)
    """
    CUSP_TANGENT_ADJUST = 0.025
    SAMPLES = 50
    EPSILON = 0.000000001
    fast_ = False   # This parameter has been removed from Synfig 1.4 onwards
    smoothness = to_Synfig_axis(smoothness_p.get_value(fr), "real")
    dash_offset = to_Synfig_axis(dash_offset_p.get_value(fr), "real")
    dash_enabled = dash_enabled_p.get_value(fr)

    bline_pos, hbline_pos = [], []

    wplist = width_point_list.get_list_at_frame(fr)
    dilist = dash_item_list.get_list_at_frame(fr)
    dwplist = []

    dash_enabled = dash_enabled_p.get_value(fr) and (dash_item_list.get_len() != 0)
    dstart_tip = 4  # WidthPointList::TYPE_FLAT
    dend_tip   = 4  # WidthPointList::TYPE_FLAT

    # Check if bline is looped
    blineloop = bline.get_loop()

    # Check if width point list is looped
    wplistloop = width_point_list.get_loop()

    bline_size = bline.get_len()
    wplist_size = width_point_list.get_len()

    # Initializing the iterator
    bnext = 0

    first = True
    middle_corner = False
    done_tip = False
    inserted_first = False
    inserted_last = False

    bezier_size = 1.0 / (bline_size if blineloop else (1.0 if bline_size==1 else (bline_size-1)))

    # Iterator
    bend = bline.get_len()

    # Outline grow
    gv = get_outline_grow(fr)

    if not blineloop and bline_size == 1:
        return

    bindex = 0.0
    biter = 0
    while biter != bend:
        bline_pos.append(bindex * bezier_size)
        hbline_pos.append(bline_pos[-1] if fast_ else std_to_hom(bline, bline_pos[-1], wplistloop, blineloop, fr))
        bindex += 1
        biter += 1

    if blineloop:
        bline_pos.append(1.0)
        hbline_pos.append(1.0)
    else:
        bline_pos.pop()
        bline_pos.append(1.0)
        hbline_pos.pop()
        hbline_pos.append(1.0)

    hbpiter = 0
    bpiter = 0
    biter_pos = bline_pos[bpiter]
    bpiter += 1
    hbpiter += 1
    bnext_pos = bline_pos[bpiter]
    hbpnext_pos = hbline_pos[hbpiter]

    # Setup chunk list
    side_a, side_b = [], []
    
    for witer in wplist:
        witer.set_position(witer.get_norm_position(wplistloop))

    wplist = sorted(wplist)

    biter = 0
    if blineloop:
        biter = bline.get_len() - 1
    else:
        biter = bnext
        bnext = bnext + 1

    last_tangent = get_outline_param_at_frame(bline[biter], fr)[2]

    if blineloop and cusp_type == 0 and last_tangent.is_equal_to(Vector(0, 0)):
        curve = Hermite(get_outline_param_at_frame(bline[biter-1], fr)[0],
                        get_outline_param_at_frame(bline[biter], fr)[0],
                        get_outline_param_at_frame(bline[biter-1], fr)[3],
                        get_outline_param_at_frame(bline[biter], fr)[2])
        last_tangent = curve.derivative(1.0 - CUSP_TANGENT_ADJUST)

    if not blineloop:
        if wplist_size != 0:
            wpfront = wplist[0]
            wpback = wplist[-1]
            if wpfront.get_side_type_before() == 0 and wpfront.get_position() != 0.0:
                wplist.append(WidthPoint(0.0, wpfront.get_width(), start_tip, 0))
            if wpback.get_side_type_after() == 0 and wpback.get_position() != 1.0:
                wplist.append(WidthPoint(1.0, wpback.get_width(), 0, end_tip))
        else:
            wplist.append(WidthPoint(0.0, 1.0, start_tip, 0))
            wplist.append(WidthPoint(1.0, 1.0, 0, end_tip))
    else:
        if wplist_size != 0:
            wpfront = wplist[0]
            wpback = wplist[-1]
            wpfb_int = wpfront.get_side_type_before() == 0
            wpba_int = wpback.get_side_type_after() == 0

            if wpfb_int or wpba_int:
                if wpfront.get_position() != 0.0:
                    i = copy.deepcopy(wpback)
                    n = copy.deepcopy(wpfront)
                    if not homogeneous and not fast_:
                        i.set_position(std_to_hom(bline, i.get_position(), wplistloop, blineloop, fr)) 
                        n.set_position(std_to_hom(bline, n.get_position(), wplistloop, blineloop, fr)) 
                    wplist.append(WidthPoint(0.0, widthpoint_interpolate(i, n, 0.0, smoothness), 0, 0))
                    inserted_first = True
                if wpfront.get_position() != 1.0:
                    i = copy.deepcopy(wpback)
                    n = copy.deepcopy(wpfront)
                    if not homogeneous and not fast_:
                        i.set_position(std_to_hom(bline, i.get_position(), wplistloop, blineloop, fr)) 
                        n.set_position(std_to_hom(bline, n.get_position(), wplistloop, blineloop, fr)) 
                    wplist.append(WidthPoint(1.0, widthpoint_interpolate(i, n, 1.0, smoothness), 0, 0))
                    inserted_last = True
        else:
            wplist.append(WidthPoint(0.0, 1.0, 0, 0))
            wplist.append(WidthPoint(1.0, 0.0, 0, 0))

    wplist = sorted(wplist)

    step = 1.0/SAMPLES/bline_size

    if dash_enabled:
        blinelength = bline_length(bline, blineloop, None, fr)
        if blinelength > EPSILON:
            dashes_length = 0.0
            diter = 0
            rditer = len(dilist) - 1 
            before, after = None, None  # These are WidthPoints
            while diter != len(dilist):
                dashes_length += dilist[diter].get_length() + dilist[diter].get_offset()
                diter += 1
            if dashes_length > EPSILON:
                if math.fabs(dash_offset) > dashes_length:
                    dash_offset = math.fmod(dash_offset, dashes_length)
                dpos = dash_offset if dash_offset >= 0 else (dashes_length + dash_offset)
                diter = 0
                inserted_to_blinelength = 0
                while dpos < blinelength:
                    before_pos = (dpos + dilist[diter].get_offset())/blinelength
                    after_pos = (dpos + dilist[diter].get_offset() + dilist[diter].get_length())/blinelength 
                    before_pos = before_pos if homogeneous else hom_to_std(bline, before_pos, wplistloop, blineloop)
                    after_pos = after_pos if homogeneous else hom_to_std(bline, after_pos, wplistloop, blineloop)
                    before = WidthPoint(before_pos, 1.0, dilist[diter].get_side_type_before(), 0, True)
                    after = WidthPoint(after_pos, 1.0, 0, dilist[diter].get_side_type_after(), True)
                    dwplist.append(before)
                    dwplist.append(after)
                    dpos += dilist[diter].get_offset() + dilist[diter].get_length()
                    diter += 1
                    inserted_to_blinelength += 1
                    if diter == len(dilist):
                        diter = 0
                if inserted_to_blinelength != 0:
                    after = dwplist[-1]
                    if after.get_position() >= 1.0:
                        after.set_position(1.0)
                        dwplist.pop()
                        before = dwplist[-1]

                        if before.get_position() >= 1.0:
                            dwplist.pop()
                            inserted_to_blinelength -= 1
                        else:
                            dend_tip = after.get_side_type_after()
                            dwplist.append(after) 
                inserted_to_zero = 0

                dpos = dash_offset if (dash_offset >= 0) else (dashes_length + dash_offset)
                while dpos > 0.0:
                    before_pos = (dpos - dilist[rditer].get_length())/blinelength
                    after_pos = (dpos) / blinelength
                    before_pos = before_pos if homogeneous else hom_to_std(bline, before_pos, wplistloop, blineloop)
                    after_pos = after_pos if homogeneous else hom_to_std(bline, after_pos, wplistloop, blineloop)
                    before = WidthPoint(before_pos, 1.0, dilist[rditer].get_side_type_before(), 0.0, True)
                    after = WidthPoint(after_pos, 1.0, 0, dilist[rditer].get_side_type_after(), True)
                    dwplist.insert(0, after)
                    dwplist.insert(0, before)
                    dpos -= dilist[rditer].get_offset() + dilist[rditer].get_length()
                    rditer -= 1
                    inserted_to_zero += 1
                    if rditer == -1:
                        rditer = len(dilist) - 1

                if inserted_to_zero != 0:
                    before = dwplist[0]
                    if before.get_position() <= 0.0:
                        before.set_position(0.0)
                        dwplist.pop(0)
                        after = dwplist[0]

                        if after.get_position() <= 0.0:
                            dwplist.pop(0)
                            inserted_to_zero -= 1
                        else:
                            dstart_tip = before.get_side_type_before()
                            dwplist.insert(0, before)

                if inserted_to_blinelength == 0 and inserted_to_zero == 0:
                    before = WidthPoint(0.5, 1.0, 4, 0, True)
                    after = WidthPoint(0.5, 1.0, 0, 4, True)
                    dwplist.append(before)
                    dwplist.append(after)
                     


def hom_to_std(bline, pos, index_loop, bline_loop, fr):
    size = bline.get_len()
    from_vertex = 0

    if pos == 0.0 or pos == 1.0:
        return pos
    if not bline_loop:
        size -= 1
    if size < 1:
        return 0.0
    int_pos = int(pos)
    one = 0.0

    if index_loop:
        pos = pos - int_pos
        if pos < 0:
            pos += 1
            one = 1.0
    else:
        if pos < 0: pos = 0
        if pos > 1: pos = 1

    tl = 0
    pl = 0
    bl = 0
    lengths = []
    tl = bline_length(bline, bline_loop, lengths, fr)
    mpl = pos*tl
    nxt = 0
    itr = 0
    if bline_loop:  itr = bline.get_len() - 1
    else:
        itr = nxt
        nxt = nxt + 1
    liter = 0

    while mpl > pl and nxt != bline.get_len():
        pl += lengths[liter]
        bl = lengths[liter]
        itr = nxt
        nxt += 1
        liter += 1
        from_vertex += 1

    if pl > mpl:
        liter -= 1
        nxt -= 1
        if nxt == 0:
            itr = bline.get_len() - 1
        else:
            itr -= 1
        pl -= lengths[liter]
        from_vertex -= 1

    blinepoint0 = bline[itr]
    blinepoint1 = bline[nxt]
    curve = Hermite(get_outline_param_at_frame(blinepoint0, fr)[0],
                    get_outline_param_at_frame(blinepoint1, fr)[0],
                    get_outline_param_at_frame(blinepoint0, fr)[3],
                    get_outline_param_at_frame(blinepoint1, fr)[2])

    sn = 0.0
    sn1 = 0.0
    sn2 = 1.0
    t0 = ((mpl - pl)/bl)
    iterations = 0
    max_iterations = 100
    max_error = 0.00001
    error = 0
    fsn1 = t0 - curve.find_distance(0.0, sn1)/bl
    fsn2 = t0 - curve.find_distance(0.0, sn2)/bl
    fsn = 0

    # DO WHILE LOOP IN PYTHON
    while True:
        sn = sn1 - fsn1*((sn1-sn2)/(fsn1-fsn2))
        fsn = t0 - curve.find_distance(0.0, sn)/bl
        sn2 = sn1
        sn1 = sn
        fsn2 = fsn1
        fsn1 = fsn
        error = math.fabs(fsn2 - fsn1)
        iterations += 1
        if not (error > max_error and max_iterations > iterations):
            break

    return int_pos + (from_vertex + sn)/size - one

def std_to_hom(bline, pos, index_loop, bline_loop, fr):
    size = bline.get_len()
    if pos == 0.0 or pos == 1.0:
        return pos
    if not bline_loop:
        size -= 1
    if size < 1:
        return 0.0
    int_pos = int(pos)
    one = 0.0

    if index_loop:
        pos = pos - int_pos
        if pos < 0:
            pos += 1
            one = 1.0
    else:
        if pos < 0: pos = 0
        if pos > 1: pos = 1

    tl = 0
    pl = 0
    lengths = []
    tl = bline_length(bline, bline_loop, lengths, fr)
    if tl == 0.0:   return pos
    from_vertex = int(pos*size)

    liter = 0
    i = 0
    while i < from_vertex:
        pl += lengths[liter]
        i += 1
        liter += 1

    nxt = 0 # bline.begin()
    itr = 0
    if bline_loop:
        itr = bline.get_len() - 1
    else:
        itr = nxt
        nxt = nxt + 1
    if from_vertex > size - 1:
        from_vertex = size - 1
    bline_point0 = bline[from_vertex-1] if from_vertex else bline[itr]
    bline_point1 = bline[from_vertex]
    curve = Hermite(get_outline_param_at_frame(bline_point0, fr)[0],
                    get_outline_param_at_frame(bline_point1, fr)[0],
                    get_outline_param_at_frame(bline_point0, fr)[3],
                    get_outline_param_at_frame(bline_point1, fr)[2])
    pl += curve.find_distance(0.0, pos*size - from_vertex)

    return int_pos + pl/tl-one


def bline_length(bline, bline_loop, lengths, fr):
    size = bline.get_len()
    if not bline_loop:  size -= 1
    if size < 1:    return 0.0
    tl = 0

    nxt = 0 # nxt = bline.begin()
    itr = 0
    if bline_loop:
        itr = bline.get_len() - 1
    else:
        itr = nxt
        nxt = nxt + 1
    while nxt != bline.get_len():
        bline_point0 = bline[itr]
        bline_point1 = bline[nxt]
        curve = Hermite(get_outline_param_at_frame(bline_point0, fr)[0],
                        get_outline_param_at_frame(bline_point1, fr)[0],
                        get_outline_param_at_frame(bline_point0, fr)[3],
                        get_outline_param_at_frame(bline_point1, fr)[2]) 
        l = curve.length()
        if lengths is not None:
            lengths.append(l)
        tl += l
        itr = nxt
        nxt += 1
    return tl

def widthpoint_interpolate(prev, nxt, p, smoothness):
    side_int = 0
    nw, pw = 0, 0
    rw = 0.0
    epsilon = 0.0000001
    np = nxt.get_position()
    pp = prev.get_position()
    nw = nxt.get_width()
    pw = prev.get_width()
    nsb = nxt.get_side_type_before()
    nsa = nxt.get_side_type_after()
    psb = prev.get_side_type_before()
    psa = prev.get_side_type_after()

    if p == np:
        return nw
    if p == pp:
        return pw
    if np > pp:
        if np > p and p > pp:
            q = 0
            if nsb != side_int:
                nw = 0.0
            if psa != side_int:
                pw = 0.0
            if (np - pp < epsilon):
                q = 0.5
            else:
                q = (p - pp) / (np - pp)
            rw = pw + (nw-pw)*(q*(1.0-smoothness)+q*q*q*(10+q*(6*q-15))*smoothness)
        elif p < pp:
            if psb != side_int:
                pw = 0.0
            rw = pw
        elif p > np:
            if nsa != side_int:
                nw = 0.0
            rw = nw
    elif p > pp or np > p:
        q = 0.0
        if nsb != side_int:
            nw = 0.0
        if psa != side_int:
            pw = 0.0
        if np+1.0-pp < epsilon:
            q = 0.5
        else:
            if p > pp:
                q = (p - pp)/(np+1.0-pp)
            if np > p:
                q = (p+1.0-pp)/(np+1.0-pp)
        rw = pw+(nw-pw)*(q*(1.0-smoothness)+q*q*q*(10+q*(6*q-15))*smoothness)
    elif p > np or p < pp:
        q = 0.0
        if nsa != side_int:
            nw = 0.0
        if psb != side_int:
            pw = 0.0
        if pp - np < epsilon:
            q = 0.5
        else:
            q = (p - np) / (pp - np)
        rw = nw+(pw-nw)*(q*(1.0-smoothness)+q*q*q*(10+q*(6*q-15))*smoothness) 
    return rw
