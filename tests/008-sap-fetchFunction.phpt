--TEST--
Sap::fetchFunction()
--SKIPIF--
<?php
$config = include 'config.inc';

if (!$config) {
	print 'skip connection configuration not available';
}
?>
--FILE--
<?php
class InvalidSapFunction {}

class CustomSapFunction extends SapFunction {}

class OtherCustomSapFunction extends SapFunction
{
	private $arg;
	
	public function __construct($arg)
	{
		$this->arg = $arg;
	}
	
	public function getArg()
	{
		return $this->arg;
	}
}

class RfcPing extends OtherCustomSapFunction
{
	public function getName(): string
	{
		return 'RFC_PING';
	}
}

$config = include 'config.inc';

$s = new Sap($config);

/** test fetchFunction returns SapFunction object when fetching remote function */
$f = $s->fetchFunction('RFC_PING');
var_dump(get_class($f));
var_dump($f->getName());

/** test fetchFunction returns custom SapFunction object when a custom function class has been set */
$s->setFunctionClass(CustomSapFunction::class);
$f = $s->fetchFunction('RFC_PING');
var_dump(get_class($f));

/** test fetchFunction returns requested type of SapFunction if 2nd argument provided */
$f = $s->fetchFunction('RFC_PING', SapFunction::class);
var_dump(get_class($f));

/** test fetchFunction properly handles constructor of custom function class */
$f = $s->fetchFunction('RFC_PING', RfcPing::class, ['test']);
var_dump(get_class($f));
var_dump($f->getArg());

/** test fetchFunction returns null if invalid 2nd argument provided */
$f = $s->fetchFunction('RFC_PING', 'invalid');
var_dump(gettype($f));

/** test fetchFunction throws SapException if remote function not found */
try { $f = $s->fetchFunction('SOME_INVALID_RFC_NAME'); }
catch (SapException $e) {
	var_dump(get_class($e));
}

/** test fetchFunction accepts SapFunction object as 1st argument and returns the same object */
$func = new RfcPing('arg');
$f = $s->fetchFunction($func);
var_dump($f === $func);

/** test fetchFunction returns callable object */
var_dump(gettype($f()));
?>
--EXPECTF--
string(11) "SapFunction"
string(8) "RFC_PING"
string(17) "CustomSapFunction"
string(11) "SapFunction"
string(7) "RfcPing"
string(4) "test"

Warning: Sap::fetchFunction() expects parameter 2 to be a class name derived from SapFunction, 'invalid' given in %s on line %d
string(4) "NULL"
string(12) "SapException"
bool(true)
string(5) "array"