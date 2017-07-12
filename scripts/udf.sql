use udftest;

DROP FUNCTION IF EXISTS udftest.ice_to_char(timestamp,string);
DROP FUNCTION IF EXISTS udftest.ice_to_char(int,string);
DROP FUNCTION IF EXISTS udftest.ice_to_char(double,string);
DROP FUNCTION IF EXISTS udftest.ice_to_char(DECIMAL(18,6),int,string);

create function ice_to_char(timestamp,string) returns string location '/tmp/udf/libice_to_char.so' symbol='ice_to_char';
create function ice_to_char(int,string) returns string location '/tmp/udf/libice_to_char.so' symbol='ice_to_char_int';
create function ice_to_char(double,string) returns string location '/tmp/udf/libice_to_char.so' symbol='ice_to_char_double';
create function ice_to_char(DECIMAL(18,6), int, string) returns string location '/tmp/udf/libice_to_char.so' symbol='ice_to_char_decimal';

show functions;