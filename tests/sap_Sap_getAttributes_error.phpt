--TEST--
Sap::getAttributes error behavior
--FILE--
<?php
$s = new Sap();

try { $s->getAttributes(); }
catch (SapConnectionException $e) {
	echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
There is no connection to a SAP R/3 system
