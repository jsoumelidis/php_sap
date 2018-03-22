--TEST--
Sap::call() error behavior
--FILE--
<?php
$s = new Sap();

/** test Sap::call accepts string as 1st argument */
try { $s->call([]); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test Sap::call accepts array as 2nd argument */
try { $s->call('RFC_PING', 'invalid'); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test Sap::call accepts bool as 3rd argument */
try { $s->call('RFC_PING', null, []); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test Sap::call throws LogicException if not connected */
try { $s->call('RFC_PING', [], false); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
Argument 1 passed to Sap::call() must be of the type string, array given
Argument 2 passed to Sap::call() must be of the type array, string given
Argument 3 passed to Sap::call() must be of the type boolean, array given
There is no connection to a SAP R/3 system