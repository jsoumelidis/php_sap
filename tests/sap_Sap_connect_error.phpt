--TEST--
Sap::connect() error behavior
--FILE--
<?php
$config = include 'config.inc';

$s = new Sap();

/** test accepts array as 1st argument */
try { $s->connect('invalid'); }
catch (TypeError $e ) {
	echo $e->getMessage(), PHP_EOL;
}

/** test does not accept empty array as logon parameters */
try { $s->connect([]); }
catch (InvalidArgumentException $e ) {
	echo $e->getMessage(), PHP_EOL;
}

/** test throws SapConnectionException when invalid parameters provided */
try { $s->connect(['invalid' => 'invalid']); }
catch (SapConnectionException $e ) {
	echo SapConnectionException::class, PHP_EOL; //Message depends on the nw rfc sdk
}
?>
--EXPECT--
Argument 1 passed to Sap::connect() must be of the type array, string given
Logon parameters array must not be empty
SapConnectionException