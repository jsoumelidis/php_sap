--TEST--
Sap::connect() basic behavior
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

/* test successful connection */
(new Sap())->connect($config);
?>
--EXPECT--