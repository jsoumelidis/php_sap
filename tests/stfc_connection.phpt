--TEST--
Invoke STFC_CONNECTION
--FILE--
<?php
$logon = include 'config.inc';

$echotext = 'T€st invoke';

$sap = new Sap($logon);

$result = $sap->call('STFC_CONNECTION', [
	'REQUTEXT' => $echotext,
]);

echo isset($result['ECHOTEXT']) ? 'echotext set' : 'echotext not set', PHP_EOL;
echo $result['ECHOTEXT'] === $echotext ? 'echotext ok' : "echotext not ok ({$result['ECHOTEXT']})", PHP_EOL;

?>
--EXPECT--
echotext set
echotext ok