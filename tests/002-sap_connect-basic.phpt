--TEST--
sap_connect basic behavior
--FILE--
<?php
/* test does not accept empty array */
try { $c = sap_connect([]); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test throws SapException on invalid connection arguments */
try { $c = sap_connect(['invalid' => 'invalid']); }
catch (SapException $e) {
	echo get_class($e), PHP_EOL;
}
?>
--EXPECTF--
Logon parameters array must not be empty
SapException
