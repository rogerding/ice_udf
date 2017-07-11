select
udftest.ice_to_char(time,'SSSS') as col1,
udftest.ice_to_char(time,'SSSS.SS') as col2,
udftest.ice_to_char(time,'SSSS-YY') as col3,
udftest.ice_to_char(time,'HH:MM') as col4,
udftest.ice_to_char(time,'MI') as col5
from udftest.pos_tbl;

