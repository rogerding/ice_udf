select
udftest.ice_to_char(time,'SS') as col6,
udftest.ice_to_char(time,'YYYY') as col7,
udftest.ice_to_char(time,'HH24') as col8,
udftest.ice_to_char(time,'J') as col9,
udftest.ice_to_char(time,'JJ') as col10
from udftest.pos_tbl;

