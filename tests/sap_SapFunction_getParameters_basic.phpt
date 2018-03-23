--TEST--
SapFunction::getParameters() basic behavior
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

$f = (new Sap($config))->fetchFunction('STFC_STRUCTURE');

var_dump($f->getParameters());
?>
--EXPECT--
array(4) {
  [0]=>
  string(10) "ECHOSTRUCT"
  [1]=>
  string(8) "RESPTEXT"
  [2]=>
  string(12) "IMPORTSTRUCT"
  [3]=>
  string(8) "RFCTABLE"
}