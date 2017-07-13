# ice_udf

scripts: bash script and impala sql files

src: UDF c++ source code

## Testing Environment
Cloudera nightlies cluster

http://nightly511-unsecure-1.gce.cloudera.com:7180

2 impala daemons, 16G

## Testing Procedure
(1) build UDF

(2) run.sh: 
    
    copy files to nightly511-unsecure-1.gce.cloudera.com:/root/
    create tables and populate with data
    
(3) ssh to nightly511-unsecure-1.gce.cloudera.com, run the test
    
    udf_test_pos_where_clause.sh
    udf_test_multiple_format_concurrent.sh
