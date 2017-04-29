<?php

  parse_str($_SERVER['QUERY_STRING'],$args);
  $text=$args['piece'];
  $text = preg_replace(",[/],", '_', $text);
  $text = preg_replace("/[^A-Za-z0-9+=:_\-\.]/", '', $text);
  unlink("/tmp/succinctdatarecords/record.".$text);

?>
