
select concat('
 ',trim(case when lag(lqd_flag,1,NULL) over (partition by lqd_flag order by lqd_flag, etpid, ordtype) is NULL then lqd_flag else '' end) ,'
 ', '
 ' ,trim(case when lag(etpid,1,NULL) over (partition by lqd_flag, etpid order by lqd_flag, etpid, ordtype) is NULL then etpid else '' end) ,'
 ', '
 ' ,trim(ordtype) ,'
 ', '
 ' ,trim(ice_to_char( cast(volume as int), '9,999,999,999,999,999')) ,'
 ', '
 ' ,trim(ice_to_char( cast(trd_count as int), '9,999,999,999,999,999')) ,'
 ', '
 ' ,trim(ice_to_char( cast(avg5_vol as int), '9,999,999,999,999,999')) ,'
 ', '
 ' ,trim(ice_to_char( cast(avg30_vol as int), '9,999,999,999,999,999')) ,'
 ') from mem_leak order by lqd_flag, etpid, ordtype
;
 
