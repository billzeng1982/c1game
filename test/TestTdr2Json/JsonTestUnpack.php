<?php

require("./oss_proto.class.php");

$json = '{"Head":{"MsgID":50001,"Uin":0,"ReservID":111},"Body":{"GetUinReq":{"AccountName":"billzeng"}}}'; // ���������յ�������

// unpack
$OssPkg = new OSSPkg;
$OssPkg->unpack( json_decode($json) );

//var_dump( $OssPkg->Head );
var_dump( $OssPkg->Body );	

//var_dump( $OssPkg->Body['GetUinReq']->AccountName );

?>