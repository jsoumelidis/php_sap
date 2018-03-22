--TEST--
Sap basic behavior
--FILE--
<?php
class InvalidSapFunction {}

class CustomSapFunction extends SapFunction {}

/** test accepts array as 1st argument */
try { $s = new Sap('invalid'); }
catch (TypeError $e ) {
	echo $e->getMessage(), PHP_EOL;
}

/** test does not accept empty array as logon parameters */
try { $s = new Sap([]); }
catch (InvalidArgumentException $e ) {
	echo $e->getMessage(), PHP_EOL;
}

/** test throws SapConnectionException when invalid parameters provided */
try { $s = new Sap(['invalid' => 'invalid']); }
catch (SapConnectionException $e ) {
	echo SapConnectionException::class, PHP_EOL; //Message depends on the nw rfc sdk
}

/** test constructor no arguments */
$s = new Sap();

/** test constructor passing null as logon parameters is ok */
$s = new Sap(null);

/** test default SapFunction class */
var_dump($s->getFunctionClass());

/** test does not accept a function class not derived from SapFunction */
/** test default function class has not changed */
$s->setFunctionClass(InvalidSapFunction::class);
var_dump($s->getFunctionClass());

/** test default function class has changed */
$s->setFunctionClass(CustomSapFunction::class);
var_dump($s->getFunctionClass());

/** test default function class can be reverted */
$s->setFunctionClass(SapFunction::class);
var_dump($s->getFunctionClass());

/** test Sap::call accepts string as 1st argument */
try { $s->call([]); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test Sap::call accepts array or null as 2nd argument */
try { $s->call('RFC_PING', 'invalid'); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test Sap::call accepts bool or null as 3rd argument */
try { $s->call('RFC_PING', null, []); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test Sap::call throws LogicException if not connected */
try { $s->call('RFC_PING', [], false); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/** test Sap::fetchFunction throws LogicException if not connected */
try { $s->fetchFunction('RFC_PING'); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}

?>
--EXPECTF--
Argument 1 passed to Sap::__construct() must be of the type array or null, string given
Logon parameters array must not be empty
SapConnectionException
string(11) "SapFunction"

Warning: Sap::setFunctionClass() expects parameter 1 to be a class name derived from SapFunction, 'InvalidSapFunction' given in %s on line %d
string(11) "SapFunction"
string(17) "CustomSapFunction"
string(11) "SapFunction"
Argument 1 passed to Sap::call() must be of the type string, array given
Argument 2 passed to Sap::call() must be of the type array or null, string given
Argument 3 passed to Sap::call() must be of the type boolean or null, array given
There is no connection to a SAP R/3 system
There is no connection to a SAP R/3 system
