--TEST--
Verify ext loaded and classes
--FILE--
<?php
echo extension_loaded('sap') ? 'yes' : 'no', PHP_EOL;
echo class_exists('Sap', false) ? 'yes' : 'no', PHP_EOL;
echo class_exists('SapFunction', false) ? 'yes' : 'no', PHP_EOL;
echo class_exists('SapException', false) ? 'yes' : 'no', PHP_EOL;
echo class_exists('SapRfcReadTable', false) ? 'yes' : 'no', PHP_EOL;
?>
--EXPECT--
yes
yes
yes
yes
yes