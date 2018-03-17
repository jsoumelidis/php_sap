--TEST--
Invoke STFC_STRUCTURE
--FILE--
<?php
$logon = include 'config.inc';

$struct = [
	'RFCFLOAT' => 1.056725,
	'RFCCHAR1' => '!',
	'RFCINT2' => 2568,
	'RFCINT1' => 23,
	'RFCCHAR4' => 'abcd',
	'RFCINT4' => 5896441,
	'RFCHEX3' => 'abc',
	'RFCCHAR2' => 'yo',
	'RFCTIME' => new DateTime(),
	'RFCDATE' => new DateTime(),
	'RFCDATA1' => 'Lorem ipsum dolor',
	'RFCDATA2' => 'ΑλφαΒήταΓάμαΔέλτα',
];

$sap = new Sap($logon);

$result = $sap->call('STFC_STRUCTURE', [
	'IMPORTSTRUCT' => $struct,
	'RFCTABLE' => [$struct],
]);

echo $result['ECHOSTRUCT']['RFCFLOAT']	=== $struct['RFCFLOAT'] ? 'ok' : 'not ok', PHP_EOL;
echo $result['ECHOSTRUCT']['RFCCHAR1']	=== $struct['RFCCHAR1'] ? 'ok' : 'not ok', PHP_EOL;
echo $result['ECHOSTRUCT']['RFCINT2']	=== $struct['RFCINT2'] ? 'ok' : 'not ok', PHP_EOL;
echo $result['ECHOSTRUCT']['RFCINT1']	=== $struct['RFCINT1'] ? 'ok' : 'not ok', PHP_EOL;
echo $result['ECHOSTRUCT']['RFCCHAR4']	=== $struct['RFCCHAR4'] ? 'ok' : 'not ok', PHP_EOL;
echo $result['ECHOSTRUCT']['RFCINT4']	=== $struct['RFCINT4'] ? 'ok' : 'not ok', PHP_EOL;
echo $result['ECHOSTRUCT']['RFCHEX3']	=== $struct['RFCHEX3'] ? 'ok' : 'not ok', PHP_EOL;
echo $result['ECHOSTRUCT']['RFCCHAR2']	=== $struct['RFCCHAR2'] ? 'ok' : 'not ok', PHP_EOL;
echo $result['ECHOSTRUCT']['RFCTIME']	=== $struct['RFCTIME']->format('His') ? 'ok' : 'not ok', PHP_EOL;
echo $result['ECHOSTRUCT']['RFCDATE']	=== $struct['RFCDATE']->format('Ymd') ? 'ok' : 'not ok', PHP_EOL;
echo $result['ECHOSTRUCT']['RFCDATA1']	=== $struct['RFCDATA1'] ? 'ok' : 'not ok', PHP_EOL;
echo $result['ECHOSTRUCT']['RFCDATA2']	=== $struct['RFCDATA2'] ? 'ok' : 'not ok', PHP_EOL;

?>
--EXPECT--
ok
ok
ok
ok
ok
ok
ok
ok
ok
ok
ok
ok
