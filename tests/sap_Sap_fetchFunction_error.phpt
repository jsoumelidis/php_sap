--TEST--
Sap::fetchFunction() error behavior
--SKIPIF--
<?php
$config = include 'config.inc';

if (!$config) {
	print 'skip connection configuration not available';
}
?>
--FILE--
<?php
$config = include 'config.inc';

$s = new Sap();

/** test throws LogicException when not connected */
try { (new Sap())->fetchFunction('RFC_PING'); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test raises warning and returns null if invalid 2nd argument provided */
$f = (new Sap())->fetchFunction('RFC_PING', 'invalid');
var_dump(gettype($f));

$s->connect($config);

/** test throws TypeError if invalid 1st argument provided */
try { $s ->fetchFunction(new stdClass); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test throws SapException if remote function not found */
try { $s->fetchFunction('SOME_INVALID_RFC_NAME'); }
catch (SapException $e) {
	var_dump(get_class($e));
}
?>
--EXPECTF--
There is no connection to a SAP R/3 system

Warning: Sap::fetchFunction() expects parameter 2 to be a class name derived from SapFunction, 'invalid' given in %s on line %d
string(4) "NULL"
Argument 1 of Sap::fetchFunction() must be a string or a SapFunction object (object given)
string(12) "SapException"