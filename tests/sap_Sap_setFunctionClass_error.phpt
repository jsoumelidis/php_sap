--TEST--
Sap::setFunctionClass() error behavior
--FILE--
<?php
$s = new Sap();

/* test accepts string */
try { $s->setFunctionClass([]); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test accepts class name derived from SapFunction without changing default */
$s->setFunctionClass(stdClass::class);
var_dump($s->getFunctionClass());
?>
--EXPECTF--
Argument 1 passed to Sap::setFunctionClass() must be of the type string, array given

Warning: Sap::setFunctionClass() expects parameter 1 to be a class name derived from SapFunction, 'stdClass' given in %s on line %d
string(11) "SapFunction"