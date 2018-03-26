<?php

class SapRfcReadTable extends SapFunction
{
	/**
	 * SapRfcReadTable is a special SapFunction class designed for the built in RFC_READ_TABLE function module.
	 *
	 * SapRfcReadTable::select() method simulates the
     * SQL "SELECT 'fields' FROM 'table' WHERE 'options' LIMIT 'limit' OFFSET 'offset'"
	 * to read data directly from a SAP database table (logon user must have access to read that table).
     *
	 * Param $fields could be a string (select one field only), or '*' (SQL STAR constant) for all table's fields,
	 * or an array containing the fields of the table that you are interested.
     * For every item in $fields array with a string key, an alias will be created for this item's value
     * (alias => fieldname) and the result array will export 'fieldname' as 'alias' like the
     * "SELECT fieldname AS alias ..." SQL syntax.
     *
	 * Every item in $options with a string key will be interpreted as 'FIELDNAME' => 'FIELDVALUE'
	 * and a corresponding "AND FIELDNAME EQ 'FIELDVALUE'" clause will be appended to function's OPTIONS table.
	 * If key is integer then the value is assumed as a custom 'WHERE' clause and appended as is.
	 * Eg. $options = array('FIELDNAME LIKE 'str%');
	 * All $options array values will be converted to strings. Be aware of quotes in $options array values,
     * they must be escaped.
	 * Each $options array value should be at most 72 characters long (as limited by the RFC_READ_TABLE function).
	 * Total length of requested fields should not exceed 512 characters (as limited by the RFC_READ_TABLE function).
	 *
	 * @param array|string $fields  Fields to select
	 * @param string $table         The table name to select rows from
	 * @param array $options        Select constraints
	 * @param int $limit            Limit the number of returned rows
	 * @param int $offset           SQL's offset
	 *
	 * @return string[][]   rows selected from $table e.g.
     * <pre>
     * array(
     *  array(
     *      'FIELD' => 'VALUE',
     *      ...
     *  ),
     *  array(
     *      'FIELD' => 'VALUE',
     *      ...
     *  )
     * )
     * </pre>
	 *
     * @throws TypeError                For invalid type of $fields parameter
     * @throws InvalidArgumentException If parameter $fields is an empty string
     * @throws InvalidArgumentException If parameter $fields is an array containing non-string or empty values
     * @throws InvalidArgumentException If parameter $table is an empty string
     * @throws InvalidArgumentException If parameter $limit is a negative number
     * @throws InvalidArgumentException If parameter $offset is a negative number
     * @throws LogicException           If no connection available, or function's description has not been fetched
     *                                  through a Sap object
	 * @throws SapException             If other error occurred
	 */
	public function select($fields, $table, array $options = [], $limit = 0, $offset = 0) {}

    /**
     * Retrieve structure metadata for a table (logon user must have access to read that table).
     *
     * @param string        $table  The table to retrieve metadata for
     * @param array|null    $fields Fields you are interested, or null for all fields
     *
     * @return array Table's metadata for the selected fields e.g.
     * <pre>
     * array(
     *  array(
     *      'FIELDNAME' => 'FIELD1',
     *      'OFFSET' => 1,
     *      'LENGTH' => 12,
     *      'TYPE'  => 'C'
     *      ...
     *  ),
     *  array(
     *      'FIELDNAME' => 'FIELD2',
     *      'OFFSET' => 2,
     *      'LENGTH' => 8,
     *      'TYPE'  => 'D'
     *      ...
     *  )
     * )
     * </pre>
     *
     * @throws InvalidArgumentException If parameter $table is an empty string
     * @throws InvalidArgumentException If parameter $fields is an array containing non-string or empty values
     * @throws LogicException           If no connection available, or function's description has not been fetched
     *                                  through a Sap object
     * @throws SapException             If other error occurred
     */
	public function describe($table, array $fields = null) {}
}