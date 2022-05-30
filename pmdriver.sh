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

source "${PMBENCHMARKS}/catalog.sh"

OUT=${PMBENCHMARKS}/out

LITMUS=${PMBENCHMARKS}/litmus
NVTRAVERSE=${PMBENCHMARKS}/NVTraverse

RED=`tput setaf 1`
GREEN=`tput setaf 2`
CYAN=`tput setaf 6`
POWDER_BLUE=`tput setaf 153`
NC=`tput sgr0`

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
  printf "| ${CYAN}%-14s${NC} | ${CYAN}%-6s${NC} | ${CYAN}%-6s${NC} | ${CYAN}%-10s${NC} | ${CYAN}%-7s${NC} | ${CYAN}% 8s${NC} |\n" \
  	   "Testcase" "Result" "Status" "Executions" "Blocked" "Time"
  printline

}

print_single_result() {

  if test "$?" -ne 0
  then
    if [ "${expected_results[${test}]}" == "unsafe" ]
    then
      result=1

    else
      result=0
    fi
  else
    if [ "${expected_results[${test}]}" == "safe" ]
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

  printf "| ${POWDER_BLUE}%-14s${NC} | % 6s | ${rescolour}% 6s${NC} | % 10s | % 7s | %8s |\n" \
     "${test}" "${expected_results[${test}]}" "${res}" "${explored}" "${blocked}" "${time}s"

}

#
# Run litmus tests
#

header="*                        Running litmus tests                        *"
print_header

for lit in ${LITMUS}/*.cpp
do

  test=$(basename ${lit} .cpp)

  output=`${GenMC} -disable-race-detection --tso --persevere -- -DPWB_IS_CLFLUSH ${LITMUS}/${test}.cpp 2>&1`

  print_single_result
  ${GenMC} -disable-race-detection --tso --persevere -- -DPWB_IS_CLFLUSH ${LITMUS}/${test}.cpp > \
    ${OUT}/${test}.out
done
printline

#
# Run NVTraverse tests
#

header="*                      Running NVTraverse tests                      *"
print_header

for ds in List Skiplist
do

  test=${ds}

  output=`${GenMC} -disable-race-detection --tso --persevere -- -DPWB_IS_CLFLUSH ${NVTRAVERSE}/${ds}/run${ds}.cpp 2>&1`

  ${GenMC} -disable-race-detection --tso --persevere -- -DPWB_IS_CLFLUSH ${NVTRAVERSE}/${ds}/run${ds}.cpp >\
   ${OUT}/pm${ds}.out

done
