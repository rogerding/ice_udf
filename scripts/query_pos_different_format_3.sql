select
udftest.ice_to_char(time,'SSJ') as col11,
udftest.ice_to_char(time,'MM') as col12,
udftest.ice_to_char(time,'DDDD') as col13,
udftest.ice_to_char(time,'YYMMDD') as col14,
udftest.ice_to_char(time,'YYDDMM') as col15
from udftest.pos_tbl;

