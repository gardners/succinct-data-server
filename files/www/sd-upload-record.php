<?php

// Get important fields
          $text = file_get_contents("php://input"); 
          $text = preg_replace(",[/],", '_', $text);
          $text = preg_replace("/[^A-Za-z0-9+=:_]/", '', $text);
          $prefix = substr($text,0,10);
          $sender = "sd-upload-record.php";
          $dest_dir = "/tmp/succinctdatarecords";
          $name="$dest_dir/record.${sender}.${prefix}";

          // Make sure prefix is at least 8 bytes
          if ( strlen($prefix) == 10 ) {
            echo "$name \n";
            echo "message = $text\n";

            // Write message
            if (!file_exists($dest_dir)) mkdir($dest_dir);
            chmod($dest_dir,0755);
            if (file_exists($name)) unlink($name);
            file_put_contents($name,$text);
            echo($name);

            // And persistent copy for recovering from errors
            $dest_dir="/tmp/succinctdatalog";
            $name="$dest_dir/record.${sender}.${prefix}";
            if (!file_exists($dest_dir)) mkdir($dest_dir);
            chmod($dest_dir,0755);
            if (file_exists($name)) unlink($name);
            file_put_contents($name,$text);

          }  else file_put_contents("/tmp/dud",$text);

?>
