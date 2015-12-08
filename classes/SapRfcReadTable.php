<?php
class SapRfcReadTable extends SapFunction
{
	/**
	 * SapRfcReadTable is a special SapFunction class designed for the built in RFC_READ_TABLE function module.
	 *
	 * select() method simulates the SQL "SELECT 'fields' FROM 'table' WHERE 'options' LIMIT 'limit' OFFSET 'offset'"
	 * to read data directly from a SAP database table.
	 * Param $fields should be a string (select one field only), or '*' (SQL STAR constant) for all table's fields,
	 * or an array containing the fields of the table that you are interested. For every item in $fields array with
	 * a string key, an alias will be created for this item's value (alias => fieldname) and the result array will
	 * export 'fieldname' as 'alias' like the "SELECT fieldname AS alias ..." SQL syntax
	 * Every item in $options with a string key will be interpreted as 'FIELDNAME' => 'FIELDVALUE'
	 * and a corresponding "AND FIELDNAME EQ 'FIELDVALUE'" clause will be appended to function's OPTIONS table.
	 * If key is integer then the value is assumed as a custom 'WHERE' clause and is appended as is.
	 * Eg. $options = array('FIELDNAME LIKE 'str%');
	 * All $options array values will be converted to strings.
	 * Be aware of quotes in $options array values, they must be escaped.
	 * Each $options array value should be at most 72 characters long (as limited by the RFC_READ_TABLE function)
	 * Total length of requested fields should not exceed 512 characters (as limited by the RFC_READ_TABLE function)
	 *
	 * @param array|string $fields
	 * @param string $table
	 * @param array $options
	 * @param int $limit
	 * @param int $offset
	 * @return array
	 */
	public function select($fields, string $table, array $options = [], int $limit = 0, int $offset = 0) : array {}
}