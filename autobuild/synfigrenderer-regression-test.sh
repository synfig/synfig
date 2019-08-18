#!/bin/bash

set -x

export PATH="${TRAVIS_BUILD_DIR}/_production/build/bin:$PATH"
which synfig

ccache -s # show ccache stats
git clone --depth 1 https://github.com/synfig/synfig-tests-regressions.git
bash ./synfig-tests-regressions/test-rendering.sh results
