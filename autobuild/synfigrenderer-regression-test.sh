#!/bin/bash

set -x 

ccache -s # show ccache stats
git clone -b update-for-result-generation https://github.com/reSHARMA/synfig-tests-regressions.git
bash ./synfig-tests-regressions/sources/force-render-png.sh results
