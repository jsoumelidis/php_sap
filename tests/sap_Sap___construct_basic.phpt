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

/** test constructor passing null as logon parameters is ok */
$s = new Sap(null);

$config = include 'config.inc';

$s = new Sap($config);
?>
--EXPECT--