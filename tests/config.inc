<?php
date_default_timezone_set('UTC');

/**
 * catch E_RECOVERABLE_ERROR errors and throw them as InvalidArgumentException
 */
set_error_handler(function($errno, $errstr) {
	throw new InvalidArgumentException($errstr, $errno);
}, E_RECOVERABLE_ERROR);

return [
	'dest' => 'EDS',
];