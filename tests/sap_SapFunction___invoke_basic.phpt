--TEST--
SapFunction::__invoke() basic behavior
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

$f = (new Sap($config))->fetchFunction('RFC_PING');

/* test invokes remote function and returns array */
$exports = $f();
var_dump($exports);

$f = (new Sap($config))->fetchFunction('STFC_CONNECTION');

$imports = ['REQUTEXT' => 'T€st invoke'];

/* test does not trim export strings */
$exports = $f($imports, false);
var_dump($exports);

/* test trims export strings */
$exports = $f($imports, true);
var_dump($exports);

ini_set('sap.rtrim_export_strings', 'Off');

/* test uses default sap.rtrim_export_strings setting when argument 4th not provided (#1) */
$exports = $f($imports);
var_dump($exports);

ini_set('sap.rtrim_export_strings', 'On');
/* test uses default sap.rtrim_export_strings setting when argument 4th not provided (#2) */
$exports = $f($imports);
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