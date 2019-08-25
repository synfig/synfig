#!/bin/bash

set -x

export PATH="${TRAVIS_BUILD_DIR}/_production/build/bin:$PATH"

ccache -s # show ccache stats
git clone https://github.com/synfig/synfig-tests-regressions.git
bash ./synfig-tests-regressions/test-rendering.sh results
