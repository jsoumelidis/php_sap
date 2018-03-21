--TEST--
sap_invoke_function with configuration
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

try { $c = sap_connect($config); }
catch (Throwable $e) {
	echo $e->getMessage(), PHP_EOL;
	exit(0);
}

/* test accepts array as 3rd argument */
try { $r = sap_invoke_function('DUMMY_RFC', $c, 'invalid'); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test accepts null as 3rd argument */
$exports = sap_invoke_function('RFC_PING', $c, null);
var_dump($exports);

/* test invokes remote function and returns array */
$exports = sap_invoke_function('RFC_PING', $c);
var_dump($exports);
?>
--EXPECT--
Argument 3 passed to sap_invoke_function() must be of the type array, string given
array(0) {
}
array(0) {
}