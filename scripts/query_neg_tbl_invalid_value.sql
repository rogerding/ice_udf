use udftest;

select time from neg_tbl
where cast (time as timestamp) is NULL;