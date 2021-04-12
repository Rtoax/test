#!/bin/bash

# 收集
sudo perf record -e block:block_rq_issue -e block:block_rq_complete -a sleep 12

# 加工
sudo perf script | awk '{ gsub(/:/, "") } $5 ~ /issue/ { ts[$6, $10] = $4 }
    $5 ~ /complete/ { if (l = ts[$6, $9]) { printf "%.f %.f\n", $4 * 1000000,
		    ($4 - l) * 1000000; ts[$6, $10] = 0 } }' > out.lat_us

# 生成 svg 图
./trace2heatmap.pl --unitstime=us --unitslat=us --maxlat=50000 out.lat_us > out.svg


rm -f perf.data*
