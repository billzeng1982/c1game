$(document).ready(function(){
      id=$("#Uin").attr("name")
      ip=$("#SvrIP").attr("name")
      svrid=$("#SvrId").attr("name")

     /* 页面显示*/
        posturl="http://"+ip+":8002/GetRoleWebTaskInfo"
        jsondata={TaskType:20,Uin: id,SvrId:svrid}
        Jdata=JSON.stringify(jsondata)
    $.ajax({
        type:'POST',
        url:posturl,
        data:Jdata,
        success:function(data){

            webdata=JSON.parse(data)
            if (webdata['Errno']!=0){
                alert("您已经掉线或者数据错误")
            }
            else{
                try{
                 ButtonS()/*显示领取按钮*/
                cumProbar()/* 执行进度条*/
            puzzle()/*执行拼图*/}
                catch(err){
                    alert("任务数据错误")
                    ButtonErr()

                }
            }




        },
         error:function(){alert("获取数据错误")}
    })



    /* 领取物品*/
  $("#mission1buttonYes,#mission2buttonYes,#mission3buttonYes,#mission4buttonYes,#mission5buttonYes,#mission6buttonYes,#PuzzleButtonYes").click(function(){
        TaskIDs=$(this).attr("name");
        TaskID=parseInt(TaskIDs)

        var data;

      data="TaskId="+TaskID+"&Uin="+id+"&SvrId="+svrid
      url="http://"+ip+":8002/GetWebTaskRwd"

     $.ajax({
        type: "get",
        url:url,
        async:false, // 不支持同步，同步无效
        data: data,
        success:function(Errno){
            Err=JSON.parse(Errno)
           if(Err['Errno']==0){
            alert("领取成功");

            }
            else{alert(Errno)}

        },
         error:function(){alert("网络错误")}
    });
    });







});

/*显示领取按钮*/
function ButtonS(){
    for(i=0;i<6;i++){
                buttonYes="#mission"+(i+1)+"buttonYes"
                buttonNo1="#mission"+(i+1)+"buttonNo1"
                buttonNo2="#mission"+(i+1)+"buttonNo2"
                missionDescPNum="missionDescP:eq("+i+")"
                missionPtext=$(missionDescPNum).text()

                    if (webdata['TaskInfo'][i]['IsDrawed']==1){
                        $(buttonYes).show()
                         $(buttonNo1).hide()
                         $(buttonNo2).hide()
                    }
                    else {
                        if(webdata['TaskInfo'][i]['Progress']<missionPtext){
                         $(buttonYes).hide()
                         $(buttonNo1).show()
                         $(buttonNo2).hide()
                        }
                        else{
                        $(buttonYes).hide()
                         $(buttonNo1).hide()
                         $(buttonNo2).show()
                        }

                    }
                    /*显示进度*/
                 missionPtext=webdata['TaskInfo'][i]['Progress'].toString()+"/"+missionPtext
                 $(missionDescPNum).text(missionPtext)

                }

}

/*处理按钮报错*/
function ButtonErr(){
     for(i=0;i<6;i++) {
         buttonYes = "#mission" + (i + 1) + "buttonYes"
         buttonNo1 = "#mission" + (i + 1) + "buttonNo1"
         buttonNo2 = "#mission" + (i + 1) + "buttonNo2"
         $(buttonYes).hide()
         $(buttonNo1).hide()
         $(buttonNo2).show()
     }

     $("#PuzzleButtonYes").hide()
        $("#PuzzleButtonNo1").hide()
        $("#PuzzleButtonNo2").show()
    $(".cumProbar").width(0)
}



         /* 拼图解锁显示*/
function puzzle() {
    puzzleMissonNum=$("#puzzleMisson").attr("name")

    var puzzle = webdata['TaskInfo'][6]['Progress']
    if(webdata['TaskInfo'][6]['IsDrawed']==1){
        $("#PuzzleButtonYes").show()
        $("#PuzzleButtonNo1").hide()
        $("#PuzzleButtonNo2").hide()
    }
    else{
        if(puzzle<puzzleMissonNum){
        $("#PuzzleButtonYes").hide()
        $("#PuzzleButtonNo1").show()
        $("#PuzzleButtonNo2").hide()
        }
        else{
        $("#PuzzleButtonYes").hide()
        $("#PuzzleButtonNo1").hide()
        $("#PuzzleButtonNo2").show()
        }
    }


    puzzleMissonNum = puzzleMissonNum - puzzle
    if (puzzleMissonNum == 0) {
        $("#puzzleMisson").text("已解锁皮肤！")

    }
    else {
        $("#puzzleMisson").text("还有" + puzzleMissonNum + "片拼图解锁皮肤")
        /* 拼图数量显示*/
    }
    for (var i = 0; i < puzzle; i++) {
        var puzzleSuccess = "td:eq(" + i + ")";
        $(puzzleSuccess).css("background", "rgba(0,0,0, 0)")
    }
    ;
}


  /* 进度条显示*/
function cumProbar() {
    var Progress = $("missionDescP:eq(5)").text()
    Progressdata = Progress.split("/")
    WindowWidth = $(window).width();
      var progress = (Progressdata[0] / Progressdata[1]);

    $(".cumProbar1").width(function (n, WindowWidth) {

        if (progress <= (1 / 3)) {
            p = 0.7 * (1 / 6) * (Progressdata[0] / 10)
        }
        else {
            p = 0.7 * (1 / 6)
        }
        return WindowWidth * p;
    });
     $(".cumProbar2").width(function (n, WindowWidth) {

        if (progress <= (1 / 3)) {
            p =0
        }
        else if(progress>(1/3)&&progress<=(2/3)){
            p=((Progressdata[0] / 10)-1)*0.7*(1/3)*1.47
        }
        else {
            p =0.7*(1/3)*1.47

        }
        return WindowWidth * p;
    });
    $(".cumProbar3").width(function (n, WindowWidth) {

        if (progress <= (2 / 3)) {
            p =0
        }
        else if(progress>(2/3)&&progress<1){
             p=((Progressdata[0]-20)/10)*0.7*0.85
        }
        else {
             p =0.7*0.85

        }
        return WindowWidth * p;
    });
      $(".cumProbar4").width(function (n, WindowWidth) {

        if (progress <= 1){
            p =0
        }
        else {
            p =((Progressdata[0]-30)/6)*0.7

        }
        return WindowWidth * p;
    });

}

/*重新弹窗*/
window.alert = function(str) {
    var alertBox = document.createElement("div");
    alertBox.id="alertBox";
    alertBox.style.position = "fixed";
    alertBox.style.width = "30%";
    alertBox.style.background = "#ffffff";
    alertBox.style.border = "1px solid grey";
    alertBox.style.left = "50%";
    alertBox.style.top = "50%";
    alertBox.style.transform = "translate(-50%, -50%)";
    alertBox.style.textAlign = "center";
    alertBox.style.zIndex = "999999";
    var strHtml = "";
    strHtml += '<div id="title"><div id="close" onclick="certainFunc()"></div></div>';
    strHtml += '<div id="content">'+str+'</div>';
    strHtml += '<div id="certain"><input id="btn" type="button" value="确 定" onclick="certainFunc()" onhover="hoverFunc()"/></div>';
    alertBox.innerHTML = strHtml;
    document.body.appendChild(alertBox);
    var title = document.getElementById("title");
    title.style.textAlign = "left";
   title.style.marginTop = "3vh";
    title.style.paddingLeft = "3vh";
    title.style.height = "3vh";
    title.style.fontSize = "1.5vmax";
    var close = document.getElementById("close");
    close.style.width = "3vh";
    close.style.marginRight = "3vh";
    close.style.float = "right";
    var content = document.getElementById("content");
    content.style.margin = "3vh";
    content.style.fontSize = "1.5vmax";
    var certain = document.getElementById("certain");
    certain.style.position = "relative";
    certain.style.height = "10vh";
    certainFunc = function() {
        alertBox.parentNode.removeChild(alertBox);
        window.location.reload();
    };
     var btn = document.getElementById("btn");
    btn.style.width = "30%";
    btn.style.height = "8vh";
    btn.style.background = "#cccccc";
    btn.style.border = "1px solid grey";
    btn.style.position = "absolute";
    btn.style.borderRadius = "5px";
    btn.style.right = "1vmax";
    btn.style.bottom = "1vmax";
    btn.style.marginTop = "1vmax";
    btn.style.cursor = "pointer";
    btn.style.color = "#333";
    hoverFunc = function() {
        btn.style.border = "1px blue solid";
    };
}