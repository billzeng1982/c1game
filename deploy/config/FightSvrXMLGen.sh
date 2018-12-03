#!/bin/bash

KeyValueA=
GenNum=0
File=
PortStart=7300
ShmKeyStart=13000
TbusServiceOrginFile=tbus_services_orgin.xml
TbusServiceFile=tbus_services.xml
function set_value()
{
	local Limit=${#KeyValueA[*]}
	for ((i=0,j=1; i<Limit; i=i+2,j=j+2))
	do
		#echo $j
		#echo "s/${KeyValueA[$i]}>.*</${KeyValueA[$i]}>${KeyValueA[$j]}</"
	    sed -i "s/${KeyValueA[$i]}>.*</${KeyValueA[$i]}>${KeyValueA[$j]}</" $File
	done
}
function gen_one_group_xml()
{
    local CurNum=$1
    local Port=$((PortStart + CurNum))
	local ShmKey=$((ShmKeyStart + CurNum))
    local SvrProcId="0.0.4.$CurNum"
    local ConnProcId="0.0.3.$CurNum"
    local FileSvrName="FightSvr$Port.xml"
    local FileConnName="FightConn$Port.xml"

    cat FightSvr.xml > $FileSvrName
    File=$FileSvrName
    KeyValueA=(
        ##Key			Value
        ProcIDStr  		$SvrProcId
        ConnIDStr       $ConnProcId
        ConnSvrPort     $Port
    )
    set_value
    

    cat FightConn.xml > $FileConnName
    File=$FileConnName
    KeyValueA=(
        ##Key			Value
        ProcIDStr  		$ConnProcId
        LogicSvrIDStr   $SvrProcId
        UdpBindPort     $Port
		ShmKey			$ShmKey
    )
    set_value
    echo "Generate SvrProcId:$SvrProcId  ConnProcId:$ConnProcId  Port:$Port "
}


function add_one_service()
{
    local TmpFile=$1
    local CurNum=$2
    local Port=$((PortStart + CurNum))
    local TbusGCIMEnd="</TbusGCIM>"
    local SvrProcId="0.0.4.$CurNum"
    local ConnProcId="0.0.3.$CurNum"
    
    #删除结尾头
    sed -i '/<\/TbusGCIM>/d' $TbusServiceFile
    cat  << EOF >> $TmpFile
	<Channels>
		<Priority>8</Priority>
		<Desc>fight connect $Port and fight server</Desc>
   	 	<Address>$ConnProcId</Address> 
		<Address>$SvrProcId</Address>
    	<SendSize>2097152</SendSize>
    	<RecvSize>2097152</RecvSize>
  	</Channels>
	
	<Channels>
		<Priority>8</Priority>
		<Desc>fight server $Port and cluster gate</Desc>
   	 	<Address>$SvrProcId</Address> 
		<Address>0.0.9.1</Address>
    	<SendSize>2097152</SendSize>
    	<RecvSize>2097152</RecvSize>
  	</Channels>
$TbusGCIMEnd
EOF
}

function add_to_tbus_service()
{
    local Date=`date "+%Y%m%d%H%M%S"`

    #备份
    mv $TbusServiceFile $TbusServiceFile.$Date.bak
    #echo "$TbusServiceFile  $TbusServiceOrginFile"
    cp $TbusServiceOrginFile $TbusServiceFile
    

    for ((k=1; k<=GenNum; k++))
    do
        add_one_service $TbusServiceFile $((k+1))
    done
    
}

if [ "$#" -lt "1" ];then
    GenNum=1
else 
    #检查是不是整数
    var=$(echo $1 | bc 2>/dev/null)
    if [ "$var" != "$1" ];then
        echo "Para is error"
        exit 1
    fi
    GenNum=$1
fi
echo $GenNum
for ((k=1; k<=GenNum; k++))
do
    gen_one_group_xml $((k+1))
done

add_to_tbus_service

echo "Generate all OK!!"
