#!/bin/bash

set -x

export SCRIPTPATH=$(cd `dirname "$0"`; pwd)

export PATH="${SCRIPTPATH}/../_production/build/bin:$PATH"

ccache -s # show ccache stats
cd "${SCRIPTPATH}/../_production/"
if [ ! -d synfig-tests ]; then
    git clone https://gitlab.com/synfig/synfig-tests.git
else
    cd synfig-tests
    git pull
    cd ..
fi
bash ./synfig-tests/test-rendering.sh results
