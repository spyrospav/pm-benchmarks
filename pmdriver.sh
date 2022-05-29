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

GENMC=/home/spyros/Desktop/thesis/genmc-tool/src/genmc
PMBENCHMARKS=$(pwd)
OUT=${PMBENCHMARKS}/out

LITMUS=${PMBENCHMARKS}/litmus
NVTRAVERSE=${PMBENCHMARKS}/NVTraverse

GREEN=`tput setaf 2`
#GREEN='\033[32m'
NC=`tput sgr0`

for lit in ${LITMUS}/*
do
  test=$(basename ${lit} .cpp)
  echo -e "${GREEN}${test}${NC}"
  ${GENMC} -disable-race-detection --tso --persevere -- -DPWB_IS_CLFLUSH ${LITMUS}/${test}.cpp >\
   ${OUT}/${test}.out

done


for ds in List Skiplist
do

  ${GENMC} -disable-race-detection --tso --persevere -- -DPWB_IS_CLFLUSH ${NVTRAVERSE}/${ds}/run${ds}.cpp >\
   ${OUT}/pm${ds}.out

done
