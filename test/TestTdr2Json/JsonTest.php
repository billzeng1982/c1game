<?php

require("./oss_proto.class.php");

// test decode (unpack)
$json = '{"Head":{"MsgID":50001,"Uin":0,"ReservID":111},"Body":{"GetUinReq":{"AccountName":"billzeng"}}}'; // 从网络上收到的数据
$OSSPkgDecode = json_decode($json);
//var_dump( $OSSPkgDecode );

$Head = $OSSPkgDecode->{ 'Head' };
$Body = $OSSPkgDecode->{ 'Body' };
$GetUinReq = $Body->{ 'GetUinReq' };
//var_dump($GetUinReq);
$AccountName = $GetUinReq->{ 'AccountName' } ;
//var_dump($AccountName);

// test encode
$OssPkgEncode = new OSSPkg();
$OssPkgEncode->Head = new OSSPkgHead();
//$OssPkgEncode->Body = new OSSPkgBody();

// construct head (pack)
$OssPkgEncode->Head->MsgID = 50001;
$OssPkgEncode->Head->Uin = 0;
$OssPkgEncode->Head->ReservID = 111;
// construct body
$OssPkgEncode->Body['GetUinReq'] = new OSS_PKG_GET_UIN_REQ();
$OssPkgEncode->Body['GetUinReq']->AccountName = "billzeng";

$json= json_encode($OssPkgEncode);
var_dump($json);
?>