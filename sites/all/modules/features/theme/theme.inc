<?php
// $Id: theme.inc,v 1.1.2.5 2009/06/19 21:22:33 yhahn Exp $

/**
 * Display feature component info
 */
function template_preprocess_features_admin_components(&$vars) {
  drupal_add_css(drupal_get_path('module', 'features') .'/features.css');

  $rows = array();
  foreach ($vars['dependencies'] as $dependency => $status) {
    $row = array(
      array('data' => $dependency, 'class' => 'component'),
      theme('features_module_status', $status),
    );
    $rows[] = $row;
  }
  $vars['dependencies'] = theme('table', array(t('Dependency'), t('Status')), $rows);

  $rows = array();
  foreach ($vars['components'] as $type => $items) {
    $row = array(
      array('data' => $type, 'header' => TRUE),
      array('data' => $vars['conflicts'][$type], 'header' => TRUE),
    );
    $rows[] = $row;

    // Add a threshold check for displaying long component lists
    if (count($items) <= 10) {
      foreach ($items as $item) {
        $row = array(
          array('data' => $item, 'colspan' => 2, 'class' => 'component'),
        );
        $rows[] = $row;
      }
    }
    else {
      $row = array(
        array('data' => implode(', ', $items), 'colspan' => 2, 'class' => 'component'),
      );
      $rows[] = $row;      
    }
  }
  $components = theme('table', array(), $rows);
  $vars['components'] = $components;
}

/**
 * Themes a module status display.
 */
function theme_features_module_status($status, $module = NULL) {
  switch ($status) {
    case FEATURES_MODULE_ENABLED:
      $text = !empty($module) ? $module : t('Enabled');
      return "<span class='admin-enabled'>{$text}</span>";
    case FEATURES_MODULE_DISABLED:
      $text = !empty($module) ? $module : t('Disabled');
      return "<span class='admin-disabled'>{$text}</span>";
    case FEATURES_MODULE_MISSING:
      $text = !empty($module) ? $module : t('Missing');
      return "<span class='admin-missing'>{$text}</span>";
  }
}

/**
 * Themes a module status display.
 */
function theme_features_storage($storage) {
  switch ($storage) {
    case FEATURES_OVERRIDDEN:
      return "<span class='admin-overridden'>". t('Overridden') ."</span>";
    case FEATURES_DEFAULT:
      return "<span class='admin-default'>". t('Default') ."</span>";
  }
}

/**
 * Theme function for displaying form buttons
 */
function theme_features_form_buttons($element) {
  drupal_add_css(drupal_get_path('module', 'features') .'/features.css');

  $output = drupal_render($element);
  return !empty($output) ? "<div class='buttons clear-block'>{$output}</div>" : '';
}

/**
 * Theme for features management form.
 */
function theme_features_form($form) {
  drupal_add_css(drupal_get_path('module', 'features') .'/features.css');
  drupal_add_js(drupal_get_path('module', 'features') .'/features.js');

  $output = '';

  $header = array('', t('Feature'), t('Signature'), t('State'), t('Actions'));

  $rows = array();
  foreach (element_children($form['status']) as $element) {
    // Yank title & description fields off the form element for
    // rendering in their own cells.
    $name = "<div class='feature'>";
    $name .= "<strong>{$form['status'][$element]['#title']}</strong>";
    $name .= "<div class='description'>{$form['status'][$element]['#description']}</div>";
    $name .= "</div>";
    unset($form['status'][$element]['#title']);
    unset($form['status'][$element]['#description']);

    $row = array(
      'status' => drupal_render($form['status'][$element]),
      'name' => $name,
      'sign' => drupal_render($form['sign'][$element]),
      'state' => drupal_render($form['state'][$element]),
      'actions' => drupal_render($form['actions'][$element]),
    );

    // Determine row classes
    $class = $form['status'][$element]['#default_value'] ? 'enabled' : 'disabled';

    // Collect data + classes & add to master array.
    foreach ($row as $key => $data) {
      $row[$key] = array('data' => $data, 'class' => $key);
    }
    $rows[] = array('data' => $row, 'class' => $class);
  }

  if (empty($rows)) {
    $rows[] = array('', array('data' => t('No features available.'), 'colspan' => 4));
  }

  $output .= theme('table', $header, $rows, array('id' => 'features-form-table', 'class' => 'features'));

  // Prevent section from being rendered by drupal_render().
  unset($form['features']);

  $output .= drupal_render($form['buttons']);
  $output .= drupal_render($form);
  return $output;
}

/**
 * Theme functions ====================================================
 */

/**
 * Theme function for features_export_form (step 2)
 */
function theme_features_export_form_confirm($form) {
  drupal_add_css(drupal_get_path('module', 'features') .'/features.css');

  $output = drupal_render($form['step']);
  $rows = array();
  foreach (element_children($form['added']) as $element) {
    $row = array();
    $row[] = "<strong>{$form['added'][$element]['#title']}</strong>";
    unset($form['added'][$element]['#title']);

    $row[] = drupal_render($form['detected'][$element]);
    $row[] = drupal_render($form['added'][$element]);
    $rows[] = $row;
  }
  $output .= theme('table', array('', t('Auto-detected components'), t('Select additional components')), $rows, array('class' => 'features-export'));
  $output .= drupal_render($form);
  return $output;
}

