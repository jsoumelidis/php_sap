--TEST--
sap_connect() error behavior
--FILE--
<?php
$config = include 'config.inc';

/* test accepts array */
try { $c = sap_connect(null); }
catch (InvalidArgumentException $e) {
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
Argument 1 passed to sap_connect() must be of the type array, null given
Logon parameters array must not be empty
SapConnectionException
