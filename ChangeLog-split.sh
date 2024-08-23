#!/bin/bash


# https://stackoverflow.com/questions/59895/how-to-get-the-source-directory-of-a-bash-script-from-within-the-script-itself
SOURCE="${BASH_SOURCE[0]}"
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
CHANGELOG="${DIR}/ChangeLog.md"
if ( grep "AM_CONDITIONAL(DEVELOPMENT_SNAPSHOT, true)" < "${DIR}/synfig-studio/configure.ac" > /dev/null ); then
export CHANGELOG_DEV="${DIR}/ChangeLog-development.md"
fi

MODULEDIR=ETL
[ ! -f "${DIR}/${MODULEDIR}/NEWS" ] || rm -f "${DIR}/${MODULEDIR}/NEWS"
[ -z "${CHANGELOG_DEV}" ] || grep "\[ETL\]" < ${CHANGELOG_DEV} | sed -e "s/ \[ETL\]//g" | sed -e "s/ \[core\]//g" | sed -e "s/ \[studio\]//g" >> "${DIR}/${MODULEDIR}/NEWS"
grep "\[ETL\]" < ${CHANGELOG} | sed -e "s/ \[ETL\]//g" | sed -e "s/ \[core\]//g" | sed -e "s/ \[studio\]//g" >> "${DIR}/${MODULEDIR}/NEWS"
echo >> "${DIR}/${MODULEDIR}/NEWS"
echo "--------------------------" >> "${DIR}/${MODULEDIR}/NEWS"
echo >> "${DIR}/${MODULEDIR}/NEWS"
cat "${DIR}/${MODULEDIR}/ChangeLog.old" >> "${DIR}/${MODULEDIR}/NEWS"

MODULEDIR=synfig-core
[ ! -f "${DIR}/${MODULEDIR}/NEWS" ] || rm -f "${DIR}/${MODULEDIR}/NEWS"
[ -z "${CHANGELOG_DEV}" ] || grep "\[core\]" < ${CHANGELOG_DEV} | sed -e "s/ \[ETL\]//g" | sed -e "s/ \[core\]//g" | sed -e "s/ \[studio\]//g" >> "${DIR}/${MODULEDIR}/NEWS"
grep "\[core\]" < ${CHANGELOG} | sed -e "s/ \[ETL\]//g" | sed -e "s/ \[core\]//g" | sed -e "s/ \[studio\]//g" >> "${DIR}/${MODULEDIR}/NEWS"
echo >> "${DIR}/${MODULEDIR}/NEWS"
echo "--------------------------" >> "${DIR}/${MODULEDIR}/NEWS"
echo >> "${DIR}/${MODULEDIR}/NEWS"
cat "${DIR}/${MODULEDIR}/ChangeLog.old" >> "${DIR}/${MODULEDIR}/NEWS"

MODULEDIR=synfig-studio
[ ! -f "${DIR}/${MODULEDIR}/NEWS" ] || rm -f "${DIR}/${MODULEDIR}/NEWS"
[ -z "${CHANGELOG_DEV}" ] || grep "\[studio\]" < ${CHANGELOG_DEV} | sed -e "s/ \[ETL\]//g" | sed -e "s/ \[core\]//g" | sed -e "s/ \[studio\]//g" >> "${DIR}/${MODULEDIR}/NEWS"
grep "\[studio\]" < ${CHANGELOG} | sed -e "s/ \[ETL\]//g" | sed -e "s/ \[core\]//g" | sed -e "s/ \[studio\]//g" >> "${DIR}/${MODULEDIR}/NEWS"
echo >> "${DIR}/${MODULEDIR}/NEWS"
echo "--------------------------" >> "${DIR}/${MODULEDIR}/NEWS"
echo >> "${DIR}/${MODULEDIR}/NEWS"
cat "${DIR}/${MODULEDIR}/ChangeLog.old" >> "${DIR}/${MODULEDIR}/NEWS"
