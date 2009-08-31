<?php
// $Id: veoh.inc,v 1.1.2.8 2009/07/23 18:55:41 robloach Exp $

/**
 * @file
 *   Provide support for the Veoh provider to the emfield.module.
 */

define('EMVIDEO_VEOH_MAIN_URL', 'http://www.veoh.com/');
define('EMVIDEO_VEOH_API_INFO', 'http://www.veoh.com/rest/v2/doc.html');
define('EMVIDEO_VEOH_XML', 'http://www.veoh.com/rest/video/');

function emvideo_veoh_info() {
  $features = array(
    array(t('Autoplay'), t('Yes'), ''),
    array(t('RSS Attachment'), t('No'), ''),
    array(t('Show related videos'), t('No'), t('This is embedded in the video itself when enabled; currently not available with other providers. Set the feature above.')),
    array(t('Thumbnails'), t('Yes'), t('May not currently resize thumbnails. Must have an API key for thumbnails at the moment, although research is underway to determine an alternative to this. Set your API key above.')),
  );
  return array(
    'provider' => 'veoh',
    'name' => t('Veoh'),
    'url' => EMVIDEO_VEOH_MAIN_URL,
    'settings_description' => t('These settings specifically affect videos displayed from <a href="@veoh" target="_blank">Veoh</a>. You can learn more about its <a href="@api" target="_blank">API</a> here.', array('@veoh' => EMVIDEO_VEOH_MAIN_URL, '@api' => EMVIDEO_VEOH_API_INFO)),
    'supported_features' => $features,
  );
}

function emvideo_veoh_settings() {
  $form['veoh']['api'] = array(
    '#type' => 'fieldset',
    '#title' => t('Veoh API'),
    '#description' => t('If you wish to be able to display Veoh thumbnails automatically, you will first need to apply for an API Developer Key from the <a href="@veoh" target="_blank">Veoh Developer Profile page</a>. Note that you do not need this key to display Veoh videos themselves.', array('@veoh' => EMVIDEO_VEOH_API_INFO)),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['veoh']['api']['emvideo_veoh_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Veoh API Key'),
    '#default_value' => variable_get('emvideo_veoh_api_key', ''),
    '#description' => t('Please enter your Veoh Developer Key here.'),
  );
  $form['veoh']['api']['emvideo_veoh_api_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Veoh API Secret'),
    '#default_value' => variable_get('emvideo_veoh_api_secret', ''),
    '#description' => t('If you have a secret for the Veoh API, enter it here.'),
  );
  return $form;
}


function emvideo_veoh_request($embed, $cacheable = TRUE) {
  $args = array('docid' => $embed);
  return module_invoke('emfield', 'request_xml', 'veoh', EMVIDEO_VEOH_XML . $embed .'/details', $args, $cacheable);
}

function emvideo_veoh_extract($embed = '') {
  return array(
    '@http://www\.veoh\.com/videos/([^"\?]+)@i',
    '@http://www\.veoh\.com/videodetails2\.swf\?permalinkId=([^"\&]*)@i',
  );
}

function emvideo_veoh_embedded_link($video_code) {
  return 'http://www.veoh.com/videos/'. $video_code;
}

function theme_emvideo_veoh_flash($embed, $width, $height, $autoplay) {
  if ($embed) {
    $autoplay = $autoplay ? '&amp;videoAutoPlay=1' : '&amp;videoAutoPlay=0';

    $output .= '<embed src="http://www.veoh.com/videodetails2.swf?permalinkId='. $embed .'&amp;id=9183600&amp;player=videodetailsembedded'. $autoplay .'" allowFullScreen="true" width="'. $width .'" height="'. $height .'" bgcolor="#FFFFFF" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer"></embed>';
  }
  return $output;
}

function emvideo_veoh_check_thumbnail($thumbnail_check) {
  if ($thumbnail_status=@fopen($thumbnail_check, "r")) {
    fclose($thumbnail_status);
    return $thumbnail_status;
  }
  return NULL;
}

function emvideo_veoh_thumbnail($field, $item, $formatter, $node, $width, $height) {
  $thumbid = $item['value'];
  //$xml = emvideo_veoh_request($item['value']);
  //print_r($xml);
  // make the thumb index zero to follow their standard web tag format.

  // - Rysk
  $thumbnail = 'http://p-images.veoh.com/thumb.out?imageId=thumb-'. $thumbid .'-'. rand(0, 30) .'.jpg';
  if (emvideo_veoh_check_thumbnail($thumbnail) != NULL) {
    return $thumbnail;
  } 
  else {
    $url = 'http://www.veoh.com/rest/video/'. $thumbid .'/details';
    $content = file_get_contents($url);
    if ($content) {
      $start = strpos($content, 'fullMedResImagePath="')+21;
      $end = strpos($content, '"', $start);
      $thumbnail = substr($content, $start, $end-$start);
    }
    return $thumbnail;
  }
}

function emvideo_veoh_video($embed, $width, $height, $field, $item, &$node, $autoplay) {
  $output = theme('emvideo_veoh_flash', $embed, $width, $height, $autoplay);
  return $output;
}

function emvideo_veoh_preview($embed, $width, $height, $field, $item, &$node, $autoplay) {
  $output = theme('emvideo_veoh_flash', $embed, $width, $height, $autoplay);
  return $output;
}
function emvideo_veoh_emfield_subtheme() {
  return array(
  'emvideo_veoh_flash' => array(
  'arguments' => array('embed' => NULL, 'width' => NULL, 'height' => NULL, 'autoplay' => NULL),
  'file' => 'providers/veoh.inc'
    )
  );
}
