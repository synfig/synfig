#!/bin/bash

set -x 

git clone -b update-for-result-generation https://github.com/synfig/synfig-tests-regressions.git
bash ./synfig-tests-regressions/sources/force-render-png.sh results
