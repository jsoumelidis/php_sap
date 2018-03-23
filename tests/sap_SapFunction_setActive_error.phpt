--TEST--
SapFunction::setActive() error behavior
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

try { (new SapFunction)->setActive('test'); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}

try { (new Sap($config))->fetchFunction('RFC_PING')->setActive('NON_EXISTENT', false); }
catch (UnexpectedValueException $e) {
	echo $e->getMessage(), PHP_EOL;
}

?>
--EXPECT--
Function's description has not been fetched
Parameter 'NON_EXISTENT' not found