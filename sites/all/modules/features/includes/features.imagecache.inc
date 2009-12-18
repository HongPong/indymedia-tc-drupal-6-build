<?php
// $Id: features.imagecache.inc,v 1.1.2.10 2009/08/11 15:02:45 yhahn Exp $

/**
 * Implementation of hook_features_api().
 */
function imagecache_features_api() {
  return array(
    'imagecache' => array(
      'default_hook' => 'imagecache_default_presets',
    )
  );
}

/**
 * Implementation of hook_features_export_options().
 */
function imagecache_features_export_options() {
  $options = array();
  foreach (imagecache_presets() as $preset) {
    $options[$preset['presetname']] = $preset['presetname'];
  }
  return $options;
}

/**
 * Implementation of hook_features_export().
 */
function imagecache_features_export($data, &$export, $module_name = '') {
  // Collect a module to preset map
  $map = array();
  $modules = module_implements('imagecache_default_presets');
  foreach ($modules as $module) {
    $presets = module_invoke($module, 'imagecache_default_presets');
    foreach ($presets as $preset) {
      $map[$preset['presetname']] = $module;
    }
  }
  foreach ($data as $preset) {
    // If another module provides this preset, add it as a dependency
    if (isset($map[$preset]) && $map[$preset] != $module_name) {
      $module = $map[$preset];
      $export['dependencies'][$module] = $module;
    }
    // Otherwise, export the preset
    else {
      $export['features']['imagecache'][$preset] = $preset;
    }
  }
}

/**
 * Implementation of hook_features_export_render().
 */
function imagecache_features_export_render($module_name, $data) {
  $omit = array('presetid', 'storage');

  $items = array();
  foreach ($data as $key) {
    $preset = imagecache_preset_by_name($key);
    foreach ($omit as $i) {
      unset($preset[$i]);
    }
    $items[$key] = $preset;
  }
  $code = "  \$items = ". features_var_export($items, '  ') .";\n";
  $code .= '  return $items;';
  return array('imagecache_default_presets' => $code);
}

/**
 * Implementation of hook_features_revert().
 *
 * @param $module
 * name of module to revert content for
 */
function imagecache_features_revert($module = NULL) {
  // Get all presets defined in this feature
  if (module_hook($module, 'imagecache_default_presets')) {
    $default_presets = module_invoke($module, 'imagecache_default_presets');

    // Get all DB defined presets
    $current_presets = imagecache_presets();
    // update all presets to their default values
    foreach($default_presets as $default_preset) {
      foreach($current_presets as $current_preset) {
        if ($current_preset['presetname'] == $default_preset['presetname']) {
          imagecache_preset_delete($current_preset);
        }
      }
    }
  }
  else {
    drupal_set_message(t('Could not load default imagecache presets.'), 'error');
    return FALSE;
  }
  return TRUE;
}
