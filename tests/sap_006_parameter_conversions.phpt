--TEST--
import parameter conversions
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

try { $c = sap_connect($config); }
catch (Throwable $e) {
	echo 'Could not connect: ', $e->getMessage(), PHP_EOL;
	exit(0);
}

class SomeClass { public function __toString() { return '!'; } }

$struct = [
	'RFCFLOAT' => '1.056725e2',
	'RFCCHAR1' => new SomeClass,
	'RFCINT2' => '2568',
	'RFCINT1' => '23',
	'RFCCHAR4' => 1.256784, //value must be truncated to 4 chars
	'RFCINT4' => 5896441,
	'RFCHEX3' => 'aaa',
	'RFCCHAR2' => 55,
	'RFCTIME' => new DateTime('2018-01-01 12:03:01'),
	'RFCDATE' => new DateTime('2018-02-04'),
	'RFCDATA1' => 'Lorem ipsum dolor',
	'RFCDATA2' => 'ΑλφαΒήταΓάμαΔέλτα',
];

$result = sap_invoke_function('STFC_STRUCTURE', $c, ['IMPORTSTRUCT' => $struct, 'RFCTABLE' => [$struct]], true);

/* test export structure */
echo 'RFCFLOAT ';	var_dump($result['ECHOSTRUCT']['RFCFLOAT']);
echo 'RFCCHAR1 ';	var_dump($result['ECHOSTRUCT']['RFCCHAR1']);
echo 'RFCINT2 ' ;	var_dump($result['ECHOSTRUCT']['RFCINT2']);
echo 'RFCINT1 ' ;	var_dump($result['ECHOSTRUCT']['RFCINT1']);
echo 'RFCCHAR4 ';	var_dump($result['ECHOSTRUCT']['RFCCHAR4']);
echo 'RFCHEX3 ' ;	var_dump($result['ECHOSTRUCT']['RFCHEX3']);
echo 'RFCCHAR2 ';	var_dump($result['ECHOSTRUCT']['RFCCHAR2']);
echo 'RFCTIME ' ;	var_dump($result['ECHOSTRUCT']['RFCTIME']);
echo 'RFCDATE ' ;	var_dump($result['ECHOSTRUCT']['RFCDATE']);
echo 'RFCDATA1 ';	var_dump($result['ECHOSTRUCT']['RFCDATA1']);
echo 'RFCDATA2 ';	var_dump($result['ECHOSTRUCT']['RFCDATA2']);

/* test export table */
var_dump(count($result['RFCTABLE']));

echo 'RFCFLOAT ';	var_dump($result['RFCTABLE'][0]['RFCFLOAT']);
echo 'RFCCHAR1 ';	var_dump($result['RFCTABLE'][0]['RFCCHAR1']);
echo 'RFCINT2 ' ;	var_dump($result['RFCTABLE'][0]['RFCINT2']);
echo 'RFCINT1 ' ;	var_dump($result['RFCTABLE'][0]['RFCINT1']);
echo 'RFCCHAR4 ';	var_dump($result['RFCTABLE'][0]['RFCCHAR4']);
echo 'RFCHEX3 ' ;	var_dump($result['RFCTABLE'][0]['RFCHEX3']);
echo 'RFCCHAR2 ';	var_dump($result['RFCTABLE'][0]['RFCCHAR2']);
echo 'RFCTIME ' ;	var_dump($result['RFCTABLE'][0]['RFCTIME']);
echo 'RFCDATE ' ;	var_dump($result['RFCTABLE'][0]['RFCDATE']);
echo 'RFCDATA1 ';	var_dump($result['RFCTABLE'][0]['RFCDATA1']);
echo 'RFCDATA2 ';	var_dump($result['RFCTABLE'][0]['RFCDATA2']);
?>
--EXPECT--
RFCFLOAT float(105.6725)
RFCCHAR1 string(1) "!"
RFCINT2 int(2568)
RFCINT1 int(23)
RFCCHAR4 string(4) "1.25"
RFCHEX3 string(3) "aaa"
RFCCHAR2 string(2) "55"
RFCTIME string(6) "120301"
RFCDATE string(8) "20180204"
RFCDATA1 string(17) "Lorem ipsum dolor"
RFCDATA2 string(34) "ΑλφαΒήταΓάμαΔέλτα"
int(2)
RFCFLOAT float(105.6725)
RFCCHAR1 string(1) "!"
RFCINT2 int(2568)
RFCINT1 int(23)
RFCCHAR4 string(4) "1.25"
RFCHEX3 string(3) "aaa"
RFCCHAR2 string(2) "55"
RFCTIME string(6) "120301"
RFCDATE string(8) "20180204"
RFCDATA1 string(17) "Lorem ipsum dolor"
RFCDATA2 string(34) "ΑλφαΒήταΓάμαΔέλτα"