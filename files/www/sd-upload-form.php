<?php
  $name="/tmp/succinctdataforms/".sha1($HTTP_RAW_POST_DATA).".xhtml";
  mkdir("/tmp/succinctdataforms");
  chmod("/tmp/succinctdataforms",0755);
  file_put_contents($name,file_get_contents("php://input"));
?>
