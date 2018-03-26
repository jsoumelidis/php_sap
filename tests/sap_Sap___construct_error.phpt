--TEST--
Sap::__construct() error behavior
--FILE--
<?php
$config = include 'config.inc';

/** test accepts array or null as 1st argument */
try { $s = new Sap('invalid'); }
catch (TypeError $e ) {
	echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
Argument 1 passed to Sap::__construct() must be of the type array, string given