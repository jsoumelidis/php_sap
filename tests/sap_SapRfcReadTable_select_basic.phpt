--TEST--
SapRfcReadTable::select() basic behavior
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

/* select all fields of the first 3 system languages */
$t002 = $rfcReadTable->select('*', 'T002', [], 3, 0, false);
var_dump($t002);

/* select SPRAS, LASPEZ, LAISO aliasing field LASPEZ to SPECIFICATION & LAISO to ISO of the first 3 system languages */
$t002 = $rfcReadTable->select(['SPRAS', 'LASPEZ' => 'SPECIFICATION', 'LAISO' => 'ISO'], 'T002', [], 3, 0, false);
var_dump($t002);

/* select ISO format of the system languages with SPRAS = 'D' */
$t002 = $rfcReadTable->select('LAISO', 'T002', ['SPRAS' => 'D'], 1, 0, false);
var_dump($t002);
?>
--EXPECTF--
array(3) {
  [0]=>
  array(4) {
    ["SPRAS"]=>
    string(1) "%c"
    ["LASPEZ"]=>
    string(1) "%c"
    ["LAHQ"]=>
    string(1) "%c"
    ["LAISO"]=>
    string(2) "%s"
  }
  [1]=>
  array(4) {
    ["SPRAS"]=>
    string(1) "%c"
    ["LASPEZ"]=>
    string(1) "%c"
    ["LAHQ"]=>
    string(1) "%c"
    ["LAISO"]=>
    string(2) "%s"
  }
  [2]=>
  array(4) {
    ["SPRAS"]=>
    string(1) "%c"
    ["LASPEZ"]=>
    string(1) "%c"
    ["LAHQ"]=>
    string(1) "%c"
    ["LAISO"]=>
    string(2) "%s"
  }
}
array(3) {
  [0]=>
  array(3) {
    ["SPRAS"]=>
    string(1) "%c"
    ["SPECIFICATION"]=>
    string(1) "%c"
    ["ISO"]=>
    string(2) "%s"
  }
  [1]=>
  array(3) {
    ["SPRAS"]=>
    string(1) "%c"
    ["SPECIFICATION"]=>
    string(1) "%c"
    ["ISO"]=>
    string(2) "%s"
  }
  [2]=>
  array(3) {
    ["SPRAS"]=>
    string(1) "%c"
    ["SPECIFICATION"]=>
    string(1) "%c"
    ["ISO"]=>
    string(2) "%s"
  }
}
array(1) {
  [0]=>
  array(1) {
    ["LAISO"]=>
    string(2) "DE"
  }
}