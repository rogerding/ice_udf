select
udftest.ice_to_char(time,'YYYY-MONTH-dy') as col26,
udftest.ice_to_char(time,'DDD') as col27,
udftest.ice_to_char(time,'HH24') as col28,
udftest.ice_to_char(time,'CC') as col29,
udftest.ice_to_char(time,'Q') as col30
from udftest.pos_tbl;

