transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -vlog01compat -work work +incdir+E:/Quatus_program/DDS {E:/Quatus_program/DDS/Sin.v}
vlog -vlog01compat -work work +incdir+E:/Quatus_program/DDS {E:/Quatus_program/DDS/DDS.v}

