<?php
//echo ("update_db.php indicator");

function mysql_insert($table, $inserts) {
echo $table;
    $values = array_map('mysql_real_escape_string', array_values($inserts));
    $keys = array_keys($inserts);
      
    return mysql_query('INSERT INTO `'.$table.'` (`'.implode('`,`', $keys).'`) VALUES (\''.implode('\',\'', $values).'\')');

}




//Connect to database
$con = mysql_connect('localhost', 'username', 'password');
if(!$con)
	{
	die('Could not connect: ' .mysql_error());
	}
$db = mysql_select_db('database', $con);
echo "inne";

if (isset($_GET['T1'])){
	mysql_insert('Temp', array(
    	'Time' => $_GET['Time'],
    	't1' => $_GET['T1']/100,
    	't2' => $_GET['T2']/100,
    	't3' => $_GET['T3']/100,
    	't4' => $_GET['T4']/100
   	));
}
if (isset($_GET['H1'])){
	mysql_insert('Humidity', array(
	    'Time' => $_GET['Time'],
	    'h1' => $_GET['H1']/100,
	    'h2' => $_GET['H2']/100,
	    'h3' => $_GET['H3']/100,
	    'h4' => $_GET['H4']/100
	    ));
}

if (isset($_GET['E'])){
	mysql_insert('Energy', array(
	    'Time' => $_GET['Time'],
	    'E' => $_GET['E']
	    ));
}
mysql_close($con);
?> 
