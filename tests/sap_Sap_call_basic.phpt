--TEST--
Sap::call() basic behavior
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

$s = new Sap($config);

/** test returns array */
$r = $s->call('RFC_PING');
var_dump($r);
?>
--EXPECT--
array(0) {
}