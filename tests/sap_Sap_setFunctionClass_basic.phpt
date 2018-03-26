--TEST--
Sap::setFunctionClass() basic behavior
--FILE--
<?php
$config = include 'config.inc';

class CustomSapFunction extends SapFunction { }

$s = new Sap();

$s->setFunctionClass(CustomSapFunction::class);
var_dump($s->getFunctionClass());
?>
--EXPECT--
string(17) "CustomSapFunction"