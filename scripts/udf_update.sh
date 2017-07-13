#!/usr/bin/env bash

IMPALA_DAEMON="nightly511-unsecure-2.gce.cloudera.com"

hdfs dfs -put /root/libice_to_char_new.so /tmp/udf

impala-shell -i ${IMPALA_DAEMON} -f /root/udf.sql

