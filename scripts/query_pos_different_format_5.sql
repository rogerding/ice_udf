select
udftest.ice_to_char(time,'YYYY-month-DD') as col21,
udftest.ice_to_char(time,'YYYY-MON-DD') as col22,
udftest.ice_to_char(time,'SSSS-YY') as col23,
udftest.ice_to_char(time,'YYYY-MM,DAY') as col24,
udftest.ice_to_char(time,'YYYY-MONTH-DY') as col25
from udftest.pos_tbl;

