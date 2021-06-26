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
from common.Angle import RadAngle, SinAngle, CosAngle
from synfig.animation import to_Synfig_axis
from properties.shapePropKeyframe.helper import add_reverse, add, move_to, get_tangent_at_frame, insert_dict_at_adv_outline, animate_tangents
from properties.shapePropKeyframe.outline import line_intersection, get_outline_grow, get_outline_param_at_frame
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

    # Store all side_a, side_b values; because we need to make them equal in
    # size in order to render properly in lottie
    st_list = []
    en_list = []
    lottie_st = []
    lottie_en = []

    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at_adv_outline(lottie, -1, fr, False)  # This loop needs to be considered somewhere down
        lottie_st.append([st_val, fr])
        lottie_en.append([en_val, fr+1])

        st_list_value = synfig_advanced_outline(bline, st_val, origin, outer_width, expand,
                                start_tip, end_tip, cusp_type, smoothness, homogeneous,
                                dash_enabled, dash_offset, dash_item_list, width_point_list, fr)
        en_list_value = synfig_advanced_outline(bline, en_val, origin, outer_width, expand,
                                start_tip, end_tip, cusp_type, smoothness, homogeneous,
                                dash_enabled, dash_offset, dash_item_list, width_point_list, fr + 1)
        st_list.append(st_list_value)
        en_list.append(en_list_value)

        fr += 1

    append_all_lists(st_list, en_list, lottie_st, lottie_en, origin)

    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr
    lottie[-1]["h"] = 1


def append_all_lists(st_list, en_list, lottie_st, lottie_en, origin_p):
    """
    
    """
    # Find maximum size among all the lists
    mx = 0
    for st in st_list:
        mx = max(mx, len(st))
    for en in en_list:
        mx = max(mx, len(en))

    # All the lists should of size mx
    for st in st_list:
        diff = mx - len(st)
        last_value = copy.deepcopy(st[-1])
        st.extend([last_value for i in range(diff)])

    for en in en_list:
        diff = mx - len(en)
        last_value = copy.deepcopy(en[-1])
        en.extend([last_value for i in range(diff)])

    # Now render 
    for i in range(0, len(lottie_st)):
        cur_frame = lottie_st[i][1]
        origin_cur = origin_p.get_value(cur_frame)
        add(st_list[i], lottie_st[i][0], origin_cur) 
        lottie_st[i][0]["h"] = 1

    for i in range(0, len(lottie_en)):
        cur_frame = lottie_en[i][1]
        origin_cur = origin_p.get_value(cur_frame)
        add(en_list[i], lottie_en[i][0], origin_cur)
        lottie_en[i][0]["h"] = 1


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
    expand = to_Synfig_axis(expand_p.get_value(fr), "real")
    width = to_Synfig_axis(outer_width_p.get_value(fr), "real")

    bline_pos, hbline_pos = [], []

    wplist = width_point_list.get_list_at_frame(fr)
    dilist = dash_item_list.get_list_at_frame(fr)
    dwplist = []
    fdwplist = []

    dash_enabled = dash_enabled and (dash_item_list.get_len() != 0)
    dstart_tip = 4  # WidthPointList::TYPE_FLAT
    dend_tip   = 4  # WidthPointList::TYPE_FLAT

    # Check if bline is looped
    blineloop = bline.get_loop()

    # Check if width point list is looped
    wplistloop = width_point_list.get_loop()

    bline_size = bline.get_len()
    wplist_size = width_point_list.get_len()

    # Initializing the iterator
    biter, bnext = 0, 0
    bpiter, bpnext, hbpiter = 0, 0, 0
    witer, wnext, switer, swnext, cwiter, cwnext, scwiter, scwnext, dwiter, dwnext = 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

    first_tangent = None
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
    hbnext_pos = hbline_pos[hbpiter]

    # Setup chunk list
    side_a, side_b = [], []
    
    for witer in wplist:
        witer.set_position(witer.get_norm_position(wplistloop))

    wplist = sorted(wplist)

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
            wpfb_int = (wpfront.get_side_type_before() == 0)
            wpba_int = (wpback.get_side_type_after() == 0)

            if wpfb_int or wpba_int:
                if wpfront.get_position() != 0.0:
                    i = copy.deepcopy(wpback)
                    n = copy.deepcopy(wpfront)
                    if not homogeneous and not fast_:
                        i.set_position(std_to_hom(bline, i.get_position(), wplistloop, blineloop, fr)) 
                        n.set_position(std_to_hom(bline, n.get_position(), wplistloop, blineloop, fr)) 
                    wplist.append(WidthPoint(0.0, widthpoint_interpolate(i, n, 0.0, smoothness), 0, 0))
                    inserted_first = True
                if wpback.get_position() != 1.0:
                    i = copy.deepcopy(wpback)
                    n = copy.deepcopy(wpfront)
                    if not homogeneous and not fast_:
                        i.set_position(std_to_hom(bline, i.get_position(), wplistloop, blineloop, fr)) 
                        n.set_position(std_to_hom(bline, n.get_position(), wplistloop, blineloop, fr)) 
                    wplist.append(WidthPoint(1.0, widthpoint_interpolate(i, n, 1.0, smoothness), 0, 0))
                    inserted_last = True
        else:
            wplist.append(WidthPoint(0.0, 1.0, 0, 0))
            wplist.append(WidthPoint(1.0, 1.0, 0, 0))

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
                    before_pos = before_pos if homogeneous else hom_to_std(bline, before_pos, wplistloop, blineloop, fr)
                    after_pos = after_pos if homogeneous else hom_to_std(bline, after_pos, wplistloop, blineloop, fr)
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
                    before_pos = before_pos if homogeneous else hom_to_std(bline, before_pos, wplistloop, blineloop, fr)
                    after_pos = after_pos if homogeneous else hom_to_std(bline, after_pos, wplistloop, blineloop, fr)
                    before = WidthPoint(before_pos, 1.0, dilist[rditer].get_side_type_before(), 0, True)
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
                     
                wnext = 0
                if blineloop:
                    witer = len(wplist) - 1
                else:
                    witer = wnext

                while True: # Do while loop
                    witer_pos = wplist[witer].get_position()
                    wnext_pos = wplist[wnext].get_position()

                    if wplist[witer].get_side_type_after() == 0 or wplist[wnext].get_side_type_before() == 0:
                        dwiter = 0
                        while dwiter != len(dwplist):
                            dwiter_pos = dwplist[dwiter].get_position()
                            if dwiter_pos > witer_pos and dwiter_pos < wnext_pos:
                                fdwplist.append(copy.deepcopy(dwplist[dwiter]))
                            dwiter += 1
                    witer = wnext
                    wnext += 1

                    if not (wnext != len(wplist)):
                        break

                dwiter = 0
                dwnext = dwiter + 1
                
                while True: # Do while loop
                    dwiter_pos = dwplist[dwiter].get_position()
                    dwnext_pos = dwplist[dwnext].get_position()
                    witer = 0
                    while witer != len(wplist):
                        witer_pos = wplist[witer].get_position()
                        if witer_pos <= dwnext_pos and witer_pos >= dwiter_pos:
                            fdwplist.append(copy.deepcopy(wplist[witer]))
                        witer += 1
                    dwnext += 1
                    dwiter = dwnext
                    if dwnext == len(dwplist):
                        break
                    dwnext += 1

    """
    print("START")
    for my_it in fdwplist:
        print(str(my_it.get_position()) + " " + str(my_it.get_width()) + " " +
                str(my_it.get_side_type_before()) + " " +
                str(my_it.get_side_type_after()))
    print("END")
    """

    cwplist = []
    for my_iterator in wplist:
        cwplist.append(copy.deepcopy(my_iterator))
    scwplist = []
    for my_iterator in wplist:
        scwplist.append(copy.deepcopy(my_iterator))

    if homogeneous:
        scwiter = 0
        while scwiter != len(scwplist):
            scwplist[scwiter].set_position(hom_to_std(bline, scwplist[scwiter].get_position(), wplistloop, blineloop, fr)) 
            scwiter += 1
    else:
        cwiter = 0
        while cwiter != len(cwplist):
            cwplist[cwiter].set_position(std_to_hom(bline, cwplist[cwiter].get_position(), wplistloop, blineloop, fr))
            cwiter += 1

    if dash_enabled:
        wplist = []
        for my_iterator in fdwplist:
            wplist.append(copy.deepcopy(my_iterator))
        wplist = sorted(wplist)
        witer = 0


    if len(wplist) == 0:
        wplist.append(WidthPoint(0.5, 1.0, 4, 4, True))

    swplist = []
    for my_iterator in wplist:
        swplist.append(copy.deepcopy(my_iterator))

    if homogeneous:
        switer = 0
        while switer != len(swplist):
            swplist[switer].set_position(hom_to_std(bline, swplist[switer].get_position(), wplistloop, blineloop, fr))
            switer += 1
    else:
        witer = 0
        while witer != len(wplist):
            wplist[witer].set_position(std_to_hom(bline, wplist[witer].get_position(), wplistloop, blineloop, fr))
            witer += 1
    
    wnext = 0
    swnext = 0
    if blineloop:
        witer = len(wplist) - 1
        switer = len(swplist) - 1
    else:
        witer = wnext
        switer = swnext
    cwnext = 0
    scwnext = 0
    if blineloop:
        cwiter = len(cwplist) - 1
        scwiter = len(scwplist) - 1
    else:
        cwiter = cwnext
        scwiter = scwnext
    wend = len(wplist)
    swend = len(wplist)

    ipos = 0.0
    hipos = 0.0

    if not blineloop:
        if wplist[wnext].get_position() == 0.0:
            wplist[wnext].set_side_type_before(dstart_tip if dash_enabled else start_tip)
        last = len(wplist) - 1
        if wplist[last].get_position() == 1.0:
            wplist[last].set_side_type_after(dend_tip if dash_enabled else end_tip)

    if dash_enabled:
        first = 0
        last = len(wplist) - 1
        if wplist[first].get_side_type_before() == 0:
            wplist[first].set_side_type_before(dstart_tip)
        if wplist[last].get_side_type_after() == 0:
            wplist[last].set_side_type_after(dend_tip)

    """
    print("START")
    print(len(wplist))
    print(len(swplist))
    print(len(cwplist))
    print(len(scwplist))
    print(len(dwplist))
    print(len(fdwplist))
    print("END")
    """

    # Main loop
    while True:
        iter_t = get_outline_param_at_frame(bline[biter], fr)[3] 
        next_t = get_outline_param_at_frame(bline[bnext], fr)[2]
        iter_t_mag = iter_t.mag()
        next_t_mag = next_t.mag()

        split_flag = get_outline_param_at_frame(bline[biter], fr)[5] or (get_outline_param_at_frame(bline[biter], fr)[2].mag() == 0) or (get_outline_param_at_frame(bline[biter], fr)[3].mag() == 0)
        curve = Hermite(get_outline_param_at_frame(bline[biter], fr)[0],
                        get_outline_param_at_frame(bline[bnext], fr)[0],
                        iter_t,
                        next_t)
        if iter_t_mag == 0.0:
            iter_t = curve.derivative(CUSP_TANGENT_ADJUST)
        if next_t_mag == 0.0:
            next_t = curve.derivative(1.0 - CUSP_TANGENT_ADJUST) 
        if blineloop and first:
            first_tangent = iter_t
            first = False

        wnext_pos = wplist[wnext].get_position()
        swnext_pos = swplist[swnext].get_position()

        if ipos == swnext_pos:
            unitary = None
            hipos = wnext_pos
            bezier_ipos = bline_to_bezier(ipos, biter_pos, bezier_size)
            q = bezier_ipos
            if q < EPSILON:
                unitary = iter_t.norm()
            elif q > (1.0 - EPSILON):
                unitary = next_t.norm()
            else:
                unitary = curve.derivative(q).norm()
            if wplist[wnext].get_dash():
                ci = scwiter
                cn = scwnext
                if not fast_:    # fast is set to False always
                    ci = cwiter
                    cn = cwnext
                # ASSUMPTION: if fast is set to True, then the code will change below
                if cwplist[ci].get_position() == 0.0 and cwplist[cn].get_position() != 1.0 and inserted_first:
                    ci = len(cwplist) - 1
                    if inserted_last:
                        ci -= 1
                if cwplist[cn].get_position() == 1.0 and inserted_last:
                    cn = 0
                    if inserted_first:
                        cn += 1
                i = cwplist[ci]
                n = cwplist[cn]
                p = ipos
                if not fast_:
                    p = hipos
                wplist[wnext].set_width(widthpoint_interpolate(i, n, p, smoothness))
            add_tip(side_a, side_b, curve.value(q), unitary, wplist[wnext], gv, width, expand)

            witer = wnext
            switer = swnext
            wnext += 1
            swnext += 1

            if wnext == wend or swnext == swend:
                cwnext = 0
                cwiter = len(cwplist) - 1
                scwnext = 0
                scwiter = len(scwplist) - 1

                if blineloop and (get_outline_param_at_frame(bline[bnext], fr)[5] or get_outline_param_at_frame(bline[bnext], fr)[2].mag() == 0 or get_outline_param_at_frame(bline[bnext], fr)[3].mag() == 0):
                    first = 0
                    last = len(wplist) - 1

                    if wplist[first].get_side_type_before() == 0 or wplist[last].get_side_type_after() == 0:
                        i = scwplist[scwiter]
                        n = scwplist[scwnext]
                        if not fast_:
                            i = cwplist[cwiter]
                            n = cwplist[cwnext]
                        p = ipos
                        if not fast_:
                            p = hipos
                        add_cusp(side_a,
                                 side_b,
                                 get_outline_param_at_frame(bline[bnext], fr)[0],
                                 first_tangent,
                                 curve.derivative(1.0-CUSP_TANGENT_ADJUST),
                                 gv*(expand+width*0.5*widthpoint_interpolate(i, n, p, smoothness)),
                                 cusp_type)
                break
            else:
                ipos = ipos + EPSILON
                if wplist[witer].get_side_type_after() != 0:
                    done_tip = True
                else:
                    done_tip = False
                if ipos > scwplist[scwnext].get_position():
                    cwiter = cwnext
                    scwiter = scwnext
                    cwnext += 1
                    scwnext += 1
                middle_corner = False
                continue
        if (wplist[witer].get_side_type_after() != 0 and wplist[wnext].get_side_type_before() != 0) or (witer == 0 and wnext == 0):    
            ipos = swnext_pos
            if ipos > scwplist[scwnext].get_position():
                cwiter = cwnext
                scwiter = scwnext
                cwnext += 1
                scwnext += 1
            while ipos > bnext_pos and bnext+1 != bend:
                last_tangent = curve.derivative(1.0-CUSP_TANGENT_ADJUST)
                biter = bnext
                bnext += 1
                biter_pos = bnext_pos
                bpiter += 1
                hbpiter += 1
                bnext_pos = bline_pos[bpiter]
                hbnext_pos = hbline_pos[hbpiter]
            middle_corner = False
            continue

        if middle_corner == True:
            if split_flag:
                i = scwplist[scwiter]
                n = scwplist[scwnext]
                if not fast_:
                    i = cwplist[cwiter]
                    n = cwplist[cwnext]
                p = ipos
                if not fast_:
                    p = hipos
                add_cusp(side_a,
                        side_b,
                        get_outline_param_at_frame(bline[biter], fr)[0],
                        curve.derivative(CUSP_TANGENT_ADJUST),
                        last_tangent,
                        gv*(expand+width*0.5*widthpoint_interpolate(i, n, p, smoothness)),
                        cusp_type)
            middle_corner = False
            ipos = ipos + EPSILON
        # Secondary loop. For interpolation tasks
        while True:
            swnext_pos = swplist[swnext].get_position()
            if ipos > swnext_pos and bnext_pos >= swnext_pos:
                unitary = None
                ipos = swnext_pos
                hipos = wnext_pos
                q = bline_to_bezier(ipos, biter_pos, bezier_size)
                if q < EPSILON:
                    unitary = iter_t.norm()
                elif q > (1.0 - EPSILON):
                    unitary = next_t.norm()
                else:
                    unitary = curve.derivative(q).norm()
                d = unitary.perp()
                p = curve.value(q)
                ww = 0.0

                if wplist[wnext].get_side_type_before() != 0:
                    ww = 0.0
                else:
                    if wplist[wnext].get_dash():
                        i = scwplist[scwiter]
                        n = scwplist[scwnext]
                        if not fast_:
                            i = cwplist[cwiter]
                            n = cwplist[cwnext]
                        p_my = ipos
                        if not fast_:
                            p_my = hipos
                        wplist[wnext].set_width(widthpoint_interpolate(i, n, p_my, smoothness))
                    ww = wplist[wnext].get_width()
                w = gv*(expand+width*0.5*ww)
                side_a.append([p+d*w, Vector(0, 0), Vector(0, 0)])
                side_b.append([p-d*w, Vector(0, 0), Vector(0, 0)])
                break
            elif ipos > bnext_pos and bnext_pos < swnext_pos:
                hipos = hbnext_pos
                ipos = bnext_pos
                middle_corner = True
                q = bline_to_bezier(ipos, biter_pos, bezier_size)
                q = q if (q > CUSP_TANGENT_ADJUST) else (CUSP_TANGENT_ADJUST)
                q = (1.0 - CUSP_TANGENT_ADJUST) if (q > 1.0 - CUSP_TANGENT_ADJUST) else q
                d = curve.derivative(q).perp().norm()
                p = curve.value(bline_to_bezier(ipos, biter_pos, bezier_size))
                i = scwplist[scwiter]
                n = scwplist[scwnext]
                if not fast_:
                    i = cwplist[cwiter]
                    n = cwplist[cwnext]
                po = ipos
                if not fast_:
                    po = hipos
                w = gv*(expand+width*0.5*widthpoint_interpolate(i, n, po, smoothness))
                side_a.append([p+d*w, Vector(0, 0), Vector(0, 0)])
                side_b.append([p-d*w, Vector(0, 0), Vector(0, 0)])
                biter = bnext
                bnext += 1
                biter_pos = bnext_pos
                bpiter += 1
                hbpiter += 1
                bnext_pos = bline_pos[bpiter]
                hbpnext_pos = hbline_pos[hbpiter]
                last_tangent = curve.derivative(1.0-CUSP_TANGENT_ADJUST)
                break

            # Add intepolation
            unitary = None
            q = bline_to_bezier(ipos, biter_pos, bezier_size)
            unitary = curve.derivative(q).norm()
            d = unitary.perp()
            p = curve.value(q)

            if cwplist[cwiter].get_position() == 0.0 and cwplist[cwnext].get_position() != 1.0 and inserted_first:
                cwiter = len(cwplist)
                cwiter -= 1
                if inserted_last:
                    cwiter -= 1
            if cwplist[cwnext].get_position() == 1.0 and inserted_last:
                cwnext = 0
                if inserted_first:
                    cwnext += 1
            i = scwplist[scwiter]
            n = scwplist[scwnext]
            if not fast_:
                i = cwplist[cwiter]
                n = cwplist[cwnext]
            po = ipos
            if not fast_:
                po = std_to_hom(bline, ipos, wplistloop, blineloop, fr)
            w = 0.0
            if done_tip:
                w = 0
                done_tip = False
            else:
                w = (gv*(expand+width*0.5*widthpoint_interpolate(i, n, po, smoothness)))
            side_a.append([p+d*w, Vector(0, 0), Vector(0, 0)])
            side_b.append([p-d*w, Vector(0, 0), Vector(0, 0)])
            ipos = ipos + step

    if blineloop:
        side_b.reverse()
        # Remove all data after we encounter a nan value
        # From add_polygon()
        itr = 0
        while itr < len(side_a):
            if side_a[itr][0].isnan():
                break
            itr += 1
        side_a = side_a[:itr]

        itr = 0
        while itr < len(side_b):
            if side_b[itr][0].isnan():
                break
            itr += 1
        side_b = side_b[:itr]

        side_a.extend(side_b)
        return side_a


    while len(side_b) != 0:
        side_a.append(side_b[-1])
        side_b.pop()

    """
    print("Anish")
    for points in side_a:
        print(str(points[0].val1) + " " + str(points[0].val2))
    print("Gulati")
    """

    return side_a


def add_cusp(side_a, side_b, vertex, curr, last, w, cusp_type):
    CUSP_THRESHOLD = 0.40
    SPIKE_AMOUNT = 4
    SAMPLES = 50
    t1 = last.perp().norm()
    t2 = curr.perp().norm()
    cross = t1*t2.perp()
    perp = (t1-t2).mag()
    if cusp_type == 0:
        if cross > CUSP_THRESHOLD:
            p1 = vertex + t1*w
            p2 = vertex + t2*w
            side_a.append([line_intersection(p1, last, p2, curr), Vector(0, 0), Vector(0, 0)])
        elif cross < -CUSP_THRESHOLD:
            p1 = vertex - t1*w
            p2 = vertex - t2*w
            side_b.append([line_intersection(p1, last, p2, curr), Vector(0, 0), Vector(0, 0)])
        elif cross > 0 and perp > 1:
            amount = max(0.0, (cross/CUSP_THRESHOLD))*(SPIKE_AMOUNT-1)+1
            side_a.append([vertex+(t1+t2).norm()*w*amount, Vector(0, 0), Vector(0, 0)])
        elif cross < 0 and perp > 1:
            amount = max(0.0, (-cross/CUSP_THRESHOLD))*(SPIKE_AMOUNT-1)+1
            side_b.append([vertex-(t1+t2).norm()*w*amount, Vector(0, 0), Vector(0, 0)])
    elif cusp_type == 1:
        if cross > 0:
            p1 = vertex + t1*w
            p2 = vertex + t2*w
            offset = t1.angle()
            angle = t2.angle()-offset
            if angle < RadAngle(0) and offset > RadAngle(0):
                angle += DegAngle(360)
                offset += DegAngle(360)
            tangent = 4 * ((2 * CosAngle(angle/2).get() - CosAngle(angle).get() - 1)/ SinAngle(angle).get())
            curve = Hermite(p1,
                            p2,
                            Vector(-tangent*w*SinAngle(angle*0+offset).get(), tangent*w*CosAngle(angle*0+offset).get()),
                            Vector(-tangent*w*SinAngle(angle*1+offset).get(), tangent*w*CosAngle(angle*1+offset).get()))
            n = 0.0
            while n < 0.999999:
                side_a.append([curve.value(n), Vector(0, 0), Vector(0, 0)]) 
                n += 4.0 / SAMPLES

        if cross < 0:
            p1 = vertex - t1*w
            p2 = vertex - t2*w
            offset = t2.angle()
            angle = t1.angle() - offset
            if angle < RadAngle(0) and offset > RadAngle(0):
                angle += DegAngle(360)
                offset += DegAngle(360)
            tangent = 4 * ((2 * CosAngle(angle/2).get() - CosAngle(angle).get() - 1)/ SinAngle(angle).get())
            curve = Hermite(p1,
                            p2,
                            Vector(-tangent*w*SinAngle(angle*1+offset).get(), tangent*w*CosAngle(angle*1+offset).get()),
                            Vector(-tangent*w*SinAngle(angle*0+offset).get(), tangent*w*CosAngle(angle*0+offset).get()))
            n = 0.0
            while n < 0.999999:
                side_b.append([curve.value(n), Vector(0, 0), Vector(0, 0)]) 
                n += 4.0 / SAMPLES
    return
             

def add_tip(side_a, side_b, vertex, tangent, wp, gv, width, expand):
    ROUND_END_FACTOR = 4
    SAMPLES = 50
    w = gv * (expand + width*0.5*wp.get_width())
    if wp.get_side_type_before() == 1:
        curve = Hermite(vertex-tangent.perp()*w,
                        vertex+tangent.perp()*w,
                        -tangent*w*ROUND_END_FACTOR,
                        tangent*w*ROUND_END_FACTOR)
        side_a.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex, Vector(0, 0), Vector(0, 0)])
        n = 0.0
        while n < 0.499999:
            side_a.append([curve.value(0.5+n), Vector(0, 0), Vector(0, 0)]) 
            side_b.append([curve.value(0.5-n), Vector(0, 0), Vector(0, 0)]) 
            n += 2.0/SAMPLES
        side_a.append([curve.value(1.0), Vector(0, 0), Vector(0, 0)])
        side_b.append([curve.value(0.0), Vector(0, 0), Vector(0, 0)])
    elif wp.get_side_type_before() == 2:
        side_a.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_a.append([vertex-tangent*w, Vector(0, 0), Vector(0, 0)])
        side_a.append([vertex+(tangent.perp()-tangent)*w, Vector(0, 0), Vector(0, 0)])
        side_a.append([vertex+tangent.perp()*w, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex-tangent*w, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex+(-tangent.perp()-tangent)*w, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex-tangent.perp()*w, Vector(0, 0), Vector(0, 0)])
    elif wp.get_side_type_before() == 3:
        side_a.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_a.append([vertex-tangent*w, Vector(0, 0), Vector(0, 0)])
        side_a.append([vertex+tangent.perp()*w, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex-tangent*w, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex-tangent.perp()*w, Vector(0, 0), Vector(0, 0)])
    elif wp.get_side_type_before() == 4:
        side_a.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex, Vector(0, 0), Vector(0, 0)])

    if wp.get_side_type_after() == 1:
        curve = Hermite(vertex-tangent.perp()*w,
                        vertex+tangent.perp()*w,
                        tangent*w*ROUND_END_FACTOR,
                        -tangent*w*ROUND_END_FACTOR)
        n = 0.0
        while n < 0.499999:
            side_a.append([curve.value(1-n), Vector(0, 0), Vector(0, 0)]) 
            side_b.append([curve.value(n), Vector(0, 0), Vector(0, 0)]) 
            n += 2.0/SAMPLES
        side_a.append([curve.value(0.5), Vector(0, 0), Vector(0, 0)]) 
        side_b.append([curve.value(0.5), Vector(0, 0), Vector(0, 0)]) 
        side_a.append([vertex, Vector(0, 0), Vector(0, 0)]) 
        side_b.append([vertex, Vector(0, 0), Vector(0, 0)]) 
    elif wp.get_side_type_after() == 2:
        side_a.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_a.append([vertex+tangent*w, Vector(0, 0), Vector(0, 0)])
        side_a.append([vertex+(-tangent.perp()+tangent)*w, Vector(0, 0), Vector(0, 0)])
        side_a.append([vertex-tangent.perp()*w, Vector(0, 0), Vector(0, 0)])
        side_a.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex+tangent*w, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex+(tangent.perp()+tangent)*w, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex+tangent.perp()*w, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex, Vector(0, 0), Vector(0, 0)])
    elif wp.get_side_type_after() == 3:
        side_a.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_a.append([vertex+tangent*w, Vector(0, 0), Vector(0, 0)])
        side_a.append([vertex-tangent.perp()*w, Vector(0, 0), Vector(0, 0)])
        side_a.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex+tangent*w, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex+tangent.perp()*w, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex, Vector(0, 0), Vector(0, 0)])
    elif wp.get_side_type_after() == 4:
        side_a.append([vertex, Vector(0, 0), Vector(0, 0)])
        side_b.append([vertex, Vector(0, 0), Vector(0, 0)])
    return 


def bline_to_bezier(bline_pos, origin, bezier_size):
    if bezier_size != 0:
        return (bline_pos - origin)/bezier_size
    return bline_pos


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

    print(itr, nxt, bline.get_len())
    blinepoint0 = bline[itr]

    a = get_outline_param_at_frame(blinepoint0, fr)[0]
    c = get_outline_param_at_frame(blinepoint0, fr)[3]

    if nxt == bline.get_len():
        b = Vector(0, 0)
        d = Vector(0, 0)
    else:
        blinepoint1 = bline[nxt]
        b = get_outline_param_at_frame(blinepoint1, fr)[0]
        d = get_outline_param_at_frame(blinepoint1, fr)[2]

    curve = Hermite(a, b, c, d)

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
    bline_point0 = bline[nxt+from_vertex-1] if from_vertex else bline[itr]
    bline_point1 = bline[nxt+from_vertex]
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
    elif p > np and p < pp:
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
