<?php
// $Id: emfield.cck.inc,v 1.1.2.22 2009/08/24 20:36:23 aaron Exp $

/**
 * @file
 * Helper functions to implement our various cck-required functions, such as hook_field and hook_widget.
 */

/**
 *  helper function to consistently define field columns
 */
function _emfield_field_columns($field) {
  $columns = array(
    // poorly named. but this contains the original URL or embed code pre-parsing, as entered by the user/editor.
    'embed' => array('type' => 'text', 'size' => 'big', 'not null' => TRUE, 'not null' => FALSE, 'sortable' => TRUE),
    // this contains the code used by the provider to identify the media
    'value' => array('type' => 'varchar', 'length' => 255, 'not null' => TRUE, 'not null' => FALSE, 'sortable' => TRUE),
    // this is the actual provider used; matches up with the specific provider.inc file
    'provider' => array('type' => 'varchar', 'length' => 255, 'not null' => TRUE, 'not null' => FALSE, 'sortable' => TRUE),
    // an array for any extra data required by this media, such as unique thumbnail requirements or rss feed data
    'data' => array('type' => 'text', 'size' => 'big', 'not null' => TRUE, 'not null' => FALSE, 'sortable' => FALSE),
  );
  // allow other modules to add new columns
  // TODO: couldn't we just use module_invoke_all here? also make sure those modules do their magic on enable/disable
  // finally, is this actually used? seems extraneous with the data array (unless we're using views magic for them)
  foreach (module_implements('emfield_field_columns_extra') as $module) {
    $extra = module_invoke($module, 'emfield_field_columns_extra', $field);
    if (is_array($extra) && !empty($extra)) {
      $columns = array_merge($columns, $extra);
    }
  }

  return $columns;
}

/**
 * Implementation of hook_emfield_field().
 * This in turn is a callback for hook_field().
 *
 * This private function is returned directly by emfield_emfield_field().
 */
function _emfield_emfield_field($op, &$node, $field, &$items, $teaser, $page, $module) {
  $return = array();
  switch ($op) {
    // TODO: Nothing to validate at the moment. We need to have different provider api's have a chance to validate.
    case 'validate':
      foreach ($items as $delta => $item) {
        $error_field = $field['multiple'] ? $field['field_name'] .']['. $delta .'][embed' : $field['field_name'];
        _emfield_field_validate_id($field, $item, $error_field, $module, $delta);
      }
      break;

    case 'presave':
      foreach ($items as $delta => $item) {
        $list = _emfield_field_submit_id($field, $item, $module);
        $items[$delta]['provider'] = $list['provider'];
        $items[$delta]['value'] = $list['value'];
        $items[$delta]['data'] = $list['data'];
      }
      break;

    case 'load':
      // We need to unserialize the 'data' column manually.
      $field_name = $field['field_name'];
      foreach ($items as $delta => $item) {
        $data = (array)unserialize($items[$delta]['data']);
        $items[$delta]['data'] = $data;
        $node->{$field_name}[$delta]['data'] = $data;
      }

      $return[$field_name] = $items;
      break;

    case 'delete':
      break;
  }
  // Allow modules (such as emthumb) to alter our data.
  foreach (module_implements('emfield_field_extra') as $module) {
    $args = array($op, &$node, $field, &$items, $teaser, $page, $module);
    $ret = call_user_func_array($module .'_emfield_field_extra', $args);
    if (is_array($ret)) {
      $return = array_merge($return, $ret);
    }
  }
  if (in_array($op, array('insert', 'update'))) {
    // we need to manually serialize the 'data' array
    foreach ($items as $delta => $item) {
      $items[$delta]['data'] = serialize($items[$delta]['data']);
    }
  }
  return $return;
}

function _emfield_emfield_widget_settings($op, $widget, $module) {
  switch ($op) {
    case 'form':
      // Make sure to register the new type as supported by this module.
      emfield_implement_types(FALSE);

      $form = array();
      $options = array();
      $providers = emfield_system_list($module);
      foreach ($providers as $provider) {
        if (variable_get('emfield_'. $module .'_allow_'. $provider->name, TRUE)) {
          $info = emfield_include_invoke($module, $provider->name, 'info');
          $options[$provider->name] = $info['name'];
        }
      }
      $form['provider_list'] = array(
        '#type' => 'fieldset',
        '#title' => t('Providers Supported'),
        '#description' => t('Select which third party providers you wish to allow for this content type from the list below. If no checkboxes are checked, then all providers will be supported. When a user submits new content, the URL they enter will be matched to the provider, assuming that provider is allowed here.'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
      );
      $form['provider_list']['providers'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Providers'),
        '#default_value' => empty($widget['providers']) ? array() : $widget['providers'],
        '#options' => $options,
      );

      foreach (module_implements('emfield_widget_settings_extra') as $module) {
        $form[$module] = module_invoke($module, 'emfield_widget_settings_extra', 'form', $widget);
      }

      return $form;

    case 'save':
      $columns = array('providers'); //, 'helper_module', );
      foreach (module_implements('emfield_widget_settings_extra') as $module) {
        $columns = array_merge($columns, module_invoke($module, 'emfield_widget_settings_extra', 'save', $widget));
      }

      return $columns;
  }
}


/**
 * Helper function for emfield_emfield_widget, which in turn is a help function
 * for all emfields  that implement hook_widget().
 * This creates default widget handling for all the Embedded Media Fields.
 * Helper modules are expected to call this function to create the widget,
 * which will include a list of all providers as well as handle data parsing.
 */
function _emfield_emfield_widget(&$form, &$form_state, $field, $items, $delta = 0, $module) { //($op, &$node, $field, &$node_field, $module) {
  // Our form element will need to be processed as a tree, collapsing any children elements.
  $tree = array('#tree' => TRUE);

  $providers = emfield_allowed_providers($field, $module);
  $urls = array();
  $additional_form_elements = array();
  foreach ($providers as $provider) {
    // Only list providers allowed for this field. Honor global settings first.
    if (variable_get('emfield_'. $module .'_allow_'. $provider->name, TRUE)) {
      // Get the provider info.
      $info = emfield_include_invoke($module, $provider->name, 'info');
      // Grab the provider's URL.
      $urls[] = $info['url'] ? l($info['name'], $info['url'], array('attributes' => array('target' => '_blank'))) : $info['name'];
      // Allow the module to add any additional elements to the form, based on individual provider needs.
      $item = isset($items[$delta]) ? $items[$delta] : NULL;
      $additional_element = emfield_include_invoke($module, $provider->name, 'form', $field, $item);
      if ($additional_element) {
        $additional_form_elements[$provider->name] = $additional_element;
      }
    }
  }

  // Set the widget description, but allow the field to override this.
  if (!(empty($field['widget']['description']))) {
    $textfield_description = t('!description', array('!description' => content_filter_xss($field['widget']['description'])));
  }
  else {
    $textfield_description = t('Enter the URL or Embed Code here. The embedded third party content will be parsed and displayed appropriately from this.');
  }

  // Add a list of all supported third party providers.
  $textfield_description .= '<br />'. t('The following services are provided: !urls', array('!urls' => implode(', ', $urls)));
  // Get the value of our data, if it's been set for this node.
  $embed = isset($items[$delta]['embed']) ? $items[$delta]['embed'] : '';
  $value = isset($items[$delta]['value']) ? $items[$delta]['value'] : '';
  $tree['embed'] = array(
    '#type' => 'textfield',
    '#title' => t('@label', array('@label' => $field['widget']['label'])),
    '#description' => $textfield_description,
    '#default_value' => $embed,
    '#required' => $delta == 0 ? $field['required'] : FALSE,
    '#maxlength' => 4096,
  );

  $tree['value'] = array(
    '#type' => 'value',
    '#value' => $embed,
  );

  if (!empty($additional_form_elements)) {
    foreach ($additional_form_elements as $key => $element) {
      $tree[$key] = $element;
    }
  }

  if ($value) {
    $info = emfield_include_invoke($module, $items[$delta]['provider'], 'info');
    $tree['value_markup'] = array(
      '#type' => 'item',
      '#value' => t('(@provider ID: !value)', array('@provider' => $info['provider'], '!value' => l($value, emfield_include_invoke($module, $info['provider'], 'embedded_link', $value, $items[$delta]['data']), array('target' => '_blank')))),
    );
  }

  // Allow other modules, such as Embedded Media Thumbnail, to add additional elements to the widget.
  foreach (module_implements('emfield_widget_extra') as $module_extra) {
    $tree[$module_extra] = module_invoke($module_extra, 'emfield_widget_extra', $form, $form_state, $field, $items, $delta, $module);

    // In Drupal 6, we need to build multipart/form-data forms manually.
    if (function_exists($module_extra. '_emfield_widget_extra_file_included')) {
      if (call_user_func($module_extra. '_emfield_widget_extra_file_included')) {
        $form['#attributes'] = array('enctype' => "multipart/form-data");
      }
    }
  }
  return $tree;
}
