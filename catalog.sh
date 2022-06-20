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
  ["Node"]="safe"

  # NVTraverse
  ["List0"]="unsafe"
  ["List1"]="unsafe"
  ["List2"]="unsafe"
  ["List3"]="unsafe"
  ["List4"]="unsafe"
  ["ListIz0"]="safe"
  ["ListIz1"]="safe"
  ["ListIz2"]="safe"
  ["ListIz3"]="safe"
  ["ListIz4"]="safe"
  ["ListTr0"]="safe"
  ["ListTr1"]="safe"
  ["ListTr2"]="safe"
  ["ListTr3"]="safe"
  ["ListTr4"]="safe"
  ["Skiplist0"]="unsafe"

  # PersistentQueue
  ["MSQueue0"]="unsafe"
  ["DurableQueue0"]="safe"
)
