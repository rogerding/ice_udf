use udftest;

select
    max(time), city
from pos_tbl
where
  ice_to_char(time,'HH:MI:SS') > '06:31:00' and
  ice_to_char(time,'HH:MI:SS') < '07:30:30' and
  ice_to_char(time,'SS') > '26000' and
  ice_to_char(time,'YYYY') > '2001'
group by city, ice_to_char(time,'YYYY-MM-DD') ;