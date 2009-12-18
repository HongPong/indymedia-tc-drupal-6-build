<?php
// $Id: jumpcut.inc,v 1.1.2.8 2008/12/19 18:17:14 aaron Exp $

/**
 * @file
 *   This include processes jumpcut media files for use by emfield.module.
 */

define('EMVIDEO_JUMPCUT_MAIN_URL', 'http://jumpcut.com/');

function emvideo_jumpcut_info() {
  $features = array(
    array(t('Autoplay'), t('No'), ''),
    array(t('RSS Attachment'), t('No'), ''),
    array(t('Thumbnails'), t('No'), ''),
  );
  return array(
    'provider' => 'jumpcut',
    'name' => t('JumpCut'),
    'url' => EMVIDEO_JUMPCUT_MAIN_URL,
    'settings_description' => t('These settings specifically affect videos displayed from <a href="@provider" target="_blank">JumpCut</a>.', array('@provider' => EMVIDEO_JUMPCUT_MAIN_URL)),
    'supported_features' => $features,
  );
}

function emvideo_jumpcut_settings() {
  $form = array();
  return $form;
}

function emvideo_jumpcut_extract($embed) {
  return array(
    '@jumpcut\.com/view\?id=(.*)@i',
    '@jumpcut\.com/view\?id=([^\&]*)\&@i',
  );
}

function emvideo_jumpcut_video_link($video_code) {
  return 'http://jumpcut.com/view?id='. $video_code;
}

function theme_emvideo_jumpcut_flash($embed, $width, $height) {
  if ($embed) {
    $output .= '<embed src="http://jumpcut.com/media/flash/jump.swf?id='. $embed .'&amp;asset_type=movie&amp;asset_id='. $embed .'&eb=1" width="'. $width .'" height="'. $height .'" type="application/x-shockwave-flash"></embed>';
  }
  return $output;
}

function emvideo_jumpcut_thumbnail($field, $item, $formatter, $node, $width, $height) {
  return '';
}

function emvideo_jumpcut_video($embed, $width, $height, $field, $item, &$node) {
  $output = theme('emvideo_jumpcut_flash', $embed, $width, $height);
  return $output;
}

function emvideo_jumpcut_preview($embed, $width, $height, $field, $item, &$node) {
  $output = theme('emvideo_jumpcut_flash', $embed, $width, $height);
  return $output;
}

/**
 * Implementation of hook_emfield_subtheme.
 */
function emvideo_jumpcut_emfield_subtheme() {
  return array(
    'emvideo_jumpcut_flash' => array(
      'arguments' => array('embed' => NULL, 'width' => NULL, 'height' => NULL),
      'file' => 'providers/jumpcut.inc'
    )
  );
}
