<?php
// $Id: zzz_custom_url.inc,v 1.1.2.15 2009/06/26 20:53:36 aaron Exp $

/**
 * @file
 *   This is an include file used by emfield.module.
 */

function _emvideo_zzz_custom_url_default_types() {
  return array('wmv', 'wma', 'swf', 'flv', 'mov', 'rm', 'mp4');
}

function emvideo_zzz_custom_url_info() {
  $name = t('Custom URL');
  $features = array(
    array(t('Thumbnails'), t('No'), ''),
    array(t('Autoplay'), t('Yes'), ''),
    array(t('RSS attachment'), t('No'), ''),
  );
  return array(
    'provider' => 'zzz_custom_url',
    'name' => $name,
    'url' => '',
    'settings_description' => t('These settings specifically affect videos displayed from custom URLs. When a field uses a URL it determines to be a link directly to a video file, it will embed that file into the content.'),
    'supported_features' => $features,
    'weight' => 9,
  );
}

function emvideo_zzz_custom_url_settings() {
  $options = array(
    'wmv' => t('Windows Media (wmv)'),
    'wma' => t('Windows Media (wma)'),
    'swf' => t('Flash (swf)'),
    'flv' => t('Flash Video (flv)'),
    'mov' => t('Quicktime (mov)'),
    'mp4' => t('Quicktime (mp4)'),
    'rm' => t('Real Media (rm)'),
  );
  $form = array();
  $form['emvideo_zzz_custom_url_supported_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Supported Types'),
    '#options' => $options,
    '#default_value' => variable_get('emvideo_zzz_custom_url_supported_types', _emvideo_zzz_custom_url_default_types()),
    '#description' => t('Select the video types you wish to support. When a custom url with that type is entered into an embedded video field, it will be parsed and displayed appropriately. If a type is not supported, then it will be ignored.'),
  );
  return $form;
}

function _emvideo_zzz_custom_url_implode_types() {
  return implode('|', variable_get('emvideo_zzz_custom_url_supported_types', _emvideo_zzz_custom_url_default_types()));
}

function emvideo_zzz_custom_url_extract($embed = '') {
  $types = _emvideo_zzz_custom_url_implode_types();
  $baseurl = preg_quote(url(null, array('absolute' => TRUE)), '@');

  return array(
    '@' . $baseurl . '(.*' . '\.(?:' . $types . ')' . '(\?.*)?)@i',
    '@(.*\.(?:'. $types .')(\?.*)?)@i'
  );
}

function emvideo_zzz_custom_url_data($field, $item) {
  $data = array();
  // adding the version control
  $data['emvideo_zzz_custom_url_data_version'] = 1;

  // attempt to get info from headers
  $response = emfield_request_header('zzz_custom_url', $item['embed']);

  if ($response->code == 200) {
    $data['url'] = $item['embed'];
    $data['size'] = $response->headers['Content-Length'];
    $data['mime'] = $response->headers['Content-Type'];
  }
  // @todo replace ['type'] with converted mime info if available
  $types = _emvideo_zzz_custom_url_implode_types();
  $regex = '@\.('. $types .')$@i';
  if (preg_match($regex, $item['embed'], $matches)) {
    $data['type'] = $matches[1];
  }
  return $data;
}

/**
 * hook emfield_PROVIDER_rss
 */
function emvideo_zzz_custom_url_rss($item, $teaser = NULL) {
  if ($item['value']) {
    if ($item['data']['emvideo_zzz_custom_url_data_version'] >= 1) {
      $data = $item['data'];
    }
    else {
      $data = emvideo_zzz_custom_url_data(NULL, $item);
    }

    $file = array();
    if ($data['size'] > 0) {
      $file['filepath'] = $data['url'];
      $file['filesize'] = $data['size'];
      $file['filemime'] = $data['mime'];
    }

    return $file;
  }
}


function emvideo_zzz_custom_url_embedded_link($video_code) {
  return $video_code;
}

function theme_emvideo_zzz_custom_url_embedded_video($type, $url, $width, $height, $autoplay = FALSE, $field = NULL, $item = NULL) {
  if ($url = url($url, array('absolute' => TRUE))) {
      if (module_exists('flowplayer')) {
      $config = array(
        'clip' => array(
          'autoPlay' => $autoplay,
          'url' => $url,
        ),
      );
      $attributes = array(
        'style' => "width:{$width}px;height:{$height}px;",
      );
      return theme('flowplayer', $config, 'emvideo-zzz-custom-url-embedded-video', $attributes);
    }
    switch ($type) {
      case 'wmv':
      case 'wma':
        $autostart = $autoplay ? '1' : '0';
        return '<embed src="'. $url .'" width="'. $width .'" height="'. $height .'" autostart="'. $autostart .'" showcontrols="1" type="application/x-mplayer2" pluginspage="http://www.microsoft.com/windows/windowsmedia/download/"> </embed>';
      case 'mov':
      case 'mp4':
        $autostart = $autoplay ? 'true' : 'false';
        return '<embed src="'. $url .'" width="'. $width .'" height="'. $height .'" autoplay="'. $autostart .'" controller="true" type="video/quicktime" scale="tofit" pluginspage="http://www.apple.com/quicktime/download/"> </embed>';
      case 'rm':
        $autostart = $autoplay ? 'true' : 'false';
        return '<embed type="audio/x-pn-realaudio-plugin" src="'. $url .'" width="'. $width .'" height="'. $height .'" autostart="'. $autostart .'" controls="imagewindow" nojava="true" console="c1183760810807" pluginspage="http://www.real.com/"></embed><br /><embed type="audio/x-pn-realaudio-plugin" src="'. $url .'" width="'. $width .'" height="26" autostart="'. $autostart .'" nojava="true" controls="ControlPanel" console="c1183760810807"> </embed>';
      case 'swf':
        return '<embed src="'. $url .'" width="'. $width .'" height="'. $height .'" quality="high" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer"></embed>';
      case 'flv':
        $autostart = $autoplay ? 'true' : 'false';
        return '<embed src="http://freevideocoding.com/flvplayer.swf?file='. $url .'&amp;autoStart='. $autostart .'" width="'. $width .'" height="'. $height .'" quality="high" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer"></embed>';
    }
  }
}

function emvideo_zzz_custom_url_thumbnail($field, $item, $formatter, $node, $width, $height) {
  return '';
}

function emvideo_zzz_custom_url_video($code, $width, $height, $field, $item, &$node, $autoplay) {
  $type = $item['data']['type'];
  $output = theme('emvideo_zzz_custom_url_embedded_video', $type, $code, $width, $height, $autoplay, $field, $item);
  return $output;
}

function emvideo_zzz_custom_url_preview($code, $width, $height, $field, $item, &$node, $autoplay) {
  $type = $item['data']['type'];
  $output = theme('emvideo_zzz_custom_url_embedded_video', $type, $code, $width, $height, $autoplay, $field, $item);
  return $output;
}

/**
 * Implementation of hook_emfield_subtheme.
 */
function emvideo_zzz_custom_url_emfield_subtheme() {
  return array(
    'emvideo_zzz_custom_url_embedded_video' => array(
      'arguments' => array('type' => NULL, 'url' => NULL, 'width' => NULL, 'height' => NULL, 'autoplay' => FALSE, 'field' => NULL, 'item' => NULL),
      'file' => 'providers/zzz_custom_url.inc'
    )
  );
}
