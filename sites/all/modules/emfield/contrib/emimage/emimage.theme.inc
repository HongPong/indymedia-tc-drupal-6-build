<?php


function theme_emimage_image($field, $item, $formatter, $node, $code, $width, $height, $title = '', $link = NULL) {
  $url = module_invoke('emfield', 'include_invoke', 'emimage', $item['provider'], 'image_url', $code, $width, $height, $formatter, $field, $item, $node);
  $attributes = array();
  if ($width) {
    $attributes['width'] = $width;
  }
  if ($height) {
    $attributes['height'] = $height;
  }
  if (!isset($width) || !isset($height)) {
    // Find out the size of the actual image file, and scale accordingly
    if ( $item['data']['width'] && $item['data']['height'] ) {
      // The image's width & height are known
      $scale_width = $item['data']['width'] / ($width ? $width : 1);
      $scale_height = $item['data']['height'] / ($height ? $height : 1);
      if ($scale_width > $scale_height) {
        $attributes['width'] = $width;
      }
      else {
        $attributes['height'] = $height;
      }
    }
    else {
      // We don't know the size of the image, so just make it fill the space available.
      // It will probably be stretched in one direction, making it look odd.
      $attributes['width'] = $width ? $width : NULL;
      $attributes['height'] = $height ? $height : NULL;
    }
	}
  if ($item['class']) {
      $attributes['class'] = $item['class'];
  }
  else if ($item['provider']) {
      $attributes['class'] = $item['provider'];
  }
  $output = theme('image', $url, $title, $title, $attributes, false);

  if ($link) {
    $output = l($output, $link, array('html' => true));
  }
  return $output;
}

function theme_emimage_image_thumbnail($field, $item, $formatter, $node) {
  if ($item['value'] && $item['provider']) {
    $code = $item['value'];
    $width = $field['widget']['thumbnail_width'];
    $height = $field['widget']['thumbnail_height'];
    $link = $field['widget']['thumbnail_link'];
    if ($link == EMIMAGE_LINK_CONTENT) {
      $link = 'node/'. $node->nid;
    }
    else if ($link == EMIMAGE_LINK_PROVIDER) {
      $link = module_invoke('emfield', 'include_invoke', 'emimage', $item['provider'], 'embedded_link', $code, $item['data']);
    }
    else {
      $link = NULL;
    }
    $title = module_invoke('emfield', 'include_invoke', 'emimage', $item['provider'], 'image_title', $code, $item['data']);
    $output = theme('emimage_image', $field, $item, $formatter, $node, $code, $width, $height, $title, $link);
  }
  return $output;
}

function theme_emimage_image_full($field, $item, $formatter, $node) {
  if ($item['value'] && $item['provider']) {
    $code = $item['value'];
    $width = $field['widget']['full_width'];
    $height = $field['widget']['full_height'];
    $link = $field['widget']['full_link'];
    if ($link == EMIMAGE_LINK_CONTENT) {
      $link = 'node/'. $node->nid;
    }
    else if ($link == EMIMAGE_LINK_PROVIDER) {
      $link = module_invoke('emfield', 'include_invoke', 'emimage', $item['provider'], 'embedded_link', $code, $item['data']);
    }
    else {
      $link = NULL;
    }
    $title = module_invoke('emfield', 'include_invoke', 'emimage', $item['provider'], 'image_title', $code, $item['data']);
    $output = theme('emimage_image', $field, $item, $formatter, $node, $code, $width, $height, $title, $link);
  }
  return $output;
}

function theme_emimage_default($field, $item, $formatter, $node) {
  return theme('emimage_image_full', $field, $item, $formatter, $node);
}

function theme_emimage_image_preview($field, $item, $formatter, $node) {
  if ($item['value'] && $item['provider']) {
    $code = $item['value'];
    $width = $field['widget']['preview_width'];
    $height = $field['widget']['preview_height'];
    $link = $field['widget']['preview_link'];
    if ($link == EMIMAGE_LINK_CONTENT) {
      $link = 'node/'. $node->nid;
    }
    else if ($link == EMIMAGE_LINK_PROVIDER) {
      $link = module_invoke('emfield', 'include_invoke', 'emimage', $item['provider'], 'embedded_link', $code, $item['data']);
    }
    else {
      $link = NULL;
    }
    $title = module_invoke('emfield', 'include_invoke', 'emimage', $item['provider'], 'image_title', $code, $item['data']);
    $output = theme('emimage_image', $field, $item, $formatter, $node, $code, $width, $height, $title, $link);
  }
  return $output;
}

function theme_emimage_formatter_image_image($element) {
  $field = content_fields($element['#field_name'], $element['#type_name']);
  return module_invoke('emimage', 'field_formatter', $field, $element['#item'], $element['#formatter'], $element['#node']);
}

function theme_emimage_formatter_image_embed($element) {
  $field = content_fields($element['#field_name'], $element['#type_name']);
  return module_invoke('emimage', 'field_formatter', $field, $element['#item'], $element['#formatter'], $element['#node']);
}

function theme_emimage_formatter_default($element) {
  $field = content_fields($element['#field_name'], $element['#type_name']);
  return module_invoke('emimage', 'field_formatter', $field, $element['#item'], $element['#formatter'], $element['#node']);
}

function theme_emimage_formatter_image_full($element) {
  $field = content_fields($element['#field_name'], $element['#type_name']);
  return module_invoke('emimage', 'field_formatter', $field, $element['#item'], $element['#formatter'], $element['#node']);
}

function theme_emimage_formatter_image_preview($element) {
  $field = content_fields($element['#field_name'], $element['#type_name']);
  return module_invoke('emimage', 'field_formatter', $field, $element['#item'], $element['#formatter'], $element['#node']);
}

function theme_emimage_formatter_image_thumbnail($element) {
  $field = content_fields($element['#field_name'], $element['#type_name']);
  return module_invoke('emimage', 'field_formatter', $field, $element['#item'], $element['#formatter'], $element['#node']);
}

function theme_emimage_image_embed($field, $item, $formatter, $node) {
  if ($item['value'] && $item['provider']) {
    $output = drupal_get_form('emimage_embed_form', $field, $item, $formatter, $node);
  }
  return $output;
}
