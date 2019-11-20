#!/usr/bin/env bash

set -xe

MYDIR="$(cd "$(dirname "$0")"; pwd -P)"

bash "${MYDIR}/../tests/future_import_tests.sh"

if [ ! -z ${TRAVIS} ];then
    source ci_support/setup_conda.sh
fi

if [ ! -z ${CIRCLECI} ];then
    source ${HOME}/miniconda2/bin/activate root
    conda install conda-forge/label/cf201901::boost=1.69 -c conda-forge --yes --quiet
fi

python -m compileall -q .

if [ ! -z "$JENKINS_HOME" ];then
    export CPU_COUNT=4
else
    export CPU_COUNT=2
fi

conda info -a
conda list
conda list --explicit
conda render recipes/eman
conda build purge-all

conda build recipes/eman -c cryoem -c defaults -c conda-forge
