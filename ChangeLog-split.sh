#!/bin/bash


# https://stackoverflow.com/questions/59895/how-to-get-the-source-directory-of-a-bash-script-from-within-the-script-itself
SOURCE="${BASH_SOURCE[0]}"
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
CHANGELOG="${DIR}/ChangeLog.md"

MODULEDIR=ETL
cat ${CHANGELOG} | grep "\[ETL\]" | sed -e "s/ \[ETL\]//g" | sed -e "s/ \[core\]//g" | sed -e "s/ \[studio\]//g" > "${DIR}/${MODULEDIR}/NEWS"
echo >> "${DIR}/${MODULEDIR}/NEWS"
echo "--------------------------" >> "${DIR}/${MODULEDIR}/NEWS"
echo >> "${DIR}/${MODULEDIR}/NEWS"
cat "${DIR}/${MODULEDIR}/ChangeLog.old" >> "${DIR}/${MODULEDIR}/NEWS"

MODULEDIR=synfig-core
cat ${CHANGELOG} | grep "\[core\]" | sed -e "s/ \[ETL\]//g" | sed -e "s/ \[core\]//g" | sed -e "s/ \[studio\]//g" > "${DIR}/${MODULEDIR}/NEWS"
echo >> "${DIR}/${MODULEDIR}/NEWS"
echo "--------------------------" >> "${DIR}/${MODULEDIR}/NEWS"
echo >> "${DIR}/${MODULEDIR}/NEWS"
cat "${DIR}/${MODULEDIR}/ChangeLog.old" >> "${DIR}/${MODULEDIR}/NEWS"

MODULEDIR=synfig-studio
cat ${CHANGELOG} | grep "\[studio\]" | sed -e "s/ \[ETL\]//g" | sed -e "s/ \[core\]//g" | sed -e "s/ \[studio\]//g" > "${DIR}/${MODULEDIR}/NEWS"
echo >> "${DIR}/${MODULEDIR}/NEWS"
echo "--------------------------" >> "${DIR}/${MODULEDIR}/NEWS"
echo >> "${DIR}/${MODULEDIR}/NEWS"
cat "${DIR}/${MODULEDIR}/ChangeLog.old" >> "${DIR}/${MODULEDIR}/NEWS"
