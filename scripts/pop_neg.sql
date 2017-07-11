use udftest;

-- Increase the data in neg_tbl to what ever value is needed. The query can be run over a couple of time to do so
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
from udftest.neg_tbl;



