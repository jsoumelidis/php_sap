--TEST--
SapRfcReadTable::describe() error behavior
--FILE--
<?php

/* test accepts only string as $table argument */
try { (new SapRfcReadTable)->describe([]); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test accepts only non-empty string as $table argument */
try { (new SapRfcReadTable)->describe(''); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $fields argument must be array */
try { (new SapRfcReadTable)->describe('TABLE', 'invalid'); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $fields of type array must contain only strings (field names) */
try { (new SapRfcReadTable)->describe('TABLE', [6]); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test $fields of type array must contain only non empty strings */
try { (new SapRfcReadTable)->describe('TABLE', ['']); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test throws LogicException if connection not available */
try { (new SapRfcReadTable)->describe('TABLE'); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECTF--
Argument 1 passed to SapRfcReadTable::describe() must be of the type string, array given
Argument 1 passed to SapRfcReadTable::describe() must be a non-empty string
Argument 2 passed to SapRfcReadTable::describe() must be of the type array, string given
Argument 2 passed to SapRfcReadTable::describe() must be an array of strings, element of type integer detected
Argument 2 passed to SapRfcReadTable::describe() must be an array of non-empty strings, empty string detected
There is no connection to a SAP R/3 system