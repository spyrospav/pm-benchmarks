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
OUT=${PMBENCHMARKS}/out

LITMUS=${PMBENCHMARKS}/litmus
NVTRAVERSE=${PMBENCHMARKS}/NVTraverse

RED=`tput setaf 1`
GREEN=`tput setaf 2`
CYAN=`tput setaf 6`
POWDER_BLUE=`tput setaf 153`
NC=`tput sgr0`

printline() {
  for _ in {0..60}; do echo -n '-'; done; echo''
}

print_header() {
  echo
  printline
  echo "${header}"
  printline
  echo
  printline
  printf "| ${CYAN}%-14s${NC} | ${CYAN}%-6s${NC} | ${CYAN}%-10s${NC} | ${CYAN}%-7s${NC} | ${CYAN}%-8s${NC} |\n" \
  	   "Testcase" "Result" "Executions" "Blocked" "Time"
  printline
}

print_single_result() {
  explored=`echo "${output}" | awk '/explored/ { print $6 }'`
	blocked=`echo "${output}" | awk '/blocked/ { print $6 }'`
	time=`echo "${output}" | awk '/time/ { print substr($4, 1, length($4)-1) }'`
	time="${time}" && [[ -z "${time}" ]] && time=0 # if pattern was NOT found

  printf "| ${POWDER_BLUE}%-14s${NC} | ${rescolour}% 6s${NC} | % 10s | % 7s | %8s |\n" \
	   "${test}" "${res}" "${explored}" "${blocked}" "${time}s"
}

# Run litmus tests

header="*                    Running litmus tests                   *"
print_header

for lit in ${LITMUS}/*
do

  test=$(basename ${lit} .cpp)

  output=`${GenMC} -disable-race-detection --tso --persevere -- -DPWB_IS_CLFLUSH ${LITMUS}/${test}.cpp 2>&1`
  if test "$?" -ne 0
  then
    rescolour=$RED
    res="no"
  else
    rescolour=$GREEN
    res="yes"
  fi

  print_single_result

  ${GenMC} -disable-race-detection --tso --persevere -- -DPWB_IS_CLFLUSH ${LITMUS}/${test}.cpp >\
   ${OUT}/${test}.out

done
printline

for ds in List Skiplist
do

  ${GenMC} -disable-race-detection --tso --persevere -- -DPWB_IS_CLFLUSH ${NVTRAVERSE}/${ds}/run${ds}.cpp >\
   ${OUT}/pm${ds}.out

done
