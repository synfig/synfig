#!/usr/bin/python3
#
# This is a script that will run the synfig rendering multiple times on a set of
# .sif files.  Record the time it took each pass to render, and then store those
# readings into a .csv file.  To properly run this:
#
# 1. This script needs to be placed in the root of the build directory, and run
#    from that directory
# 2. The `synfig-tests` repo needs to be cloned (in the build directory as well.
#    The repo is found here: https://gitlab.com/synfig/synfig-tests
# 3. Don't have any applications running at the same time.  It can mess with the
#    performance measurements.
#
# It can take an extremely long time to run (.e.g 25 mintues right now).  Before
# trying to do any perf tweaking in the rendering code, make sure you generate
# a reference/baseline to go off from.  This should be the latest commit on
# `master`.  Don't forget to copy this CSV file to another location in case you
# accidentally purge it.  Every so often (as new commits come in), you'll need
# to run the script to get a new baseline.  But as the renderer gets faster,
# it shouldn't have to run as long.
#
# Be sure to see the `view_comparison_graph.py` script to see how to make use
# of the generated CSV file



import os
import time, datetime
import csv
import subprocess
from collections import OrderedDict

SIF_DIR = 'synfig-tests/export/lottie/'
SIF_EXE = 'output/bin/synfig'
NUM_PASSES = 10                 # if you want to work on this script, setting this value to `3` helps
COMMIT_ID = 'perf_imps-'        # TODO see list below


# TODO list:
# - get the current working branch and commit id and put it into the `COMMIT_ID` variable
# - results files should not only be a CSV.  It should also contain the metadata
#   about the environment & setup).  So instead there should be a JSON file generated
#   - Add a way of getting the current environment (e.g. Ubuntu 18.04) and put it the
#     metadata
#   - Add a way of polling the current CPU and put it into the metadata
#   - Add a way of polling the compiler used by CMake (and build type) and put it into
#     the metadata



def main():
    # Get all of the .sif files
    all_sif = os.listdir(SIF_DIR)
    all_sif = list(filter(lambda x: x.endswith('.sif'), all_sif))
    all_sif.sort()

#    all_sif = all_sif[:10]      # Uncomment this line when you want to do some development on the script.  It's help to limit the amount of .sif files

    # Result of all the renders, key=<render filename>, value=list[float]
    all_renders = OrderedDict()

    print('Doing (%i x %i) render tests' % (len(all_sif), NUM_PASSES))

    total_render_time = 0.0
    for sif in all_sif:
        # Create the path to the file
        sif_path = os.path.join(SIF_DIR, sif)

        # Create result store
        render_times = []

        for i in range(0, NUM_PASSES):
            # Run (and time) the render
            st = time.time()
            subprocess.run(
                [SIF_EXE, sif_path, '-t', 'null', '--quiet'],
                cwd=os.getcwd()
            )
            et = time.time()

            # Store result
            rt = et - st
            render_times.append(rt)
            total_render_time += rt

            # Print it
            print('%s [%02i]  ::  %.4f' % (sif, i + 1, rt))

        # Store in dict
        all_renders[sif] = render_times

    # Write the results
    time_str = datetime.datetime.now().strftime('%Y_%m_%d-%H_%M_%S')
    result_filename = '%s_n%s_%s.csv' % (COMMIT_ID, NUM_PASSES, time_str)
    with open(result_filename, 'w') as csv_file:
        # Make the header, will be: ['.sif file', 'Pass 1', 'Pass 2', ... 'Pass 10']
        fieldnames = ['.sif file'] + ['Pass %i time' % (x + 1) for x in range(0, NUM_PASSES)]

        wr = csv.writer(csv_file)
        wr.writerow(fieldnames)

        # Write each result
        for sif, render_times in all_renders.items():
            wr.writerow([sif] + render_times)

    print('Wrote results to %s' % result_filename)
    print('Total Render Time: %.4f sec (%.2f min)' % (total_render_time, total_render_time / 60.0))


if __name__ == '__main__':
    main()
