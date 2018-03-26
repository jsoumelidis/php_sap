--TEST--
Sap::setFunctionClass() error behavior
--FILE--
<?php
$s = new Sap();

/* test accepts class name derived from SapFunction without changing default */
try { $s->setFunctionClass(stdClass::class); }
catch (InvalidArgumentException $e) {
    echo $e->getMessage(), PHP_EOL;
}

var_dump($s->getFunctionClass());
?>
--EXPECTF--
Sap::setFunctionClass() expects parameter 1 to be a class name derived from SapFunction, 'stdClass' given
string(11) "SapFunction"