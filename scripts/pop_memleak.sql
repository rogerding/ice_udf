use udftest;

-- Insert the data into "mem_leak"
insert into mem_leak
 select lqd_flag,
  ordtype,
  etpid,
  volume,
  trd_count,
  avg5_vol,
  avg30_vol
from udftest.mem_leak;