create table ATable(a_text text, a_float float, b_bool bool);
create table BTable(b_text text, b_integer integer, b_float float);
insert into ATable("T1",1.0,false);
insert into ATable("T2",2.0,true);
insert into ATable("T3",3.0,false);
insert into ATable("T4",4.0,true);
insert into ATable("T5",5.0,false);
insert into ATable("T6",6.0,true);
insert into ATable("T7",7.0,false);
insert into ATable("T8",8.0,true);
insert into ATable("T9",9.0,false);
insert into BTable("T1",1,1.0);
insert into BTable("T2",2,2.0);
insert into BTable("T3",3,3.0);
insert into BTable("T4",4,4.0);
insert into BTable("T5",5,5.0);
insert into BTable("T6",6,6.0);
insert into BTable("T7",7,7.0);
insert into BTable("T8",8,8.0);
insert into BTable("T9",9,9.0);
select from ATable;
select from BTable;
update BTable set b_text = "Tupdated1" where b_text == "T9";
update BTable set b_text = "Tupdated2" where b_text contains "T8";
select from ATable;
select from BTable;
delete from ATable where a_text contains "T" and a_float >= 6.0;
select from BTable where b_text contains "T" and a_float >= 6.0;
select from ATable;

select from ATable
join BTable on a_text == b_text
where a_text contains "1" or b_float > 2.0;

select from ATable
join BTable on a_text == b_text
where a_text contains "1";
