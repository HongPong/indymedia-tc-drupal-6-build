<?php
// $Id: features.drush.inc,v 1.1.2.12 2009/08/11 15:02:45 yhahn Exp $

/**
 * @file
 *   Features module drush integration.
 */

/**
 * Implementation of hook_drush_command().
 * 
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function features_drush_command() {
  $items = array();

  $items['features'] = array(
    'callback' => 'features_command_list',
    'description' => "List all the available features for your site.",
  );
  $items['features export'] = array(
    'callback' => 'features_command_export',
    'description' => "Export a feature from your site into a module.",
    'arguments' => array(
      'feature' => 'Feature name to export.',
    ),
  );
  $items['features update'] = array(
    'callback' => 'features_command_update',
    'description' => "Update a feature module on your site.",
    'arguments' => array(
      'feature' => 'Feature name to export.',
    ),
    );
  $items['features revert'] = array(
    'callback' => 'features_command_revert',
    'description' => "Revert a feature module on your site.",
    'arguments' => array(
      'feature' => 'Feature name to revert.',
    ),
  );

  return $items;
}

/**
 * Implementation of hook_drush_help().
 */
function features_drush_help($section) {
  switch ($section) {
    case 'drush:features':
      return dt("List all the available features for your site.");
    case 'drush:features export':
      return dt("Export a feature from your site into a module.");
  }
}

/**
 * Get a list of all feature modules.
 */
function features_command_list() {
  $rows = array(array(dt('Name'), dt('Enabled/Disabled'), dt('From'), dt('Timestamp')));
  foreach (features_get_features() as $k => $m) {
    $rows[] = array(
      $m->name,
      $m->status ? dt('Enabled') : dt('Disabled'),
      $m->info['feature_uri'],
      format_date($m->info['feature_timestamp'])
    );
  }
  drush_print_table($rows, TRUE);
}

/**
 * Create a feature module based on a list of contexts.
 */
function features_command_export() {
  $args = func_get_args();

  if (count($args) == 1) {
    // Assume that the user intends to create a module with the same name as the
    // "value" of the component.
    list($source, $component) = explode(':', $args[0]);
    $stub = array($source => array($component));
    _features_command_export($stub, $component);
  }
  elseif (count($args) > 1) {
    // Assume that the user intends to create a new module based on a list of 
    // contexts. First argument is assumed to be the name.
    $name = array_shift($args);
    $stub = array();
    foreach ($args as $v) {
      list($source, $component) = explode(':', $v);
      $stub[$source][] = $component;
    }
    _features_command_export($stub, $name);
  }
  else {
    drush_print(dt('Available sources'));
    $rows = array(dt('Available sources'));
    foreach (features_get_components(TRUE) as $component => $info) {
      if ($options = features_invoke($component, 'features_export_options')) {
        foreach ($options as $key => $value) {
          $rows[] = array($component .':'. $key);
        }
      }
    }
    drush_print_table($rows, TRUE);
  }
}

/**
 * Create a feature module based on a list of contexts.
 */
function features_command_update() {
  $args = func_get_args();

  if (count($args) == 1) {
    $module = array_shift($args);
    if ($feature = feature_load($module, TRUE)) {
      $stub = $feature->info['features'];

      // Construct the correct destination path
      $destination = dirname($feature->filename);
      $split = explode('/', $destination);
      if ($feature->name == array_pop($split)) {
        $destination = implode('/', $split);
      }

      _features_command_export($stub, $feature->name, $destination);
    }
    else {
      drush_die('The feature '. $module .' could not be found.');
    }
  }
  else {
    // By default just show contexts that are available.
    $rows = array(array(dt('Available features')));
    foreach (features_get_features(NULL, TRUE) as $name => $info) {
      $rows[] = array($name);
    }
    drush_print_table($rows, TRUE);
  }
}

/**
 * Write a module to the site dir.
 *
 * @param $requests
 *   An array of the context requested in this export.
 * @param $module_name
 *  Optional. The name for the exported module, it omitted will be generated
 *  from the first listed context.
 */
function _features_command_export($stub, $module_name = NULL, $destination = 'sites/all/modules') {
  drupal_flush_all_caches();

  module_load_include('inc', 'features', 'features.export');
  $export = features_populate($stub, $module_name);
  if ($feature = feature_load($module_name)) {
    $export['name'] = $feature->info['name'];
    $export['description'] = $feature->info['description'];
  }
  else {
    $export['name'] = $module_name;
  }
  $files = features_export_render($export, $module_name, TRUE);

  if ($root = drush_locate_root()) {
    $directory = "$destination/{$module_name}";
    if (is_dir($directory)) {
      drush_print(dt('Module appears to already exist in !dir', array('!dir' => $directory)));
      if (!drush_confirm(dt('Do you really want to continue?'))) {
        drush_die('Aborting.');
      }
    }
    else {
      drush_op('mkdir', $directory);
    }
    foreach ($files as $extension => $file_contents) {
      if (!in_array($extension, array('module', 'info'))) {
        $extension .= '.inc';
      }
      drush_op('file_put_contents', "{$directory}/{$module_name}.$extension", $file_contents);
    }
    drush_print(dt("Created module: !module in !directory", array('!module' => $module_name, '!directory' => $directory)));
  }
  else {
    drush_die(dt('Couldn\'t locate site root'));
  }
}

/**
 * Revert a feature to it's code definition.
 */
function features_command_revert() {
  $args = func_get_args();

  if (count($args) == 1) {
    $module = array_shift($args);
    if (!($feature = feature_load($module, TRUE))) {
       drush_die('The feature '. $module .' could not be found.');
    }
  }
  else {
    // By default just show contexts that are available.
    $rows = array(array(dt('Available features')));
    foreach (features_get_features(NULL, TRUE) as $name => $info) {
      $rows[] = array($name);
    }
    drush_print_table($rows, TRUE);
    return;
  }

  // Find all overridden items
  module_load_include('inc', 'features', 'features.export');
  $overrides = features_detect_overrides($feature);
  if (count(array_keys($overrides)) > 0) {
    drush_print(dt('Found !number component!plural to revert: !components', array('!number' => count(array_keys($overrides)),
      '!components' => implode(array_keys($overrides), ', '),
      '!plural' => count(array_keys($overrides)) > 1 ? 's' : '')));
  }
  else {
    drush_print(dt('Current state already matches defaults, aborting.'));
    return;
  }
  foreach (array_keys($overrides) as $component) {
    if (!drush_confirm(dt('Do you really want to revert !component?', array('!component' => $component)))) {
      drush_print(dt('Skipping !component.', array('!component' => $component)));
      continue;
    }
    features_include();
    if (!features_hook($component, 'features_revert')) {
      drush_print(dt('Features does not know how to revert !component', array('!component' => $component)));
      continue;
    }
    $result = features_invoke($component, 'features_revert', $module);
    if (!$result) {
      drush_print(dt('Revert !component failed.', array('!component' => $component)));
    }
    else {
      drush_print(dt('Reverted !component.', array('!component' => $component)));
    }
  }
}
