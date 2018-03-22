--TEST--
sap_invoke_function() invocation
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
	echo 'Could not connect: ', $e->getMessage(), PHP_EOL;
	exit(0);
}

/* test invokes remote function and returns array */
$exports = sap_invoke_function('RFC_PING', $c);
var_dump($exports);

$imports = ['REQUTEXT' => 'T€st invoke'];

/* test does not trim export strings */
$exports = sap_invoke_function('STFC_CONNECTION', $c, $imports, false);
var_dump($exports);

/* test trims export strings */
$exports = sap_invoke_function('STFC_CONNECTION', $c, $imports, true);
var_dump($exports);

?>
--EXPECTF--
array(0) {
}
array(2) {
  ["ECHOTEXT"]=>
  string(%d) "T€st invoke                                                                                                                                                                                                                                                    "
  ["RESPTEXT"]=>
  string(%d) "%s"
}
array(2) {
  ["ECHOTEXT"]=>
  string(13) "T€st invoke"
  ["RESPTEXT"]=>
  string(%d) "%s"
}