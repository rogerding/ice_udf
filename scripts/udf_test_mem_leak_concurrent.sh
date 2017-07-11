#!/usr/bin/env bash

IMPALA_DAEMON="nightly511-unsecure-2.gce.cloudera.com"
max=10

for i in `seq 1 ${max}`;
do
    impala-shell -B --quiet -i ${IMPALA_DAEMON} -f query_mem_leak.sql &
done
