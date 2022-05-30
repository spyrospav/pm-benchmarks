declare -A expected_results

expected_results=(
  ["2W+2W"]="unsafe"
  ["2WRW+WFW"]="unsafe"
  ["WFW"]="safe"
  ["WFW+RW"]="safe"
  ["WW"]="unsafe"
)
