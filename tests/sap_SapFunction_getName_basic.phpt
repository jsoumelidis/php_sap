--TEST--
SapFunction::getName() basic behavior
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

$f = (new Sap($config))->fetchFunction('RFC_PING');
var_dump($f->getName());
?>
--EXPECT--
string(8) "RFC_PING"
