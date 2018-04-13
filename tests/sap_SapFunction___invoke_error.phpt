--TEST--
SapFunction::__invoke() error behavior
--FILE--
<?php
$s = new SapFunction();

/* test accepts array as 1st argument */
try { $s('invalid'); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test accepts boolean as 2nd argument */
try { $s([], []); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test throws LogicException if description not available */
try { $s(); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
Argument 1 passed to SapFunction::__invoke() must be of the type array or null, string given
Argument 2 passed to SapFunction::__invoke() must be of the type boolean or null, array given
Function's description has not been fetched