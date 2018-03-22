--TEST--
sap_connect() basic behavior
--FILE--
<?php
/* test accepts array */
try { $c = sap_connect('string'); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test does not accept empty array */
try { $c = sap_connect([]); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test throws SapConnectionException on invalid connection arguments */
try { $c = sap_connect(['invalid' => 'invalid']); }
catch (SapConnectionException $e) {
	echo get_class($e), PHP_EOL;
}
?>
--EXPECT--
Argument 1 passed to sap_connect() must be of the type array, string given
Logon parameters array must not be empty
SapConnectionException
