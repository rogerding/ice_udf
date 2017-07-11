use udftest;

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
from udftest.pos_tbl;