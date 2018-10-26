#!/bin/bash

set -x 

ccache -s # show ccache stats
git clone https://github.com/synfig/synfig-tests-regressions.git
bash ./synfig-tests-regressions/sources/force-render-png.sh results
