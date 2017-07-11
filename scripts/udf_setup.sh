#!/usr/bin/env bash

CM_HOST="nightly511-unsecure-1.gce.cloudera.com"
IMPALA_DAEMON="nightly511-unsecure-2.gce.cloudera.com"

NEG_TBL_POP_LOOP=12
MEM_LEAK_POP_LOOP=18

#####
## How many rows required to
#####
# (1) Create tables
# create 4 tables: neg_tbl, pos_tbl, err_tbl, mem_leak
# also populate neg_tbl(initial), pos_tbl(initial), err_tbl, mem_leak
impala-shell -i ${IMPALA_DAEMON} -f /root/setup.sql

# (2) continue populate and neg_tbl and pos_tbl table
for i in `seq 1 ${NEG_TBL_POP_LOOP}`
do
#echo ${i}
    impala-shell -i ${IMPALA_DAEMON} -f /root/pop_neg.sql
    impala-shell -i ${IMPALA_DAEMON} -f /root/pop_pos.sql
done

# (2) continue populate and mem_leak table
for i in `seq 1 ${MEM_LEAK_POP_LOOP}`
do
#echo ${i}
    impala-shell -i ${IMPALA_DAEMON} -f /root/pop_memleak.sql
done

# count neg_tbl and pos_tbl table
impala-shell -i ${IMPALA_DAEMON} -f /root/count_neg.sql
impala-shell -i ${IMPALA_DAEMON} -f /root/count_pos.sql

# create UDF
hdfs dfs -mkdir '/tmp/udf'
hdfs dfs -put /root/*.so /tmp/udf

impala-shell -i ${IMPALA_DAEMON} -f /root/udf.sql

# list files
hdfs dfs -ls /user/hive/warehouse/udftest.db/neg_tbl/date1=2010-10-10
hdfs dfs -ls /user/hive/warehouse/udftest.db/pos_tbl
