create database if not exists udftest;

use udftest;

drop table if exists udftest.neg_tbl;
drop table if exists udftest.pos_tbl;
drop table if exists udftest.err_tbl;
drop table if exists udftest.mem_leak;

-- Create a partitioned tables based off of web_logs, default table availble in CDH 5.11
CREATE TABLE udftest.neg_tbl(
  _version_ BIGINT,
  app STRING,
  bytes SMALLINT,
  city STRING,
  client_ip STRING,
  code TINYINT,
  country_code STRING,
  country_code3 STRING,
  country_name STRING,
  device_family STRING,
  extension STRING,
  latitude FLOAT,
  longitude FLOAT,
  method STRING,
  os_family STRING,
  os_major STRING,
  protocol STRING,
  record STRING,
  referer STRING,
  region_code BIGINT,
  request STRING,
  subapp STRING,
  time STRING,
  url STRING,
  user_agent STRING,
  user_agent_family STRING,
  user_agent_major STRING,
  id STRING
 )
 PARTITIONED BY (
  date1 STRING
 )
STORED AS PARQUET;

-- Insert the date into "neg_tbl" table from web logs. We will use the time column for our UDF.
insert into udftest.neg_tbl partition(date1='2010-10-10')
 select _version_,
 app,
 bytes,
 city,
 client_ip,
 code,
 country_code,
 country_code3,
 country_name,
 device_family,
 extension,
 latitude,
 longitude,
 method,
 os_family,
 os_major,
 protocol,
 record,
 referer,
 region_code,
 request,
 subapp,
 time,
 url,
 user_agent,
 user_agent_family,
 user_agent_major,
 id
from default.web_logs;

-- 11. Create another table "pos_tbl" to hold valid timestamp values only
CREATE TABLE udftest.pos_tbl(
  _version_ BIGINT,
  app STRING,
  bytes SMALLINT,
  city STRING,
  client_ip STRING,
  code TINYINT,
  country_code STRING,
  country_code3 STRING,
  country_name STRING,
  device_family STRING,
  extension STRING,
  latitude FLOAT,
  longitude FLOAT,
  method STRING,
  os_family STRING,
  os_major STRING,
  protocol STRING,
  record STRING,
  referer STRING,
  region_code BIGINT,
  request STRING,
  subapp STRING,
  time STRING,
  url STRING,
  user_agent STRING,
  user_agent_family STRING,
  user_agent_major STRING,
  id STRING)
STORED AS PARQUET;


-- Insert the data into "pos_tbl"
insert into pos_tbl
 select _version_,
 app,
 bytes,
 city,
 client_ip,
 code,
 country_code,
 country_code3,
 country_name,
 device_family,
 extension,
 latitude,
 longitude,
 method,
 os_family,
 os_major,
 protocol,
 record,
 referer,
 region_code,
 request,
 subapp,
 time,
 url,
 user_agent,
 user_agent_family,
 user_agent_major,
 id
from udftest.neg_tbl
where
 date1='2010-10-10'
and cast(time as timestamp) is not NULL;


-- Also create an err_tbl to house timestamp column without date
create table udftest.err_tbl(col1 string) stored as parquet;

insert into err_tbl values ('09:03:00.012698000'), ('09:06:00.012698000'), ('09:08:00.012698000'), ('09:09:00.012698000'),('09:10:00.012698000');

-- For memory Leak testing - create table "mem_leak"
CREATE TABLE udftest.mem_leak (
  lqd_flag VARCHAR(3),
  ordtype STRING,
  etpid STRING,
  volume DECIMAL(38,0),
  trd_count BIGINT,
  avg5_vol DECIMAL(18,6),
  avg30_vol DECIMAL(18,6)
)
STORED AS TEXTFILE;

INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit","AKAMC",366938,3277,362322.600000,290574.133333;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","FMCOA",37059,327,23627.200000,21613.296296;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","NITEB",16457,45,26667.600000,25861.900000;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","BMOCA",163399,1134,214400.000000,129832.266667;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","SCMCA",300,3,350.000000,19950.000000;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","EWTTA",149917,1007,196356.000000,297914.033333;
INSERT into mem_leak select cast("Add" as VARCHAR(3)),"ISO Limit","RCMAA",550,2,550.000000,892.750000;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","SUFIB",204262,190,99161.800000,159654.000000;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","JPMSA",11147562,68703,11020795.8,12211564.266667;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","FQLSA",35413332,231163,32959641.8,32813788.633333;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","RBCMA",242382,1199,675181.800000,1041705.433333;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO ALO Limit","MLUAA",8391,15,10715.000000,6659.266667;
INSERT into mem_leak select cast("Add" as VARCHAR(3)),"ISO ALO Limit","MLTVA",64541,114,76574.400000,81036.133333;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","CANTA",27675,87,32581.600000,68315.000000;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","CDRGA",422470,3322,421842.400000,574083.566667 ;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","CISTA",100,1,100.000000,255.333333 ;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","HOLLA",1881917,9338,2373105.200000,2410131.700000 ;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit","RCMAA",400,1,171.666667,748.071429 ;
INSERT into mem_leak select cast("Add" as VARCHAR(3)),"ISO Limit","TMBRA",239600,2911,412581.200000,546666.466667 ;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","WEXXA",66366,470,111158.600000,110521.766667 ;
INSERT into mem_leak select cast("Rmv" as VARCHAR(3)),"ISO Limit IOC","PIPRA",1750,11,3035.000000,7946.333333 ;


create table formats ( ts string, fmt string) stored as TEXTFILE;

insert into formats select '2010-10-10 10:10:10' , 'YYYY-MM-DD' ;
insert into formats select '2010-10-10 10:10:10' , 'SSSS' ;
insert into formats select '2010-10-10 10:10:10' , 'SSSS.SS' ;
insert into formats select '2010-10-10 10:10:10' , 'SSSS-YY' ;
insert into formats select '2010-10-10 10:10:10' , 'HH:MM' ;
insert into formats select '2010-10-10 10:10:10' , 'MI' ;
insert into formats select '2010-10-10 10:10:10' , 'SS' ;
insert into formats select '2010-10-10 10:10:10' , 'YYYY' ;
insert into formats select '2010-10-10 10:10:10' , 'HH24' ;
insert into formats select '2010-10-10 10:10:10' , 'J' ;
insert into formats select '2010-10-10 10:10:10' , 'JJ' ;
insert into formats select '2010-10-10 10:10:10' , 'SSJ' ;
insert into formats select '2010-10-10 10:10:10' , 'MM' ;
insert into formats select '2010-10-10 10:10:10' , 'DDDD' ;
insert into formats select '2010-10-10 10:10:10' , 'YYMMDD' ;
insert into formats select '2010-10-10 10:10:10' , 'YYDDMM' ;
insert into formats select '2010-10-10 10:10:10' , 'MM' ;
insert into formats select '2010-10-10 10:10:10' , 'HH12' ;
insert into formats select '2010-10-10 10:10:10' , 'RM' ;
insert into formats select '2010-10-10 10:10:10' , 'rm' ;
insert into formats select '2010-10-10 10:10:10' , 'YYYY-MM-DD HH:MI:SS.SSSS AM' ;
insert into formats select '2010-10-10 10:10:10' , 'YYYY-MM-DD HH:MI:SS.SSSS a.m' ;
insert into formats select '2010-10-10 10:10:10' , 'YYYY-month-DD' ;
insert into formats select '2010-10-10 10:10:10' , 'YYYY-MON-DD' ;
insert into formats select '2010-10-10 10:10:10' , 'YYYY-MM,DAY' ;
insert into formats select '2010-10-10 10:10:10' , 'YYYY-MONTH-DY';
insert into formats select '2010-10-10 10:10:10' , 'YYYY-MONTH-dy' ;
insert into formats select '2010-10-10 10:10:10' , 'DDD' ;
insert into formats select '2010-10-10 10:10:10' , 'CC' ;
insert into formats select '2010-10-10 10:10:10' , 'J' ;
insert into formats select '2010-10-10 10:10:10' , 'Q' ;



CREATE TABLE udftest.decimal_tbl (
   col1 DECIMAL(18,6)
)
STORED AS TEXTFILE;
insert into decimal_tbl values (1), (1.1), (1.11111), (1.8), (1000000000.888888);
insert into decimal_tbl values(-922337203685.999999), (922372036853.99999), (1000000000.8000);
