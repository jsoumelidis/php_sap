--TEST--
sap_connect() basic behavior
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

/* test returns resource on successful connection */
$c = sap_connect($config);
var_dump($c);
?>
--EXPECTF--
resource(%d) of type (SAP Connection)