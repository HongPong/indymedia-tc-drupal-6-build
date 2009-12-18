<?php
// $Id: features.content.inc,v 1.1.2.9 2009/08/26 05:13:41 yhahn Exp $

/**
 * Implementation of hook_features_api().
 */
function content_features_api() {
  return array(
    'content' => array(
      'default_hook' => 'content_default_fields',
    ),
  );
}

/**
 * Implementation of hook_features_export().
 */
function content_features_export($data, &$export, $module_name = '') {
  $pipe = array();

  // The content_default_fields() hook integration is provided by the
  // features module so we need to add it as a dependency.
  $export['dependencies']['features'] = 'features';

  // Collect a field to module map
  $map = array();
  $modules = module_implements('content_default_fields');
  foreach ($modules as $module) {
    $fields = module_invoke($module, 'content_default_fields');
    foreach ($fields as $field) {
      $map["{$field['type_name']}-{$field['field_name']}"] = $module;
    }
  }

  foreach ($data as $instance) {
    // If this field is already provided by another module, remove the field
    // and add the other module as a dependency.
    if (isset($map[$instance]) && $map[$instance] != $module_name) {
      if (isset($export['features']['content'][$instance])) {
        unset($export['features']['content'][$instance]);
      }
      $module = $map[$instance];
      $export['dependencies'][$module] = $module;
    }
    // If the field has not yet been exported, add it
    else {
      $split = explode('-', $instance);
      $type_name = $split[0];
      $field_name = $split[1];
      $field = content_fields($field_name, $type_name);

      if ($field) {
        // Add field item instance for later export.
        $export['features']['content'][$instance] = $instance;

        // Add module which provides field.
        $export['dependencies'][$field['module']] = $field['module'];

        // Add module which provides field widget.
        $export['dependencies'][$field['widget']['module']] = $field['widget']['module'];

        // Add modules which provide display.
        foreach (array('teaser', 'full') as $display) {
          $formatter = _content_get_formatter($field['display_settings'][$display]['format'], $field['type']);
          $export['dependencies'][$formatter['module']] = $formatter['module'];

          // TODO make this logic more generic, for now though we just handle
          // the imagecache presets.
          if ($formatter['module'] == 'imagecache') {
            $format = $field['display_settings'][$display]['format'];

            $parts = explode('_', $format);
            $style = array_pop($parts);
            $presetname = implode('_', $parts);

            $pipe[$formatter['module']][] = $presetname;
          }
        }
      }
    }
  }

  return $pipe;
}

/**
 * Implementation of hook_features_export_render().
 */
function content_features_export_render($module = 'foo', $data) {
  $translatables = $code = array();

  $code[] = '  $fields = array();';
  $code[] = '';
  foreach ($data as $instance) {
    $instance = explode('-', $instance);
    $type_name = $instance[0];
    $field_name = $instance[1];

    $field = content_fields($field_name, $type_name);
    unset($field['columns']);
    unset($field['locked']);
    unset($field['db_storage']);

    $code[] = '  // Exported field: '. $field_name;
    $code[] = '  $fields[] = '. features_var_export($field, '  ') .';';
    $code[] = '';

    // Add any labels to translatables array.
    if (!empty($field['widget']['label'])) {
      $translatables[] = $field['widget']['label'];
    }
  }
  if (!empty($translatables)) {
    $code[] = features_translatables_export($translatables, '  ');
  }
  $code[] = '  return $fields;';
  $code = implode("\n", $code);
  return array('content_default_fields' => $code);
}

/**
 * Implementation of hook_features_revert().
 *
 * @param $module
 * name of module to revert content for
 */
function content_features_revert($module = NULL) {
  // Get all fields defined in this feature
  if (module_hook($module, 'content_default_fields')) {
    module_load_include('inc', 'content', 'includes/content.crud');
    $default_fields = module_invoke($module, 'content_default_fields');

    // Update all fields to their default values
    foreach($default_fields as $field) {
      // Ensure that the field exists before attempting an update.
      $params = array('field_name' => $field['field_name'], 'type_name' => $field['type_name']);
      $field_exists = content_field_instance_read($params);
      if ($field_exists) {
        content_field_instance_update($field, FALSE);
        variable_set('menu_rebuild_needed', TRUE);
      }
    }
  }
  else {
    drupal_set_message(t('Could not load default fields.'), 'error');
    return FALSE;
  }
  return TRUE;
}

/**
 * Implementation of hook_features_rebuild().
 * Rebuilds CCK field definitions from code defaults.
 */
function content_features_rebuild() {
  module_load_include('inc', 'content', 'includes/content.crud');
  content_clear_type_cache(TRUE);

  $fields = module_invoke_all('content_default_fields');
  foreach ($fields as $field) {
    $conflicts = array();
    $existing_field = content_fields($field['field_name']);
    $existing_instance = content_fields($field['field_name'], $field['type_name']);

    // If field storage doesn't exists, create it
    if (!$existing_field) {
      content_field_instance_create($field, FALSE);
      variable_set('menu_rebuild_needed', TRUE);
    }
    // If field storage exists, check that storage and key descriptors don't conflict
    else {
      // Iterate through field properties to determine compatibility
      foreach ($field as $key => $value) {
        $excluded = array('type_name');
        if (!is_array($value) && !in_array($key, $excluded)) {
          if ($value !== $existing_field[$key]) {
            $conflicts[] = $key;
          }
        }
      }
      if (!empty($conflicts)) {
        $tokens = array('!field' => $field['field_name'], '!type' => $field['type_name'], '!list' => implode(', ', $conflicts));
        $message = t('Field <strong>!field</strong> was not created for <strong>!type</strong> because a conflict was detected for the following properties: <em>!list</em>', $tokens);
        drupal_set_message($message);
      }
      else if (!$existing_instance) {
        content_field_instance_create($field, FALSE);
        variable_set('menu_rebuild_needed', TRUE);
      }
    }
  }
}
