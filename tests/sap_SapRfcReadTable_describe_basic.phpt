--TEST--
SapRfcReadTable::describe() basic behavior
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

$fields = $rfcReadTable->describe('T002');
var_dump($fields);

$fields = $rfcReadTable->describe('T002', ['LAISO', 'LASPEZ']);
var_dump($fields);
?>
--EXPECTF--
array(4) {
  [0]=>
  array(5) {
    ["FIELDNAME"]=>
    string(5) "SPRAS"
    ["OFFSET"]=>
    int(0)
    ["LENGTH"]=>
    int(1)
    ["TYPE"]=>
    string(1) "C"
    ["FIELDTEXT"]=>
    string(%d) "%s"
  }
  [1]=>
  array(5) {
    ["FIELDNAME"]=>
    string(6) "LASPEZ"
    ["OFFSET"]=>
    int(1)
    ["LENGTH"]=>
    int(1)
    ["TYPE"]=>
    string(1) "C"
    ["FIELDTEXT"]=>
    string(%d) "%s"
  }
  [2]=>
  array(5) {
    ["FIELDNAME"]=>
    string(4) "LAHQ"
    ["OFFSET"]=>
    int(2)
    ["LENGTH"]=>
    int(1)
    ["TYPE"]=>
    string(1) "C"
    ["FIELDTEXT"]=>
    string(%d) "%s"
  }
  [3]=>
  array(5) {
    ["FIELDNAME"]=>
    string(5) "LAISO"
    ["OFFSET"]=>
    int(3)
    ["LENGTH"]=>
    int(2)
    ["TYPE"]=>
    string(1) "C"
    ["FIELDTEXT"]=>
    string(%d) "%s"
  }
}
array(2) {
  [0]=>
  array(5) {
    ["FIELDNAME"]=>
    string(5) "LAISO"
    ["OFFSET"]=>
    int(0)
    ["LENGTH"]=>
    int(2)
    ["TYPE"]=>
    string(1) "C"
    ["FIELDTEXT"]=>
    string(%d) "%s"
  }
  [1]=>
  array(5) {
    ["FIELDNAME"]=>
    string(6) "LASPEZ"
    ["OFFSET"]=>
    int(2)
    ["LENGTH"]=>
    int(1)
    ["TYPE"]=>
    string(1) "C"
    ["FIELDTEXT"]=>
    string(%d) "%s"
  }
}