--TEST--
Sap::__construct basic behavior
--SKIPIF--
<?php
$config = include 'config.inc';

if (!$config) {
	print 'skip connection configuration not available';
}
?>
--FILE--
<?php
/** test constructor no arguments */
$s = new Sap();

/** test connect() is not called */
try { $s->getAttributes(); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test connect() is not called */
try { $s->getAttributes(); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}

$config = include 'config.inc';

/** test can connect */
$s = new Sap($config);
?>
--EXPECT--
There is no connection to a SAP R/3 system
There is no connection to a SAP R/3 system
