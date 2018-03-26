--TEST--
Sap::call() error behavior
--FILE--
<?php
$config = include 'config.inc';

$s = new Sap();

/** test Sap::call accepts string as 1st argument */
try { $s->call([]); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test Sap::call accepts array or null as 2nd argument */
try { $s->call('RFC_PING', 'invalid'); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test Sap::call accepts bool or null as 3rd argument */
try { $s->call('RFC_PING', [], []); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test Sap::call throws LogicException if not connected */
try { $s->call('RFC_PING', [], false); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
Sap::call() expects parameter 1 to be string, array given
Argument 2 passed to Sap::call() must be of the type array, string given
Sap::call() expects parameter 3 to be boolean, array given
There is no connection to a SAP R/3 system