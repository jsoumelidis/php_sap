--TEST--
SapFunction::setActive() basic behavior
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

$f = (new Sap($config))->fetchFunction('STFC_STRUCTURE');

var_dump($f->isActive('IMPORTSTRUCT'));

$f->setActive('IMPORTSTRUCT', false); //do not transfer IMPORTSTRUCT
var_dump($f->isActive('IMPORTSTRUCT'));

$struct = [
	'RFCFLOAT' => 1.65,
	'RFCCHAR1' => 'X',
];

/* test IMPORTSTRUCT is not transfered, do not trim export strings */
$result = $f(['IMPORTSTRUCT' => $struct], false);

// ECHOSTRUCT should be INITIAL
var_dump($result['ECHOSTRUCT']['RFCFLOAT']);
var_dump($result['ECHOSTRUCT']['RFCCHAR1']);
// RFCTABLE should contain data
var_dump(count($result['RFCTABLE']));

$f->setActive('IMPORTSTRUCT' /*, $active = true*/); //transfer IMPORTSTRUCT
$f->setActive('RFCTABLE', false); //do not transfer RFCTABLE

/* test IMPORTSTRUCT is transfered, RFCTABLE is not exported */
$result = $f(['IMPORTSTRUCT' => $struct], false);

var_dump($result['ECHOSTRUCT']['RFCFLOAT']);
var_dump($result['ECHOSTRUCT']['RFCCHAR1']);
var_dump(isset($result['RFCTABLE']));
?>
--EXPECT--
bool(true)
bool(false)
float(0)
string(1) " "
int(1)
float(1.65)
string(1) "X"
bool(false)