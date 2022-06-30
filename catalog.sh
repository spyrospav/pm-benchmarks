declare -A expected_results

expected_results=(
  # Litmus
  ["2W+2W"]="unsafe"
  ["2WRW+WFW"]="unsafe"
  ["WFW"]="safe"
  ["WFW+RW"]="safe"
  ["WW"]="unsafe"
  ["2W+RFW"]="safe"
  ["CAS+CAS"]="safe"
  ["WW+RMFW"]="safe"
  ["6W"]="safe"
  ["Node"]="safe"

  # NVTraverse
  ["l-ww"]="unsafe"
  ["l-pw+ww"]="unsafe"
  ["l-pw+w+w"]="unsafe"
  ["l-pw+w+ww"]="unsafe"
  ["l-pw+w+d"]="unsafe"
  ["l-pw+w+w+d"]="unsafe"
  ["liz-ww"]="safe"
  ["liz-pw+ww"]="safe"
  ["liz-pw+w+w"]="safe"
  ["liz-pw+w+ww"]="safe"
  ["liz-pw+w+d"]="safe"
  ["liz-pw+w+w+d"]="safe"
  ["ltr-ww"]="safe"
  ["ltr-pw+ww"]="safe"
  ["ltr-pw+w+w"]="safe"
  ["ltr-pw+w+ww"]="safe"
  ["ltr-pw+w+d"]="safe"
  ["ltr-pw+w+w+d"]="safe"
  ["Skiplist0"]="unsafe"
  ["SkiplistIz0"]="safe"

  # PersistentQueue
  ["MSQueue0"]="unsafe"
  ["DurableQueue0"]="safe"
)
