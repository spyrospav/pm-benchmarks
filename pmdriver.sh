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

# GenMC=/home/spyros/Desktop/genmc-tool/src/genmc
GenMC=genmc
PMBENCHMARKS=$(pwd)
FLUSHFLAG=-DPWB_IS_CLFLUSH

LITMUS=${PMBENCHMARKS}/litmus
NVTRAVERSE=${PMBENCHMARKS}/NVTraverse
PERSISTENT_QUEUE=${PMBENCHMARKS}/PersistentQueue

RED=`tput setaf 1`
GREEN=`tput setaf 2`
CYAN=`tput setaf 6`
POWDER_BLUE=`tput setaf 153`
NC=`tput sgr0`

mkdir -p ${PMBENCHMARKS}/out
OUT=${PMBENCHMARKS}/out

debug_mode=0
run_mode="default"

usage()
{
  echo "Usage: ./pmdriver.sh [-m|--mode run_mode] [-d|--debug] [-h|--help]"
  echo
  echo "  -h,--help     - Prints help manual"
  echo "  -m,--mode     - Selects which mode to run"
  echo "    =default    -   Runs the expected tests"
  echo "    =missing    -   Runs the buggy tests (some FLUSH instructions are missing)"
  echo "    =all        -   Runs both expected and buggy tests"
  echo "  -d,--debug    - Produce debug information like execution graphs and error graphs"
  exit 1
}

while true; do
  case "$1" in
    -m|--mode)
      run_mode="$2"
      shift 2
      ;;
    -d|--debug)
      debug_mode=1
      shift
      ;;
    -h|--help)
      usage
      shift
      ;;
    -*)
      echo "Wrong parameter"
      exit 3
      ;;
    *)
      shift
      break
      ;;
  esac
done

if [ "$run_mode" = "all" ];
then
  run_default=1
  run_missing=1
elif [ "$run_mode" = "default" ];
then
  run_default=1
  run_missing=0
elif [ "$run_mode" = "missing" ];
then
  run_default=0
  run_missing=1
else
  echo "Wrong input for run mode. Check -h(--help) for available options."
  exit 3
fi

if [ ${debug_mode} == 1 ]
then
  echo
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~DEBUG MODE ON~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
  mkdir -p ${OUT}/graphs
  GRAPHS=${OUT}/graphs
fi

source "${PMBENCHMARKS}/catalog.sh"

printline() {

  for _ in {0..75}; do echo -n '-'; done; echo''

}

print_header() {

  header="Running ${header} tests"
  sl1=$(((74-${#header})/2))
  sl2=$((75-$sl1-${#header}))
  echo ${sl}
  echo
  printline
  printf "*%*s%s%*s\n" $((${sl1})) "" "$header" $((${sl2})) "*"
  printline
  echo
  printline
  printf "| ${CYAN}%-18s${NC} | ${CYAN}%-8s${NC} | ${CYAN}%-6s${NC} | ${CYAN}%-10s${NC} | ${CYAN}%-7s${NC} | ${CYAN}% 8s${NC} |\n" \
  	   "Testcase" "Expected" "Status" "Executions" "Blocked" "Time"
  printline

}

print_single_result() {

  if test "$?" -ne 0
  then
    actual_res="unsafe"
  else
    actual_res="safe"
  fi

  if [ "${actual_res}" == "${expected}" ]
  then
    rescolour=$GREEN
    # res="✓"
    res="pass"
  else
    rescolour=$RED
    res="fail"
  fi

  explored=`echo "${output}" | awk '/explored/ { print $6 }'`
  blocked=`echo "${output}" | awk '/blocked/ { print $6 }'`
  time=`echo "${output}" | awk '/time/ { print substr($4, 1, length($4)-1) }'`
  time="${time}" && [[ -z "${time}" ]] && time=0 # if pattern was NOT found

  printf "| ${POWDER_BLUE}%-18s${NC} | % 8s | ${rescolour}% 6s${NC} | % 10s | % 7s | %8s |\n" \
     "${outname}" "${expected}" "${res}" "${explored}" "${blocked}" "${time}s"

}

run_single_test() {

  testname=$(basename ${test} .cpp)
  if [[ $1 != '' ]]
  then
    outname="${testname}-$1"
    u1=$(echo "$1" | awk '{ print toupper($0) }')
    DFLAG="-DB${u1}"
  else
    outname="${testname}"
    DFLAG=""
  fi
  expected="${expected_results[${testname}]}"

  if [ ${debug_mode} == 1 ]
  then

    output=`${GenMC} -disable-race-detection --tso --persevere --dump-error-graph=${GRAPHS}/${testname}.dot \
      --print-error-trace -- ${FLUSHFLAG} ${DFLAG} ${test} 2>&1`

    print_single_result

    if test -f "${GRAPHS}/${testname}.dot"; then
      dot -Tpng ${GRAPHS}/${testname}.dot -o ${GRAPHS}/${testname}.png 2> /dev/null
    fi

  else

    output=`${GenMC} -disable-race-detection --tso --persevere -- ${FLUSHFLAG} ${DFLAG} ${test} 2>&1`

    print_single_result

  fi

  echo "${output}" &> ${OUT}/${outname}.out

}

if [ $run_default = 1 ]
then
  #
  # Run litmus tests
  #

  header="litmus"
  print_header

  outfile=$OUT/litmus.tex
  truncate -s 0 ${outfile}

  for test in ${LITMUS}/*.cpp
  do

    run_single_test

    echo "\tabrow{${testname}}{\\${expected}}{${explored}}{${blocked}}{${time}}" >> ${outfile}

  done
  printline

  #
  # Run NVTraverse tests
  #

  header="NVTraverse"
  print_header

  for ds in Skiplist
  do

    outfile=$OUT/nvtraverse${ds}.tex
    truncate -s 0 ${outfile}

    for test in ${NVTRAVERSE}/${ds}/tests/ad.cpp
    do

      run_single_test

      echo "\tabrow{${testname}}{\\${expected}}{${explored}}{${blocked}}{${time}}" >> ${outfile}

    done

  done

  printline

  #
  # Run PersistentQueue tests
  #

  header="PersistentQueue"
  print_header

  for test in ${PERSISTENT_QUEUE}/*.cpp
  do

    run_single_test

  done
  printline
fi

if [ $run_missing = 1 ]
then

  outfile=$OUT/buggy.tex
  truncate -s 0 ${outfile}

  header="NVTraverse buggy"
  print_header

  LTRAVERSE=${NVTRAVERSE}/List/tests
  STRAVERSE=${NVTRAVERSE}/Skiplist/tests

  for test in ${STRAVERSE}/sliz-pw+w+w.cpp
  do
    run_single_test "smf"
    echo "\tabrow{${testname}}{\\${expected}}{${explored}}{${blocked}}{${time}}" >> ${outfile}
  done
  for test in ${LTRAVERSE}/ltr-pw+w+d.cpp ${LTRAVERSE}/ltr-pw+w+w+d.cpp
  do
    run_single_test "imf"
    echo "\tabrow{${testname}}{\\${expected}}{${explored}}{${blocked}}{${time}}" >> ${outfile}
    run_single_test "icf"
    echo "\tabrow{${testname}}{\\${expected}}{${explored}}{${blocked}}{${time}}" >> ${outfile}
  done

  for test in ${LTRAVERSE}/ltr-pw+d+d.cpp ${LTRAVERSE}/ltr-pw+w+w+d.cpp
  do
    run_single_test "rmf"
    echo "\tabrow{${testname}}{\\${expected}}{${explored}}{${blocked}}{${time}}" >> ${outfile}
    run_single_test "rcf"
    echo "\tabrow{${testname}}{\\${expected}}{${explored}}{${blocked}}{${time}}" >> ${outfile}
  done

  printline

fi

echo
