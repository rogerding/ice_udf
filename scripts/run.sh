#!/usr/bin/env bash

# (1) Create tables
CM_HOST="nightly511-unsecure-1.gce.cloudera.com"
IMPALA_DAEMON="nightly511-unsecure-2.gce.cloudera.com"

UDF_DIR="/home/rding/CLionProjects/201707/cmake-build-debug/build"
SCRIPT_DIR="/home/rding/workspace/ice_udf/scripts"

scp ${SCRIPT_DIR}/*.sql ${SCRIPT_DIR}/udf*.sh ${UDF_DIR}/*.so root@${CM_HOST}:/root/

ssh root@${CM_HOST} <<EOF
/root/udf_setup.sh
EOF

