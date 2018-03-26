<?php

class Sap
{
	/**
     * Sap constructor
     * If $logonParameters provided {@see Sap::connect} is called
     *
	 * @param array $logonParameters    System logon parameters
	 */
	public function __construct(array $logonParameters = null) {}

    /**
     * For documentation regarding logon parameters see the demo sapnwrfc.ini
     * provided by your SAP Netweaver RFC SDK (available from SAP Marketplace)
     *
     * All logon parameters/values should be strings, otherwise they will
     * be internally converted to strings
     *
     * @param array $logonParameters    System logon parameters
     *
     * @return void
     *
     * @throws InvalidArgumentException If empty array provided
     * @throws SapConnectionException   For invalid logon parameters or other error occured
     */
    public function connect(array $logonParameters) {}
    
	/**
	 * Fetches a function module's description from the R/3 backend
	 * and returns a ready to call SapFunction object.
	 
	 * If argument #1 is a SapFunction object, function's description will be
	 * fetched into that object. Previous function description stored will be
	 * destroyed. Object must implement the getName() method which should return
	 * the corresponding Remote Function name as string.
	 * 
	 * If argument #2 is provided, fetchFunction() will return a new object of $class
	 * using $ctor_args as constructor arguments (if those provided).
	 *
	 * @param string|SapFunction    $function
	 * @param string                $class      Class derived from SapFunction
	 * @param array                 $ctor_args  Constructor arguments, used in conjunction with $class argument
	 *
	 * @return SapFunction
	 *
     * @throws LogicException   If connection to a SAP system is not available
	 * @throws SapException     If function not found or other error occured
	 */
	public function fetchFunction($function, string $class = null, array $ctor_args = null) : SapFunction {}

	/**
	 * Instant call of a Remote Function Module.
	 *
	 * @param string        $function   Function's name
	 * @param array|null    $args       import/changing/tables parameters
	 * @param bool|null     $rtrim      Right-trim string export fields. If not provided, the global .ini setting will be used
	 *
	 * @return array Function's export/changing/tables parameters
	 * 
     * @throws LogicException   If connection to a SAP system is not available
	 * @throws SapException     If function not found or other error occured
	 */
	public function call(string $function, array $args = null, bool $rtrim = null) : array {}
	
	/**
	 * Sets the default class, for this connection, when fetching remote functions.
	 *
	 * @param string $class A class name derived from SapFunction
     *
     * @return void
	 */
	public function setFunctionClass(string $class) {}

    /**
     * @return string The default class this connection uses for fetching Remote Function (SapFunction) objects
     */
	public function getFunctionClass(): string {}

	/**
	 * Get connection attributes
     *
     * @return array
	 */
	public function getAttributes(): array {}
}