select
udftest.ice_to_char(time,'HH12') as col16,
udftest.ice_to_char(time,'RM') as col17,
udftest.ice_to_char(time,'rm') as col18,
udftest.ice_to_char(time,'YYYY-MM-DD HH:MI:SS.SSSS AM') as col19,
udftest.ice_to_char(time,'YYYY-MM-DD HH:MI:SS.SSSS a.m') as col20
from udftest.pos_tbl;

