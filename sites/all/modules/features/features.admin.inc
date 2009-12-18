<?php
// $Id: features.admin.inc,v 1.1.2.21 2009/08/11 19:02:13 yhahn Exp $

/**
 * admin/build/features page callback.
 */
function features_admin_form($feature = NULL) {
  // Load export functions to use in comparison.
  module_load_include('inc', 'features', 'features.export');

  // Clear & rebuild key caches
  module_rebuild_cache();
  features_rebuild();

  $form = array('#theme' => 'features_form');

  $form['feature'] =
  $form['links'] =
  $form['version'] =
  $form['weight'] =
  $form['status'] = array(
    '#tree' => TRUE,
  );

  $modules = features_get_modules('', TRUE);
  $conflicts = features_get_conflicts();

  // Generate features form.
  foreach (features_get_features('', TRUE) as $name => $module) {
    $form['feature'][$name] = array(
      '#type' => 'value',
      '#value' => $module,
    );

    $disabled = FALSE;
    $description = $module->info['description'];

    // Detect unmet dependencies
    if (!empty($module->info['dependencies'])) {
      $unmet_dependencies = array();
      $dependencies = _features_export_maximize_dependencies($module->info['dependencies']);
      foreach ($dependencies as $dependency) {
        if (empty($modules[$dependency])) {
          $unmet_dependencies[] = theme('features_module_status', FEATURES_MODULE_MISSING, $dependency);
        }
      }
      if (!empty($unmet_dependencies)) {
        $description .= "<div class='dependencies'>". t('Unmet dependencies: !dependencies', array('!dependencies' => implode(', ', $unmet_dependencies))) ."</div>";
        $disabled = TRUE;
      }
    }

    // Detect potential conflicts
    if (!empty($conflicts[$name])) {
      $module_conflicts = array();
      foreach ($conflicts[$name] as $conflict) {
        $module_conflicts[] = theme('features_module_status', FEATURES_MODULE_MISSING, $conflict);
        // Only disable modules with conflicts if they are not already enabled.
        // If they are already enabled, somehow the user got themselves into a
        // bad situation and they need to be able to disable a conflicted module.
        if (module_exists($conflict) && !module_exists($name)) {
          $disabled = TRUE;
        }
      }
      $description .= "<div class='conflicts'>". t('Incompatible with: !conflicts', array('!conflicts' => implode(', ', $module_conflicts))) ."</div>";
    }

    $form['status'][$name] = array(
      '#type' => 'checkbox',
      '#title' => $module->info['name'],
      '#description' => $description,
      '#default_value' => $module->status,
      '#disabled' => $disabled,
    );

    $uri = !empty($module->info['project status url']) ? l(truncate_utf8($module->info['project status url'], 35, TRUE, TRUE), $module->info['project status url']) : t('Unavailable');
    $version = !empty($module->info['version']) ? $module->info['version'] : '';
    $version = !empty($version) ? "<div class='description'>$version</div>" : '';
    $sign = "$uri $version";

    $form['sign'][$name] = array(
      '#type' => 'markup',
      '#value' => $sign,
    );

    $actions = l(t('View'), "admin/build/features/{$name}");
    $actions .= '<span class="admin-overridden"> | </span>';
    $actions .= l(t('Update'), "admin/build/features/{$name}/update", array(
      'attributes' => array('class' => 'admin-overridden')
    ));
    $actions .= '<span class="admin-overridden"> | </span>';
    $actions .= l(t('Revert'), "admin/build/features/{$name}/revert", array(
      'attributes' => array('class' => 'admin-overridden')
    ));

    if ($module->status) {
      $state = '<span class="admin-loading">' . t('Checking...') . '</span>';
      $state .= l(t('Check'), "admin/build/features/{$name}/status", array('attributes' => array('class' => 'admin-check')));
      $state .= l(theme('features_storage', FEATURES_OVERRIDDEN), "admin/build/features/{$name}/compare", array(
        'html' => TRUE, 
        'attributes' => array('class' => 'admin-overridden')
      ));
      $state .= theme('features_storage', FEATURES_DEFAULT);
    }
    else {
      $state = t('Disabled');
    }

    $form['state'][$name] = array(
      '#type' => 'markup',
      '#value' => !empty($state) ? $state : '',
    );
    $form['actions'][$name] = array(
      '#type' => 'markup',
      '#value' => !empty($actions) ? $actions : '',
    );
  }

  $form['buttons'] = array(
    '#theme' => 'features_form_buttons',
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
    '#submit' => array('features_form_submit'),
    '#validate' => array('features_form_validate'),
  );
  return $form;
}

/**
 * Display the components of a feature.
 */
function features_admin_components($feature) {
  module_load_include('inc', 'features', 'features.export');
  $conflict = features_detect_overrides($feature);

  // Retrieve basic information
  $info = array();
  $info['name'] = $feature->info['name'];
  $info['description'] = $feature->info['description'];

  // Iterate over dependencies and retrieve status for display
  $dependencies = array();
  if (!empty($feature->info['dependencies'])) {
    foreach ($feature->info['dependencies'] as $dependency) {
      $status = features_get_module_status($dependency);
      $dependencies[$dependency] = $status;
    }
  }

  // Iterate over components and retrieve status for display
  $components = $feature->info['features'];
  $conflicts = array();
  foreach ($feature->info['features'] as $type => $items) {
    if (!empty($conflict[$type])) {
      $conflicts[$type] = l(theme('features_storage', FEATURES_OVERRIDDEN), "admin/build/features/{$feature->name}/compare", array('html' => TRUE));
      unset($conflict[$type]);
    }
    else {
      $conflicts[$type] = theme('features_storage', FEATURES_DEFAULT);
    }
  }
  // There are new additions -- show
  if (!empty($conflict)) {
    foreach ($conflict as $type => $items) {
      // If exportables are standardized with keyed arrays, this would work much better!
      $components[$type] = array();
      $conflicts[$type] = l(theme('features_storage', FEATURES_OVERRIDDEN), "admin/build/features/{$feature->name}/compare", array('html' => TRUE));
    }
  }

  return theme('features_admin_components', $info, $dependencies, $components, $conflicts);
}

/**
 * Validate handler for the 'manage features' form.
 */
function features_form_validate(&$form, &$form_state) {
  $conflicts = features_get_conflicts();
  foreach ($form_state['values']['status'] as $module => $status) {
    if ($status && !empty($conflicts[$module])) {
      foreach ($conflicts[$module] as $conflict) {
        if (!empty($form_state['values']['status'][$conflict])) {
          form_set_error('status', t('The feature !module cannot be enabled because it conflicts with !conflict.', array('!module' => $module, '!conflict' => $conflict)));
        }
      }
    }
  }
}

/**
 * Submit handler for the 'manage features' form
 */
function features_form_submit(&$form, &$form_state) {
  module_load_include('inc', 'features', 'features.export');

  $features = $form_state['values']['feature'];
  $install = array();
  $disable = array();
  if (!empty($features)) {
    foreach ($features as $name => $feature) {
      // Enable feature
      if ($form_state['values']['status'][$name] && !module_exists($name)) {
        if (!empty($feature->info['dependencies'])) {
          $dependencies = _features_export_maximize_dependencies($feature->info['dependencies']);
          foreach ($dependencies as $dependency) {
            if (!module_exists($dependency)) {
              $install[] = $dependency;
            }
          }
        }
        $install[] = $name;
      }
      // Disable feature
      else if (!$form_state['values']['status'][$name] && module_exists($name)) {
        $disable[] = $name;
      }
    }

    if (!empty($install)) {
      $install = array_unique($install);
      module_enable($install);

      // Make sure the install API is available.
      include_once './includes/install.inc';
      drupal_install_modules($install);
    }
    if (!empty($disable)) {
      module_disable($disable);
    }
  }

  // Clear drupal caches after enabling a feature. We do this in a separate
  // page callback rather than as part of the submit handler as some modules
  // have includes/other directives of importance in hooks that have already
  // been called in this page load.
  $form_state['#redirect'] = 'admin/build/features/cache-clear';
}

/**
 * Clear caches after enabling a feature.z
 */
function features_cache_clear() {
  drupal_flush_all_caches();
  drupal_goto('admin/build/features');
}

/**
 * Features revert form.
 */
function features_revert_form($form_state, $feature) {
  $form = array(
    'module' => array(
      '#type' => 'value',
      '#value' => $feature->name,
    ),
    'revert' => array(
      '#type' => 'checkboxes',
      '#title' => t('Revert all components'),
      '#options' => array(),
      '#default_value' => array(),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    ),
  );

  // Find all overridden items
  features_include();
  module_load_include('inc', 'features', 'features.export');
  $overrides = features_detect_overrides($feature);  
  foreach (array_keys($overrides) as $component) {
    if (features_hook($component, 'features_revert')) {
      $form['revert']['#options'][$component] = $component;
      $form['revert']['#default_value'][$component] = $component;
    }
  }

  return $form;
}

/**
 * Submit handler for revert form.
 */
function features_revert_form_submit(&$form, &$form_state) {
  features_include();
  $module = $form_state['values']['module'];
  foreach ($form_state['values']['revert'] as $component => $status) {
    if ($status) {
      $result = features_invoke($component, 'features_revert', $module);
      if ($result) {
        drupal_set_message(t('Reverted all <strong>!component</strong> components for <strong>!module</strong>.', array('!component' => $component, '!module' => $module)));
      }
      else {
        drupal_set_message(t('An error occurred trying to revert <strong>!component</strong> components for <strong>!module</strong>.', array('!component' => $component, '!module' => $module)));
      }
    }
  }
  $form_state['redirect'] = 'admin/build/features/'. $module;
}

/**
 * Page callback to display the differences between what's in code and
 * what is in the db.
 *
 * @param $module
 *  The name of the feature module to check display differences for.
 *
 * @return Themed display of what is different.
 */
function features_feature_comparison($feature) {
  drupal_add_css(drupal_get_path('module', 'features') .'/features.css');
  module_load_include('inc', 'features', 'features.export');

  $conflict = features_detect_overrides($feature);

  // There are differences
  if (!empty($conflict)) {
    // Diff is installed
    if (module_exists('diff')) {
      module_load_include('php', 'diff', 'DiffEngine');
      $formatter = new DrupalDiffFormatter();
      foreach ($conflict as $k => $v) {
        if (module_exists('diff')) {
          if (is_object($v['default']) && get_class($v['default']) == 'view' ){
            $old = $v['default']->export();
            $new = $v['current']->export();
          }
          else {
            $old = features_var_export($v['default']);
            $new = features_var_export($v['current']);
          }
          // Eliminate whitespace differences
          $old = _features_linetrim(explode("\n", $old));
          $new = _features_linetrim(explode("\n", $new));

          $diff = new Diff($old, $new);
          $output .= '<h3>' . $k . '</h3>';
          $output .= theme('diff_table', array(t('Default'), '', t('Current'), ''), $formatter->format($diff), array('class' => 'diff'));
        }
        else {
          $items[] = $k;
        }
      }
    }
    // Diff is not installed
    else {
      $output = "<div class='features-empty'>". t('Please install the diff module to view changes.') ."</div>";
    }
  }
  // No changes
  else {
    $output = "<div class='features-empty'>". t('No changes have been made to this feature.') ."</div>";
  }
  $output = "<div class='features-comparison'>{$output}</div>";
  return $output;
}

/**
 * Javascript call back that returns the status of a feature.
 */
function features_feature_status($feature) {
  module_load_include('inc', 'features', 'features.export');
  $storage = count(features_detect_overrides($feature)) ? FEATURES_OVERRIDDEN : FEATURES_DEFAULT;
  return drupal_json(array('status' => $storage));
}

/**
 * Detect potential conflicts between any features that provide
 * identical components.
 */
function features_get_conflicts($reset = FALSE) {
  $conflicts = array();
  $component_info = features_get_components();
  $map = features_get_component_map($reset);

  foreach ($map as $type => $components) {
    foreach ($components as $component => $modules) {
      if (isset($component_info[$type]['duplicates']) && $component_info[$type]['duplicates'] == FEATURES_DUPLICATES_ALLOWED) {
        continue;
      }
      else if (count($modules) > 1) {
        foreach ($modules as $module) {
          if (!isset($conflicts[$module])) {
            $conflicts[$module] = array();
          }
          foreach ($modules as $m) {
            if ($m != $module) {
              $conflicts[$module][$m] = $m;
            }
          }
        }
      }
    }
  }

  return $conflicts;
}

/**
 * Provide a component to feature map.
 */
function features_get_component_map($reset = FALSE) {
  static $map;

  if (!isset($map) || $reset) {
    $map = array();
    $features = features_get_features('', TRUE);
    foreach ($features as $feature) {
      foreach ($feature->info['features'] as $type => $components) {
        if (!isset($map[$type])) {
          $map[$type] = array();
        }
        foreach ($components as $component) {
          $map[$type][$component][] = $feature->name;
        }
      }
    }
  }

  return $map;
}
