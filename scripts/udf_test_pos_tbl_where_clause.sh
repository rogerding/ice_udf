#!/usr/bin/env bash

IMPALA_DAEMON="nightly511-unsecure-2.gce.cloudera.com"
max=5

for i in `seq 1 ${max}`;
do
    impala-shell -B --quiet -i ${IMPALA_DAEMON} -f query_pos_tbl_where_clause.sql &
done
