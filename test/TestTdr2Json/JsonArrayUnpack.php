<?php

require("./oss_proto.class.php");

$json =  '{"Head":{"MsgID":50003,"Uin":0,"ReservID":111},"Body":{"ArrayTest":{"count":5,"ArrTest":[{"Uin":10001,"Age":10},{"Uin":10002,"Age":11},{"Uin":10003,"Age":12},{"Uin":10004,"Age":13},{"Uin":10005,"Age":14}]}}}';

//var_dump( json_decode( $json ) );

// unpack
$OssPkg = new OSSPkg;
$OssPkg->unpack( json_decode($json) );

//var_dump( $OssPkg->Head );
var_dump( $OssPkg->Body );	

?>
