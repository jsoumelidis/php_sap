--TEST--
RFC RAISE exception throws SapException
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

$rfcReadTable = (new Sap($config))->fetchFunction(new SapRfcReadTable);

/* should RAISE TABLE_NOT_AVAILABLE */
try { $rfcReadTable->select(null, 'NON_EXISTENT_TABLE'); }
catch (SapException $e) {
	var_dump($e->getMessageKey());
	var_dump($e->getMessageType());
	var_dump($e->getMessageId());
	var_dump($e->getMessageNumber());
	var_dump($e->getMessageVar1());
	var_dump($e->getMessageVar2());
	var_dump($e->getMessageVar3());
	var_dump($e->getMessageVar4());
	var_dump($e->getNwSdkFunction());
}

?>
--EXPECTF--
string(19) "TABLE_NOT_AVAILABLE"
string(1) "%c"
string(%d) "%s"
string(%d) "%s"
string(18) "NON_EXISTENT_TABLE"
string(0) ""
string(0) ""
string(0) ""
string(9) "RfcInvoke"