--TEST--
Edge
--SKIPIF--
<?php
/* $Id: gmagick-018-edgeimage.phpt 280206 2009-05-09 18:22:48Z vito $ */
if(!extension_loaded('gmagick')) die('skip');
?>
--FILE--
<?php
$gm = new Gmagick();
$gm->read("magick:rose");
$gm->edgeImage(0.8);
echo "ok";
?>
--EXPECTF--
ok