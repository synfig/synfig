dnl

 define(`forloop',
		`pushdef(`$1', `$2')_forloop(`$1', `$2', `$3', `$4')popdef(`$1')')
 define(`_forloop',
		`$4`'ifelse($1, `$3', ,
			   `define(`$1', incr($1))_forloop(`$1', `$2', `$3', `$4')')')

define(`_PRINT_ARGS',`dnl
ifelse($#,1,,`$2 v$1`'ifelse($#,2,,`, _PRINT_ARGS(incr($1), shift(shift($@)))')')dnl
')dnl

define(`_PRINT_ARGS2',`dnl
ifelse($#,1,,`v$1`'ifelse($#,2,,`, _PRINT_ARGS2(incr($1), shift(shift($@)))')')dnl
')dnl

dnl PX_DEFINE_FUNC(func_name, ret_type, args...)
define(`PX_DEFINE_FUNC',`
	sigc::slot< $2`'ifelse($#,2,,`, shift(shift($@))') > _slot_$1;
	$2 $1(ifelse($#,2,,`_PRINT_ARGS(1,shift(shift($@)))')) {
		return _slot_$1(ifelse($#,2,,`_PRINT_ARGS2(1,shift(shift($@)))'));
	}
')dnl

dnl PX_DEFINE_FUNC_CONST(func_name, ret_type, args...)
define(`PX_DEFINE_FUNC_CONST',`
	sigc::slot< $2`'ifelse($#,2,,`, shift(shift($@))') > _slot_$1_const;
	$2 $1(ifelse($#,2,,`_PRINT_ARGS(1,shift(shift($@)))'))const {
		return _slot_$1_const(ifelse($#,2,,`_PRINT_ARGS2(1,shift(shift($@)))'));
	}
')dnl

define(`PX_DEFINE_DATA', `dnl
PX_DEFINE_FUNC_CONST(get_$1, $2)
PX_DEFINE_FUNC(set_$1, void, $2)
')dnl

