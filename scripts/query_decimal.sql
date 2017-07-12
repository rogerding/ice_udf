select
udftest.ice_to_char(col1, 6, '999,999.') as col1,
udftest.ice_to_char(col1, 6, '9') as col2,
udftest.ice_to_char(col1, 6, '99') as col3,
udftest.ice_to_char(col1, 6, '999') as col4,
udftest.ice_to_char(col1, 6, '9999') as col5,
udftest.ice_to_char(col1, 6, '999,999') as col6,
udftest.ice_to_char(col1, 6, '999,999.9') as col7,
udftest.ice_to_char(col1, 6, '999,999.999') as col8,
udftest.ice_to_char(col1, 6, '999,999.999999') as col9,
udftest.ice_to_char(col1, 6, '999,999,999.999999') as col10,
udftest.ice_to_char(col1, 6, '999,999,999,999.999999') as col11,
udftest.ice_to_char(col1, 6, '999,999,999,999,999.999999') as col12
from udftest.decimal_tbl;

