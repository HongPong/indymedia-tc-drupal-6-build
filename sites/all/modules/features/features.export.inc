<?php
// $Id: features.export.inc,v 1.1.2.33 2009/08/11 20:39:42 yhahn Exp $

/**
 * Update form.
 */
function features_update_form($form_state, $module) {
  features_include();
  $export = array_merge($module->info, features_populate($module->info['features'], $module->name));

  $form['markup'] = array(
    '#type' => 'markup',
    '#value' => "<h3>". t('Update !module_name', array('!module_name' => $module->name)) ."</h3>",
  );
  $form['export'] = array(
    '#type' => 'value',
    '#value' => $export,
  );
  $form['name'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#default_value' => !empty($module->info['name']) ? $module->info['name'] : '',
  );
  $form['module_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Machine-readable name'),
    '#value' => $module->name,
    '#default_value' => $module->name,
    '#disabled' => TRUE,
  );
  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#default_value' => !empty($module->info['description']) ? $module->info['description'] : '',
  );
  $form['optional'] = array(
    '#title' => t('Optional information'),
    '#type' => 'fieldset',
    '#tree' => FALSE,
  );
  $form['optional']['project_status_url'] = array(
    '#title' => t('Update feed'),
    '#type' => 'textfield',
    '#required' => FALSE,
    '#default_value' => !empty($module->info['project status url']) ? $module->info['project status url'] : '',
  );
  $form['optional']['version'] = array(
    '#title' => t('Version'),
    '#type' => 'textfield',
    '#required' => FALSE,
    '#default_value' => !empty($module->info['version']) ? $module->info['version'] : '',
  );
  $form['download'] = array(
    '#type' => 'submit',
    '#value' => t('Download updated feature'),
  );
  return $form;
}

/**
 * Update form submit.
 */
function features_update_form_submit(&$form, &$form_state) {
  $export = $form_state['values']['export'];
  $export['name'] = $form_state['values']['name'];
  $export['description'] = $form_state['values']['description'];
  if (!empty($form_state['values']['version'])) {
    $export['version'] = $form_state['values']['version'];
  }
  if (!empty($form_state['values']['project_status_url'])) {
    $export['project status url'] = $form_state['values']['project_status_url'];
  }

  $module_name = $form_state['values']['module_name'];
  $filename = !empty($export['version']) ? "{$module_name}-{$export['version']}" : $module_name;

  $files = features_export_render($export, $module_name, TRUE);
  features_export_download_files($files, $module_name, $filename);
}

/**
 * Export form.
 */
function features_export_form($form_state, $module = NULL) {
  features_include();

  $steps = array(
    t('Step 1: provide basic information'),
    t('Step 2: choose sources'),
    t('Step 3: confirm components'),
    t('Step 4: download your feature'),
  );

  // retrieve storage variables
  $module_name = !empty($form_state['storage']['module_name']) ? $form_state['storage']['module_name'] : '';
  $export = !empty($form_state['storage']['export']) ? $form_state['storage']['export'] : array();

  // we are re-exporting a module -- load up data and export
  if (!empty($module)) {
    $step = $form_state['storage']['step'] = 3;
    $steps[3] = t('Update your feature: !module_name', array('!module_name' => $module->name));
    $module_name = $module->name;
    $export = array_merge($module->info, features_populate($module->info['features'], $module->name));
  }
  // we are coming in without a step, so default to step 1
  else {
    $step =
    $form_state['storage']['step'] = empty($form_state['storage']['step']) ? 0 : $form_state['storage']['step'];
  }

  $form = array();
  $form['step'] = array(
    '#type' => 'markup',
    '#value' => "<h3>{$steps[$step]}</h3>",
  );

  switch ($step) {

    // Provide additional information =================================
    case 0:
      $form['name'] = array(
        '#title' => t('Name'),
        '#description' => t('Provide a name for your feature.'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => '',
        '#attributes' => array('class' => 'feature-name'),
      );
      $form['module_name'] = array(
        '#type' => 'textfield',
        '#title' => t('Machine-readable name'),
        '#description' => t('Provide a machine-readable name for your feature. This may only contain lowercase letters, numbers and underscores. <strong>It should also avoid conflicting with the names of any existing Drupal modules.</strong>'),
        '#required' => TRUE,
        '#default_value' => '',
        '#attributes' => array('class' => 'feature-module-name'),
      );
      $form['description'] = array(
        '#title' => t('Description'),
        '#description' => t('Provide a description for your feature.'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => '',
      );
      $form['optional'] = array(
        '#title' => t('Optional information'),
        '#type' => 'fieldset',
        '#tree' => FALSE,
      );
      $form['optional']['project_status_url'] = array(
        '#title' => t('Update feed'),
        '#description' => t('The URL of the update XML feed for this feature.'),
        '#type' => 'textfield',
        '#required' => FALSE,
        '#default_value' => '',
      );
      $form['optional']['version'] = array(
        '#title' => t('Version'),
        '#description' => t('The version information for this feature.'),
        '#type' => 'textfield',
        '#required' => FALSE,
        '#default_value' => '',
      );
      drupal_add_js(drupal_get_path('module', 'features') .'/features.js');
      break;

    // Choose a feature source ========================================
    case 1:
      $form['sources'] = array('#tree' => TRUE);
      foreach (features_get_components(TRUE) as $component => $info) {
        if ($options = features_invoke($component, 'features_export_options')) {
          $form['sources'][$component] = array(
            '#type' => 'checkboxes',
            '#options' => $options,
            '#title' => $component,
          );
        }
      }
      break;

    // Confirm components =============================================
    case 2:
      $form['#theme'] = 'features_export_form_confirm';
      $form['detected'] =
      $form['added'] = array('#tree' => TRUE);

      foreach ($export['conflicts'] as $type => $messages) {
        foreach ($messages as $msg) {
          drupal_set_message($msg, $type);
        }
      }

      // Display each set of components and options for adding to the components
      foreach (features_get_components() as $component => $info) {
        $export_options = features_hook($component, 'features_export_options') ? features_invoke($component, 'features_export_options') : array();
        $detected_components = !empty($export['features'][$component]) ? $export['features'][$component] : array();

        if (!empty($detected_components) || !empty($export_options)) {
          if (!empty($detected_components)) {
            $form['detected'][$component] = array(
              '#type' => 'markup',
              '#value' => theme('item_list', array_keys($detected_components)),
            );
          }
          if (!empty($export_options)) {
            $form['added'][$component] = array(
              '#type' => 'checkboxes',
              '#title' => $component,
              '#options' => array_diff_key($export_options, $detected_components),
            );
          }
          else {
            $form['added'][$component] = array(
              '#type' => 'markup',
              '#title' => $component,
              '#value' => "<em>". t('This component does not support additional options.') ."</em>",
            );
          }
        }
      }

      // Dependencies
      $form['detected']['dependencies'] = array(
        '#type' => 'markup',
        '#value' => theme('item_list', array_keys($export['dependencies'])),
      );

      $options = array();
      foreach (features_get_modules() as $module_name => $info) {
        if ($info->status && !empty($info->info)) {
          $options[$module_name] = $info->info['name'];
        }
      }
      $options = array_diff_key($options, $export['dependencies']);

      $form['added']['dependencies'] = array(
        '#tree' => TRUE,
        '#title' => t('Module dependencies'),
        '#type' => 'checkboxes',
        '#options' => $options,
      );
      break;

    // Download/export ================================================
    case 3:
      if ($files = features_export_render($export, $module_name, TRUE)) {
        $filename = !empty($export['version']) ? "{$module_name}-{$export['version']}" : $module_name;
        $module_display = function_exists('gzencode') ? "{$filename}.tgz" : "{$filename}.tar";

        $form['module_display'] = array(
          '#type' => 'markup',
          '#value' => "<h3 class='features-download'>{$module_display}</h3>",
        );

        $form['download'] = array('#tree' => TRUE, '#theme' => 'features_form_buttons');
        $form['download']['module_name'] = array('#type' => 'value', '#value' => $module_name);
        $form['download']['filename'] = array('#type' => 'value', '#value' => $filename);
        $form['download']['files'] = array('#type' => 'value', '#value' => $files);

        $form['download']['download'] = array(
          '#type' => 'submit',
          '#value' => t('Download feature'),
          '#submit' => array('features_export_download_submit'),
        );
      }
      break;
  }

  // Add Next/Prev step buttons
  $form['buttons'] = array('#tree' => FALSE, '#theme' => 'features_form_buttons');
  if ($step > 0 && $step != count($steps) - 1) {
    $form['buttons']['prev'] = array('#value' => t('Previous'), '#type' => 'submit');
  }
  if ($step < count($steps) - 1) {
    $form['buttons']['next'] = array('#value' => t('Next'), '#type' => 'submit');
    if (drupal_get_messages('error', FALSE)) {
      $form['buttons']['message'] = array(
        '#type' => 'markup',
        '#value' => "<div class='description'>". t('You should resolve all errors with your feature before continuing.') ."</div>",
      );
    }
  }

  return $form;
}

/**
 * Export form submit handler.
 */
function features_export_form_submit($form, &$form_state) {
  // tell Drupal we are redrawing the same form
  $form_state['rebuild'] = TRUE;

  switch ($form_state['storage']['step']) {

    // Step 0: Store info
    case 0:
      $export = array(
        'name' => $form_state['values']['name'],
        'description' => $form_state['values']['description'],
      );
      if (!empty($form_state['values']['version'])) {
        $export['version'] = $form_state['values']['version'];
      }
      if (!empty($form_state['values']['project_status_url'])) {
        $export['project status url'] = $form_state['values']['project_status_url'];
      }
      $form_state['storage']['export'] = $export;
      $form_state['storage']['module_name'] = $form_state['values']['module_name'];
      break;

    // Step 1: Convert sources into export object
    case 1:
      $module_name = $form_state['storage']['module_name'];
      $export = $form_state['storage']['export'];

      // Retrieve export
      $feature = array();
      foreach (element_children($form['sources']) as $elem) {
        if (!empty($form_state['values']['sources'][$elem])) {
          foreach ($form_state['values']['sources'][$elem] as $identifier => $value) {
            if ($value) {
              $feature[$elem][] = $identifier;
            }
          }
        }
      }

      $export = array_merge($export, features_populate($feature, $module_name));
      $form_state['storage']['export'] = $export;
      break;

    // Step 2: Update export object based on user input
    case 2:
      $module_name = $form_state['storage']['module_name'];
      $export = $form_state['storage']['export'];

      // Update export array based on what's been selected
      foreach ($form_state['values']['added'] as $component => $items) {
        if ($component == 'dependencies') {
          foreach ($items as $item => $value) {
            if ($value) {
              $export['dependencies'][$item] = $item;
            }
            else if (!empty($export['dependencies'][$item])) {
              unset($export['dependencies'][$item]);
            }
          }
        }
        else {
          foreach ($items as $item => $value) {
            if ($value) {
              $export['features'][$component][$item] = $item;
            }
            else if (!empty($export['features'][$component][$item])) {
              unset($export['features'][$component][$item]);
            }
          }
        }
      }

      // Build final export array
      $populated = features_populate($export['features'], $module_name);
      $export['features'] = array_merge($export['features'], $populated['features']);
      $export['dependencies'] = array_merge($export['dependencies'], $populated['dependencies']);
      $export['dependencies'] = _features_export_minimize_dependencies($export['dependencies'], $module_name);

      $form_state['storage']['export'] = $export;
      break;
  }

  // check the button that was clicked and action the step chagne
  if ($form_state['clicked_button']['#id'] == 'edit-prev') {
    $form_state['storage']['step']--;
  }
  elseif ($form_state['clicked_button']['#id'] == 'edit-next') {
    $form_state['storage']['step']++;
  }
}

/**
 * Download submit handler.
 */
function features_export_download_submit(&$form, &$form_state) {
  if (!empty($form_state['values']['download']['files'])) {
    $files = $form_state['values']['download']['files'];
    $module_name = $form_state['values']['download']['module_name'];
    $filename = $form_state['values']['download']['filename'];
    features_export_download_files($files, $module_name, $filename);
  }
}

/**
 * Delivers files to the user through a tarball download.
 */
function features_export_download_files($files, $module_name, $filename = '') {
  $tar = array();
  foreach ($files as $extension => $file_contents) {
    if (!in_array($extension, array('module', 'info'))) {
      $extension .= '.inc';
    }
    $tar["{$module_name}/{$module_name}.$extension"] = $file_contents;
  }
  $tar = features_tar_create($tar);

  $header = function_exists('gzencode') ? 'Content-type: application/x-gzip' : 'Content-type: application/x-tar';
  $filename = !empty($filename) ? $filename : $module_name;
  $filename = function_exists('gzencode') ? "{$filename}.tgz" : "{$filename}.tar";

  drupal_set_header($header);
  drupal_set_header('Content-Disposition: attachment; filename="'. $filename .'"');
  print $tar;
  exit;
}

/**
 * @param $items
 * @param $module_name
 * @return
 */
function features_populate($items, $module_name) {
  $stub = array('features' => array(), 'dependencies' => array(), 'conflicts' => array());
  $export = _features_populate($items, $stub, $module_name);
  $export['dependencies'] = _features_export_minimize_dependencies($export['dependencies'], $module_name);
  return $export;
}

/**
 * Iterate and descend into a feature definition to extract module
 * dependencies and feature definition. Calls hook_features_export for modules
 * that implement it.
 *
 * @param $pipe
 *  Associative of array of module => info-for-module
 * @param $export
 *  Associative array of items, and module dependencies which define a feature.
 *  Passed by reference.
 *
 * @return fully populated $export array.
 */
function _features_populate($pipe, &$export, $module_name = '') {
  features_include();
  foreach ($pipe as $component => $data) {
    if (features_hook($component, 'features_export')) {
      // Pass module-specific data and export array.
      // We don't use features_invoke() here since we need to pass $export by reference.
      $function = "{$component}_features_export";
      $more = $function($data, $export, $module_name);
      // Allow for export functions to request additional exports.
      if (!empty($more)) {
        _features_populate($more, $export, $module_name);
      }
    }
  }
  return $export;
}

/**
 * Iterates over a list of dependencies and kills modules that are
 * captured by other modules 'higher up'.
 */
function _features_export_minimize_dependencies($dependencies, $module_name = '') {
  // Ensure that the module doesn't depend upon itself
  if (!empty($module_name) && !empty($dependencies[$module_name])) {
    unset($dependencies[$module_name]);
  }
  foreach ($dependencies as $k => $v) {
    if (empty($v)) {
      unset($dependencies[$k]);
    }
    else {
      $module = features_get_modules($v);
      if ($module && !empty($module->info['dependencies'])) {
        // If this dependency depends on the module itself, we have a circular dependency.
        // Don't let it happen. Only you can prevent forest fires.
        if (in_array($module_name, $module->info['dependencies'])) {
          unset($dependencies[$k]);
        }
        // Iterate through the dependency's dependencies and remove any dependencies
        // that are captured by it.
        else {
          foreach ($module->info['dependencies'] as $j => $dependency) {
            if (array_search($dependency, $dependencies) !== FALSE) {
              $position = array_search($dependency, $dependencies);
              unset($dependencies[$position]);
            }
          }
        }
      }
    }
  }
  return drupal_map_assoc(array_unique($dependencies));
}

/**
 * Iterates over a list of dependencies and maximize the list of modules.
 */
function _features_export_maximize_dependencies($dependencies, $module_name = '', $first = TRUE) {
  $maximized = $dependencies;
  foreach ($dependencies as $k => $v) {
    $module = features_get_modules($v);
    if ($module && !empty($module->info['dependencies'])) {
      $maximized = array_merge($maximized, _features_export_maximize_dependencies($module->info['dependencies'], $module_name, FALSE));
    }
  }
  return array_unique($maximized);
}

/**
 * Prepare a feature export into an array representing its files and default hooks.
 */
function features_export_prepare($export, $module_name, $reset = FALSE) {
  features_include();
  $code = array();

  // Sort components to keep exported code consistent
  ksort($export['features']);

  foreach ($export['features'] as $component => $data) {
    if (!empty($data)) {
      // Sort the items so that we don't generate different exports based on order
      asort($data);
      if (features_hook($component, 'features_export_render')) {
        $hooks = features_invoke($component, 'features_export_render', $module_name, $data, $export);
        $code[$component] = $hooks;
      }
    }
  }
  return $code;
}

/**
 * Render feature export into an array representing its files.
 *
 * @param $export
 *  An exported feature definition.
 * @param $module_name
 *  The name of the module to be exported.
 * @param $reset
 *  Boolean flag for resetting the module cache. Only set to true when
 *  doing a final export for delivery.
 *
 * @return array of info file and module file contents.
 */
function features_export_render($export, $module_name, $reset = FALSE) {
  $existing = features_get_modules($module_name, $reset);
  $components = features_get_components();

  // Prepare info string -- if module exists, merge into its existing info file
  $info = $existing ? $existing->info : array('core' => '6.x', 'package' => 'Features', 'project' => $module_name);
  $export = array_merge($info, $export);

  // A couple of special cases.
  if (!empty($export['dependencies'])) {
    $export['dependencies'] = array_values($export['dependencies']);
  }
  if (!empty($export['conflicts'])) {
    unset($export['conflicts']);
  }

  // Generate code by component, hook
  $code = array();
  $component_hooks = features_export_prepare($export, $module_name, $reset);

  // Group component code into their respective files
  foreach ($component_hooks as $component => $hooks) {
    $file = array('name' => 'defaults', 'stub' => true);
    if (isset($components[$component]['default_file'])) {
      switch ($components[$component]['default_file']) {
        case FEATURES_DEFAULTS_INCLUDED:
          $file['name'] = "features.$component";
          break;
        case FEATURES_DEFAULTS_CUSTOM:
          $file['name'] = $components[$component]['default_filename'];
          $file['stub'] = false;
          break;
      }
    }

    if (!isset($code[$file['name']])) {
      $code[$file['name']] = array();
    }

    foreach ($hooks as $hook_name => $hook_code) {
      if ($file['stub']) {
        if (!isset($code['features'])) {
          $code['features'] = array();
        }
        $code['features'][$hook_name] = features_export_render_features($module_name, $hook_name, $file['name']);
      }
      $code[$file['name']][$hook_name] = features_export_render_defaults($module_name, $hook_name, $hook_code);
    }
  }

  // Finalize strings to be written to files
  foreach ($code as $filename => $contents) {
    $code[$filename] = "<?php\n\n". implode("\n\n", $contents) ."\n";
  }

  // Generate info file string
  $code['info'] = features_export_info($export);

  // Prepare the module
  // If module exists, let it be and include it in the files
  if ($existing) {
    $code['module'] = file_get_contents($existing->filename);
  }
  // Add a stub module to include the defaults
  elseif (!empty($code['features'])) {
    $code['module'] = "<?php\n\ninclude_once('{$module_name}.features.inc');\n";
  }
  return $code;
}

/**
 * Detect differences between DB and code components of a feature.
 */
function features_detect_overrides($module) {
  static $cache;
  if (!isset($cache)) {
    $cache = array();
  }
  if (!isset($cache[$module->name])) {
    // Make necessary inclusions
    if (module_exists('views')) {
      views_include('view');
    }

    // Retrieve default hooks
    $default_hooks = features_get_default_hooks();

    // Rebuild feature from .info file description and prepare an export from current DB state.
    $export = features_populate($module->info['features'], $module->name);
    // $export = array_merge($export, $module->info);
    $code = features_export_prepare($export, "_features_comparison_{$module->name}");

    $overridden = array();

    // First, compare actual component sets and mark if different
    if (sort(array_keys($export['features'])) != sort(array_keys($module->info['features']))) {
      $overridden['components'] = array(
        'default' => sort(array_keys($module->info['features'])),
        'current' => sort(array_keys($export['features']))
      );
    }

    // Merge items from both for comparison
    $merged = array_merge($export['features'], $module->info['features']);

    foreach (array_keys($merged) as $component) {
      if (isset($default_hooks[$component])) {
        $default_hook = $default_hooks[$component];

        // Eval the database version of the export
        if (isset($code[$component][$default_hook])) {
          $current[$component] = eval($code[$component][$default_hook]);
        }
        // Call the existing in-code function and collect results
        if (module_hook($module->name, $default_hook)) {
          $default[$component] = module_invoke($module->name, $default_hook);
        }

        // Compare, and push differences into the overrides array
        if (isset($current[$component]) && is_array($current[$component])) {
          foreach ($current[$component] as $j => $k) {
            // Special cases for objects -- some (views) provide their own
            // export methods which we need to respect.
            if (is_object($current[$component][$j])) {
              switch (get_class($current[$component][$j])) {
                case 'view':
                  $a = $current[$component][$j]->export();
                  if ($default[$component][$j]) {
                    $b = $default[$component][$j]->export();
                  }
                  break;
                default:
                  $a = features_var_export($current[$component][$j]);
                  $b = features_var_export($default[$component][$j]);
                  break;
              }
              $a = _features_linetrim(explode("\n", $a));
              $b = _features_linetrim(explode("\n", $b));

              $different = $a !== $b;
            }
            else {
              $different = $current[$component][$j] !== $default[$component][$j];
            }
            if ($different) {
              $overridden[$component] = array(
                'default' => $default[$component][$j],
                'current' => $current[$component][$j],
              );
            }
          }
        }
      }
    }
    $cache[$module->name] = $overridden;
  }
  return $cache[$module->name];
}

/**
 * Gets the available default hooks keyed by components.
 */
function features_get_default_hooks() {
  static $hooks;
  if (!isset($hooks)) {
    $hooks = array();
    features_include();
    foreach (module_implements('features_api') as $module) {
      $info = module_invoke($module, 'features_api');
      foreach ($info as $k => $v) {
        if (isset($v['default_hook'])) {
          $hooks[$k] = $v['default_hook'];
        }
      }
    }
  }
  return $hooks;
}

/**
 * Return a code string representing an implementation of a module hook.
 * Includes the module's defaults .inc and calls the private helper function.
 *
 * @param $module
 *   The name of the module being generated
 * @param $hook
 *   The name of the hook, without the "hook_" prefix. "hook_node_info" should
 *   be "node_info"
 * @param $filename
 *   Name of the include file. For a module 'foo', and filename'defaults'
 *   becomes 'foo.defaults.inc'.
 */
function features_export_render_features($module, $hook, $filename = 'defaults') {
  $output = array();
  $output[] = "/**";
  $output[] = " * Implementation of hook_{$hook}().";
  $output[] = " */";
  $output[] = "function {$module}_{$hook}() {";
  $output[] = "  module_load_include('inc', '{$module}', '{$module}.{$filename}');";
  $output[] = '  $args = func_get_args();';
  $output[] = "  return call_user_func_array('_{$module}_{$hook}', ".'$args'.");";
  $output[] = "}";
  return implode("\n", $output);
}

/**
 * Return a code string representing an implementation of a defaults module hook.
 */
function features_export_render_defaults($module, $hook, $code) {
  $output = array();
  $output[] = "/**";
  $output[] = " * Helper to implementation of hook_{$hook}().";
  $output[] = " */";
  $output[] = "function _{$module}_{$hook}() {";
  $output[] = $code;
  $output[] = "}";
  return implode("\n", $output);
}

/**
 * Generate code friendly to the Drupal .info format from a structured array.
 *
 * @param $info
 *   An array of parameters to put in a module's .info file.
 *
 * @return
 *   A code string ready to be written to a module's .info file.
 */
function features_export_info($info) {
  $code = array();
  foreach ($info as $k => $v) {
    if (is_array($v)) {
      $first = array_shift(array_keys($v));
      if (is_numeric($first)) {
        sort($v);
      }
      else {
        ksort($v);
      }

      foreach ($v as $l => $m) {
        if (is_numeric($l)) {
          $key = '';
        }
        else {
          $key = $l;
        }
        if (is_array($m)) {
          sort($m);
          foreach ($m as $n => $o) {
            $code[] = "{$k}[$key][] = \"{$o}\"";
          }
        }
        else {
          $code[] = "{$k}[$key] = \"{$m}\"";
        }
      }
    }
    else {
      $code[] = "{$k} = \"{$v}\"";
    }
  }
  $code = implode("\n", $code) ."\n";
  return $code;
}

/**
 * Tar creation function. Written by dmitrig01.
 *
 * @param $files
 *   A keyed array where the key is the filepath and the value is the
 *   string contents of the file.
 *
 * @return
 *   A string of the tar file contents.
 */
function features_tar_create($files) {
  $tar = '';
  foreach ($files as $name => $contents) {
    $binary_data_first = pack("a100a8a8a8a12A12",
      $name,
      '100644 ', // File permissions
      '   765 ', // UID,
      '   765 ', // GID,
      sprintf("%11s ", decoct(strlen($contents))), // Filesize,
      sprintf("%11s", decoct(time())) // Creation time
    );
    $binary_data_last = pack("a1a100a6a2a32a32a8a8a155a12", '', '', '', '', '', '', '', '', '', '');

    $checksum = 0;
    for ($i = 0; $i < 148; $i++) {
      $checksum += ord(substr($binary_data_first, $i, 1));
    }
    for ($i = 148; $i < 156; $i++) {
      $checksum += ord(' ');
    }
    for ($i = 156, $j = 0; $i < 512; $i++, $j++) {
      $checksum += ord(substr($binary_data_last, $j, 1));
    }

    $tar .= $binary_data_first;
    $tar .= pack("a8", sprintf("%6s ", decoct($checksum)));
    $tar .= $binary_data_last;

    $buffer = str_split($contents, 512);
    foreach ($buffer as $item) {
      $tar .= pack("a512", $item);
    }
  }
  if (function_exists('gzencode')) {
    $tar = gzencode($tar);
  }
  return $tar;
}

/**
 * Export var function -- from Views.
 */
function features_var_export($var, $prefix = '', $init = TRUE) {
  if (is_array($var)) {
    if (empty($var)) {
      $output = 'array()';
    }
    else {
      $output = "array(\n";
      foreach ($var as $key => $value) {
        $output .= "  '$key' => " . features_var_export($value, '  ', FALSE) . ",\n";
      }
      $output .= ')';
    }
  }
  else if (is_bool($var)) {
    $output = $var ? 'TRUE' : 'FALSE';
  }
  else if (is_string($var) && strpos($var, "\n") !== FALSE) {
    // Replace line breaks in strings with a token for replacement
    // at the very end. This protects whitespace in strings from
    // unintentional indentation.
    $var = str_replace("\n", "***BREAK***", $var);
    $output = var_export($var, TRUE);
  }
  else {
    $output = var_export($var, TRUE);
  }

  if ($prefix) {
    $output = str_replace("\n", "\n$prefix", $output);
  }

  if ($init) {
    $output = str_replace("***BREAK***", "\n", $output);
  }

  return $output;
}

/**
 * Helper function to return an array of t()'d translatables strings.
 * Useful for providing a separate array of translatables with your
 * export so that string extractors like potx can detect them.
 */
function features_translatables_export($translatables, $prefix = '') {
  sort($translatables);
  $translatables = array_unique($translatables);

  $output = $prefix . "// Translatables\n";
  $output .= $prefix . "array(\n";
  foreach ($translatables as $string) {
    $output .= $prefix . "  t('" . strtr($string, array("'" => "\'")) . "'),\n";
  }
  $output .= $prefix . ");\n";
  return $output;
}

/**
 * Helper function to eliminate whitespace differences in code.
 */
function _features_linetrim($code) {
  foreach ($code as $k => $line) {
    $code[$k] = trim($line);
  }
  return $code;
}
