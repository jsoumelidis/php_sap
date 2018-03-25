--TEST--
SapRfcReadTable::select() error behavior
--FILE--
<?php

/* test $fields argument does not accept invalid value */
try { (new SapRfcReadTable)->select(5, 'TABLE'); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $fields argument must not be empty string */
try { (new SapRfcReadTable)->select('', 'TABLE'); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $fields of type array must contain only strings (field names) */
try { (new SapRfcReadTable)->select([5], 'TABLE'); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $table argument must be string */
try { (new SapRfcReadTable)->select('*', []); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $table argument must not be empty string */
try { (new SapRfcReadTable)->select('*', ''); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $where argument must be array or null */
try { (new SapRfcReadTable)->select('*', 'TABLE', 'invalid'); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $limit argument must be integer */
try { (new SapRfcReadTable)->select('*', 'TABLE', null, []); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $limit argument does not accept negative integer */
try { (new SapRfcReadTable)->select('*', 'TABLE', null, -1); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $offset argument must be integer */
try { (new SapRfcReadTable)->select('*', 'TABLE', null, 5, []); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $offset argument does not accept negative integer */
try { (new SapRfcReadTable)->select('*', 'TABLE', null, 0, -3); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $rtrim argument must be boolean or null */
try { (new SapRfcReadTable)->select('*', 'TABLE', null, 5, 0, []); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test throws LogicException if connection not available */
try { (new SapRfcReadTable)->select('*', 'TABLE'); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECTF--
Argument 1 passed to SapRfcReadTable::select() must be of the type string or array or null, integer given
Argument 1 passed to SapRfcReadTable::select() must not be an empty string
Field names must be strings (integer detected)
Argument 2 passed to SapRfcReadTable::select() must be of the type string, array given
Argument 2 passed to SapRfcReadTable::select() must not be an empty string
Argument 3 passed to SapRfcReadTable::select() must be of the type array, string given
Argument 4 passed to SapRfcReadTable::select() must be of the type integer, array given
Argument 4 passed to SapRfcReadTable::select() must not be negative (-1)
Argument 5 passed to SapRfcReadTable::select() must be of the type integer, array given
Argument 5 passed to SapRfcReadTable::select() must not be negative (-3)
Argument 6 passed to SapRfcReadTable::select() must be of the type boolean, array given
There is no connection to a SAP R/3 system