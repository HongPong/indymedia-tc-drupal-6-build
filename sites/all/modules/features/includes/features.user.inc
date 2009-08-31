<?php
// $Id: features.user.inc,v 1.1.2.5 2009/08/27 18:05:48 jmiccolis Exp $

/**
 * Implementation of hook_features_api().
 */
function user_features_api() {
  return array(
    'user' => array(
      'feature_source' => TRUE,
      'default_hook' => 'user_default_permissions',
    )
  );
}

/**
 * Implementation of hook_features_export().
 */
function user_features_export($data, &$export, $module_name = '') {
  // The default_permissions() hook integration is provided by the
  // features module so we need to add it as a dependency.
  $export['dependencies']['features'] = 'features';

  // Ensure the modules that provide the given permissions are included as dependencies.
  $map = _user_features_permission_map();
  foreach ($data as $perm) {
    if (isset($map[$perm])) {
      $perm_module = $map[$perm];
      $export['dependencies'][$perm_module] = $perm_module;
      $export['features']['user'][$perm] = $perm;
    }
  }

  $pipe = array();
  return $pipe;
}

/**
 * Implementation of hook_features_export_options().
 */
function user_features_export_options() {
  $options = array();
  foreach (module_list() as $module) {
    $prefix = check_plain($module) .': ';
    if ($permissions = module_invoke($module, 'perm')) {
      foreach ($permissions as $permission) {
        $options[$permission] = $prefix . check_plain($permission);
      }
    }
  }
  asort($options);
  return $options;
}

/**
 * Implementation of hook_features_export_render().
 */
function user_features_export_render($module = 'foo', $data) {
  $code = array();
  $code[] = '  $permissions = array();';
  $code[] = '';

  $permissions = _user_features_get_permissions();

  foreach ($data as $perm_name) {
    $permission = array();
    $permission['name'] = $perm_name;
    $permission['roles'] = isset($permissions[$perm_name]) ? $permissions[$perm_name] : array();

    $code[] = '  // Exported permission: '. $perm_name;
    $code[] = '  $permissions[] = '. features_var_export($permission, '  ') .';';
    $code[] = '';
  }

  $code[] = '  return $permissions;';
  $code = implode("\n", $code);
  return array('user_default_permissions' => $code);
}

/**
 * Implementation of hook_features_revert().
 *
 * @param $module
 * name of module to revert content for
 */
function user_features_revert($module = NULL) {
  // Get all fields defined in this feature
  if (module_hook($module, 'user_default_permissions')) {
    $roles = _features_get_roles();
    $defaults = module_invoke($module, 'user_default_permissions');
    foreach ($defaults as $permission) {
      foreach ($roles as $role_name => $role) {
        // If this role contains the given permission, but the permission does not include
        // this role, we need to revoke this permission from the role.
        if (in_array($permission['name'], $role['perm']) && !in_array($role_name, $permission['roles'])) {
          $position = array_search($permission['name'], $role['perm']);
          unset($roles[$role_name]['perm'][$position]);
        }
      }
    }

    // Write the updated permissions.
    _user_features_save_roles($roles);
  }
  else {
    drupal_set_message(t('Could not load default permissions.'), 'error');
    return FALSE;
  }
  return TRUE;
}

/**
 * Implementation of hook_features_rebuild().
 */
function user_features_rebuild() {
  $roles = _features_get_roles();

  // Iterate through default permissions and update the permissions map.
  $defaults = module_invoke_all('user_default_permissions');
  foreach ($defaults as $permission) {
    foreach ($permission['roles'] as $role) {
      // Ensure that each role exists. If it does not, create it and store the rid.
      if (!isset($roles[$role])) {
        $record = array('name' => $role);
        drupal_write_record('role', $record);
        $roles[$role] = array(
          'rid' => $record['rid'],
          'perm' => array(),
        );
      }
      $perm = $permission['name'];
      if (!in_array($perm, $roles[$role]['perm'])) {
        $roles[$role]['perm'][] = $perm;
      }
    }
  }

  // Write the updated permissions.
  _user_features_save_roles($roles);
}

/**
 * Generate a perm to module mapping.
 */
function _user_features_permission_map($reset = FALSE) {
  static $map;
  if (!isset($map) || $reset) {
    $map = array();
    foreach (module_implements('perm') as $module) {
      if ($permissions = module_invoke($module, 'perm')) {
        foreach ($permissions as $permission) {
          $map[$permission] = $module;
        }
      }
    }
  }
  return $map;
}

/**
 * Represent the current state of permissions as a perm to role name array map.
 */
function _user_features_get_permissions() {
  $result = db_query("SELECT p.perm, r.name FROM {role} r INNER JOIN {permission} p ON p.rid = r.rid");
  $permissions = array();
  while ($row = db_fetch_object($result)) {
    $role = $row->name;
    $role_perms = explode(', ', $row->perm);
    foreach ($role_perms as $perm) {
      if (!isset($permissions[$perm])) {
        $permissions[$perm] = array();
      }
      $permissions[$perm][] = $role;
    }
  }
  return $permissions;
}

/**
 * Save/update an array of roles to the database.
 */
function _user_features_save_roles($roles) {
  // Write the updated permissions.
  foreach ($roles as $role) {
    db_query("DELETE FROM {permission} WHERE rid = %d", $role['rid']);
    $role['perm'] = implode(', ', $role['perm']);
    drupal_write_record('permission', $role);
  }
}
