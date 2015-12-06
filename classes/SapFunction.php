<?php
class SapFunction
{
	/**
	 * Get function's name
	 *
	 * @return string
	 */
	public function getName() : string {}

	/**
	 * Invokes the function module on the backend and returns it's export parameters.
	 * In order to use this functionality, the SapFunction object must be fetched
	 * through a Sap object connected to a R/3 system.
	 * The __invoke method makes the SapFunction object act as a 'callable' object, so
	 * $exports = $sapFunction->__invoke($imports) is the same as $exports = $sapFunction($imports).
	 *
	 * @param array $args import parameters
	 * @return array export parameters
	 * @throws SapException
	 */
	public function __invoke(array $imports = []) : array {}

	/**
	 * Set's a parameter active/inactive. If a parameter is set "inactive" it's data
	 * will not be transfered, neither on import nor on export.
	 * In case you want the default value applied for an import parameter (as defined in SE37)
	 * you should not set it's state or value at all, just leave it default.
	 * You could use this feature in case you are not interested
	 * in data of particular exporting tables for reducing network traffic, speed up
	 * execution etc.
	 *
	 * @param string $parameter
	 * @param bool $active
	 * @return bool
	 */
	public function setActive(string $parameter, bool $active) : bool {}
}