#!/bin/bash

# Driver script for testing persistent memory programs.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, you can access it online at
# http://www.gnu.org/licenses/gpl-2.0.html.
#
# Author: Spyros Pavlatos <spyrospavlatos4@gmail.com>

GenMC=/home/spyros/Desktop/thesis/genmc-tool/src/genmc
PMBENCHMARKS=$(pwd)

LITMUS=${PMBENCHMARKS}/litmus
NVTRAVERSE=${PMBENCHMARKS}/NVTraverse

RED=`tput setaf 1`
GREEN=`tput setaf 2`
CYAN=`tput setaf 6`
POWDER_BLUE=`tput setaf 153`
NC=`tput sgr0`

mkdir -p ${PMBENCHMARKS}/out
OUT=${PMBENCHMARKS}/out

debug_mode=1

if [ ${debug_mode} == 1 ]
then
  echo
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~DEBUG MODE ON~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
  mkdir -p ${OUT}/graphs
  GRAPHS=${OUT}/graphs
fi

source "${PMBENCHMARKS}/catalog.sh"

printline() {

  for _ in {0..69}; do echo -n '-'; done; echo''

}

print_header() {

  echo
  printline
  echo "${header}"
  printline
  echo
  printline
  printf "| ${CYAN}%-12s${NC} | ${CYAN}%-8s${NC} | ${CYAN}%-6s${NC} | ${CYAN}%-10s${NC} | ${CYAN}%-7s${NC} | ${CYAN}% 8s${NC} |\n" \
  	   "Testcase" "Expected" "Status" "Executions" "Blocked" "Time"
  printline

}

print_single_result() {

  if test "$?" -ne 0
  then
    if [ "${expected}" == "unsafe" ]
    then
      result=1
    else
      result=0
    fi
  else
    if [ "${expected}" == "safe" ]
    then
      result=1
    else
      result=0
    fi
  fi

  if [ ${result} == 1 ]
  then
    rescolour=$GREEN
    res="pass"
  else
    rescolour=$RED
    res="fail"
  fi

  explored=`echo "${output}" | awk '/explored/ { print $6 }'`
  blocked=`echo "${output}" | awk '/blocked/ { print $6 }'`
  time=`echo "${output}" | awk '/time/ { print substr($4, 1, length($4)-1) }'`
  time="${time}" && [[ -z "${time}" ]] && time=0 # if pattern was NOT found

  printf "| ${POWDER_BLUE}%-12s${NC} | % 8s | ${rescolour}% 6s${NC} | % 10s | % 7s | %8s |\n" \
     "${testname}" "${expected}" "${res}" "${explored}" "${blocked}" "${time}s"

}

run_single_test() {

  testname=$(basename ${test} .cpp)
  expected="${expected_results[${testname}]}"

  if [ ${debug_mode} == 1 ]
  then

    output=`${GenMC} -disable-race-detection --tso --persevere \
      --dump-error-graph=${GRAPHS}/${testname}.dot -- -DPWB_IS_CLFLUSH ${test} 2>&1`

    print_single_result

    if [[ "${expected}" == "unsafe" && "${res}" == "pass" ]];
    then

      dot -Tps ${GRAPHS}/${testname}.dot -o ${GRAPHS}/${testname}.ps

    fi

  else

    output=`${GenMC} -disable-race-detection --tso --persevere -- -DPWB_IS_CLFLUSH ${test} 2>&1`

    print_single_result

  fi

  echo "${output}" &> ${OUT}/${testname}.out

}

#
# Run litmus tests
#

header="*                        Running litmus tests                        *"
print_header

for test in ${LITMUS}/*.cpp
do

  run_single_test

done
printline

#
# Run NVTraverse tests
#

header="*                      Running NVTraverse tests                      *"
print_header

for ds in List #Skiplist
do
  for test in ${NVTRAVERSE}/${ds}/*.cpp
  do

    run_single_test

  done
done
printline

echo
