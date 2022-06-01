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

  #NVTraverse
  ["List0"]="unsafe"
  ["List1"]="unsafe"
  ["List2"]="unsafe"
  ["ListIz0"]="safe"
  ["ListIz2"]="safe"
  ["ListTr0"]="safe"
  ["ListTr2"]="safe"
  #["runSkiplist"]="safe"
)
