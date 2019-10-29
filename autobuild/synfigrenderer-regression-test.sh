#!/bin/bash

set -x

export PATH="${TRAVIS_BUILD_DIR}/_production/build/bin:$PATH"

ccache -s # show ccache stats
git clone https://gitlab.com/synfig/synfig-tests.git
bash ./synfig-tests/test-rendering.sh results
