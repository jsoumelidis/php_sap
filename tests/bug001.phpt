--TEST--
SapRfcReadTable::select() with 2 where conditions does not prepend "AND" for the second
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

/* select with 2 conditions */
$t002 = $rfcReadTable->select('LAISO', 'T002', ['SPRAS' => 'D', 'LAISO' => 'DE'], 1, 0, false);
var_dump($t002);
?>
--EXPECT--
array(1) {
  [0]=>
  array(1) {
    ["LAISO"]=>
    string(2) "DE"
  }
}