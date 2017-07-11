#!/usr/bin/env bash

IMPALA_DAEMON="nightly511-unsecure-2.gce.cloudera.com"
max=10

for i in `seq 1 ${max}`;
do
    impala-shell -B --quiet -i ${IMPALA_DAEMON} -f query_pos_different_format_1.sql &
    impala-shell -B --quiet -i ${IMPALA_DAEMON} -f query_pos_different_format_2.sql &
    impala-shell -B --quiet -i ${IMPALA_DAEMON} -f query_pos_different_format_3.sql &
    impala-shell -B --quiet -i ${IMPALA_DAEMON} -f query_pos_different_format_4.sql &
    impala-shell -B --quiet -i ${IMPALA_DAEMON} -f query_pos_different_format_5.sql &
    impala-shell -B --quiet -i ${IMPALA_DAEMON} -f query_pos_different_format_6.sql &
done
