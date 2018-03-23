--TEST--
Verify ext loaded, default configuration, classes and functions
--FILE--
<?php
var_dump(extension_loaded('sap'));

var_dump(class_exists('Sap', false));
var_dump(class_exists('SapFunction', false));
var_dump(class_exists('SapException', false));
var_dump(class_exists('SapConnectionException', false));
var_dump(class_exists('SapRfcReadTable', false));

var_dump(function_exists('sap_connect'));
var_dump(function_exists('sap_invoke_function'));

var_dump(ini_get('sap.rtrim_export_strings'));
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
string(3) "Off"