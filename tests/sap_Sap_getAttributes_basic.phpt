--TEST--
Sap::getAttributes
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

$s = new Sap($config);

var_dump($s->getAttributes());
?>
--EXPECTF--
array(20) {
  ["client"]=>
  string(%d) %s
  ["codepage"]=>
  string(%d) %s
  ["cpicConvId"]=>
  string(%d) %s
  ["dest"]=>
  string(%d) %s
  ["host"]=>
  string(%d) %s
  ["isoLanguage"]=>
  string(%d) %s
  ["kernelRel"]=>
  string(%d) %s
  ["language"]=>
  string(%d) %s
  ["partnerCodepage"]=>
  string(%d) %s
  ["partnerHost"]=>
  string(%d) %s
  ["partnerRel"]=>
  string(%d) %s
  ["partnerType"]=>
  string(%d) %s
  ["progName"]=>
  string(%d) %s
  ["rel"]=>
  string(%d) %s
  ["rfcRole"]=>
  string(%d) %s
  ["sysId"]=>
  string(%d) %s
  ["sysNumber"]=>
  string(%d) %s
  ["trace"]=>
  string(%d) %s
  ["type"]=>
  string(%d) %s
  ["user"]=>
  string(%d) %s
}