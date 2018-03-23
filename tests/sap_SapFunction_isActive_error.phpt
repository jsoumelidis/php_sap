--TEST--
SapFunction::isActive() error behavior
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

try { (new SapFunction)->isActive('test'); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}

try { (new Sap($config))->fetchFunction('RFC_PING')->isActive('NON_EXISTENT'); }
catch (UnexpectedValueException $e) {
	echo $e->getMessage(), PHP_EOL;
}

?>
--EXPECT--
Function's description has not been fetched
Parameter 'NON_EXISTENT' not found