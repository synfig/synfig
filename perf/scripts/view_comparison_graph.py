#!/usr/bin/python3
#
# Be sure to read the `test_render_all_perf.py` script first.  You'll need two
# generated CSV files from that script.  You'll also need matplotlib/pyplot
# installed.
#
# This script takes two arguments, the filepaths of two runs.  The first one
# provided is considered the "reference" run.  The second is the "test" run.
# Interpreting the graph is pretty simple,  Whichever bar is short means that
# it was faster. Make sure you have enough passes from the
# `test_render_all_perf.py` script (default should be set to 10).  The `mean`
# of all runs will be plotted as a bar's value.  But black line will appear
# on the edge of the bar's tip, as to mark the varriance/error.

import sys
import matplotlib.pyplot as plt
import csv
import statistics

plt.style.use('ggplot')


def main():
    if len(sys.argv) < 3:
        print('Please supply at least two CSV files of runs')
        print('  First one should be the `master` reference, second is the comparison')
        sys.exit(0)

    run_data_filenames = sys.argv[1:]

    # First one is the reference
    ref_run_filename = run_data_filenames[0]

    # Chart data
    sif_files = []
    ref_mean_runs = []
    ref_fastest_runs = []
    ref_slowest_runs = []

    # Get the data (for the reference run)
    past_first_row = False
    with open(ref_run_filename) as csvfile:
        r = csv.reader(csvfile)

        for row in r:
            # First row only has meta-data
            if not past_first_row:
                past_first_row = True
                continue

            # Else, we've got data, first column is the filename
            sif_files.append(row[0])

            # The rest are the runs of the data
            all_runs = [float(x) for x in row[1:]]
            mean_run = statistics.mean(all_runs)
            fastest_run = min(all_runs)
            slowest_run = max(all_runs)

            ref_mean_runs.append(mean_run)
            ref_fastest_runs.append(mean_run - fastest_run)
            ref_slowest_runs.append(slowest_run - mean_run)

    # test run
    # TODO, this should be put in a reusable function!!
    test_run_filename = run_data_filenames[1]
    test_mean_runs = []
    test_fastest_runs = []
    test_slowest_runs = []
    past_first_row = False

    with open(test_run_filename) as csvfile:
        r = csv.reader(csvfile)

        for row in r:
            # First row only has meta-data
            if not past_first_row:
                past_first_row = True
                continue

            # The rest are the runs of the data
            all_runs = [float(x) for x in row[1:]]
            mean_run = statistics.mean(all_runs)
            fastest_run = min(all_runs)
            slowest_run = max(all_runs)

            test_mean_runs.append(mean_run)
            test_fastest_runs.append(mean_run - fastest_run)
            test_slowest_runs.append(slowest_run - mean_run)

    # Reverse the order (os that we have 000 at the top, and NNN at the bottom)
    for x in [sif_files, ref_mean_runs, ref_fastest_runs, ref_slowest_runs, test_mean_runs, test_fastest_runs, test_slowest_runs]:
        x.reverse()

    # Create and show the graph
    sif_spacing = 2
    bar_spacing = 0
    bar_height = 0.5
    y = [(x * sif_spacing) for x in range(0, len(sif_files))]

    # Plot the Reference
    plt.barh(
        y,
        ref_mean_runs,
        bar_height,
        xerr=[ref_fastest_runs, ref_slowest_runs],
        color='darkorchid',
        label='Reference',
    )

    # Plot the test
    y = [(x - bar_height - bar_spacing) for x in y]
    plt.barh(
        y,
        test_mean_runs,
        bar_height,
        xerr=[test_fastest_runs, test_slowest_runs],
        color='dodgerblue',
        label='Test',
    )

    # Rest of the chart
    y = [(x + (bar_height / 2)) for x in y]
    plt.xlabel('Run time (seconds), smaller is better')
    plt.ylabel('.sif file')
    plt.yticks(y, sif_files)
    plt.title('Run Time Comparison')
    plt.legend(loc='best')

    plt.show()


if __name__ == '__main__':
    main()
