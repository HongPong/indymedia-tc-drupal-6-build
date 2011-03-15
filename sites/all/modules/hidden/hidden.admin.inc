<?php

/**
 * @file
 * Hidden configuration administration.
 */

/**
 * Menu callback; Module Configuration.
 *
 * @ingroup forms
 * @see theme_hidden_admin_settings()
 */
function hidden_admin_settings(&$form_state) {
  $form['hidden_logging'] = array(
    '#tree' => 'TRUE',
  );
  $form['hidden_logging']['header'] = array(
    '#value' => array(t('Type'), t('Logged')),
  );
  $logging = _hidden_settings_logging();
  $logform = array(
    HIDDEN_LOG_ERROR => array(
      'title' => t('Errors'),
      'description' => t('Error messages. You probably want to log these.'),
    ),
    HIDDEN_LOG_DEBUG_FILTER => array(
      'title' => t('Filter Debug'),
      'description' => t('Logs when content is filtered. You may want this if filters are not working as you expect.'),
    ),
    HIDDEN_LOG_DEBUG => array(
      'title' => t('Debug'),
      'description' => t('Debug messages. You probably don\'t want these at all.'),
    ),
  );
  foreach ($logform as $row => $content) {
    $form['hidden_logging'][$row]['name'] = array(
      '#type' => 'item',
      '#value' => $content['title'],
      '#description' => $content['description']
    );
    $form['hidden_logging'][$row]['enabled'] = array(
      '#type' => 'checkbox',
      '#default_value' => $logging[$row]['enabled'],
    );
  }

  $email = _hidden_settings_mail();
  $frequency = drupal_map_assoc(array(21600, 43200, 64800, 86400, 172800), 'format_interval');
  $form['hidden_email'] = array(
    '#tree' => 'TRUE',
  );
  $form['hidden_email']['domail'] = array(
     '#type' => 'checkbox',
     '#title' => t('enable e-mailing'),
     '#default_value' => $email['domail'],
     '#description' => t('if you want e-mail of hides, reports etc. enabled.'),
  );
  $form['hidden_email']['address'] = array(
     '#type' => 'textfield',
     '#title' => t('recipient address'),
     '#size' => 80,
     '#maxlength' => 255,
     '#default_value' => $email['address'],
     '#description' => t('e-mail address for hidden reports to be sent to'),
  );
  $form['hidden_email']['frequency'] = array(
    '#type' => 'select',
    '#title' => t('Periodic e-mail frequency'),
    '#options' => $frequency,
    '#default_value' => $email['frequency'],
    '#description' => t('How often any periodic e-mails should be sent out'),
  );
  $form['hidden_email']['header'] = array(
    '#value' => array(t('Log message'), t('e-mail')),
  );
  $emailform = array(
    HIDDEN_LOG_DELETE => array(
      'title' => t('Delete hidden content'),
      'description' => t('When hidden content is deleted'),
    ),
    HIDDEN_LOG_HIDE => array(
      'title' => t('Hide content'),
      'description' => t('When content is hidden'),
    ),
    HIDDEN_LOG_UNHIDE => array(
      'title' => t('Unhide content'),
      'description' => t('When hidden content is unhidden'),
    ),
    HIDDEN_LOG_REPORTED => array(
      'title' => t('Report content'),
      'description' => t('When content is reported for hiding'),
    ),
    HIDDEN_LOG_SEEN => array(
      'title' => t('Reported content marked as seen'),
      'description' => t('When content that has been reported is marked as seen'),
    ),
    HIDDEN_LOG_REASONS => array(
      'title' => t('Changes to hidden reasons'),
      'description' => t('When the reason options for hiding content are changed'),
    ),
  );
  if (module_exists('spam')) {
    $emailform[HIDDEN_LOG_SPAM] = array(
      'title' => t('Reported content marked as spam'),
      'description' => t('When reported content is marked as spam, only works from the hidden admin pages'),
    );
  }
  foreach ($emailform as $row => $content) {
    if (is_array($content)) {
      $form['hidden_email'][$row]['name'] = array(
        '#type' => 'item',
        '#title' => $content['title'],
        '#description' => $content['description'],
      );
      $form['hidden_email'][$row]['when'] = array(
        '#type' => 'radios',
        '#default_value' => $email[$row]['when'],
        '#options' => array(t('don\'t mail'), t('mail immediately'), t('mail periodically')),
      );
    }
  }
  $form['hidden_public_notes'] = array(
    '#title' => t('Allow public notes'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('hidden_public_notes', TRUE),
    '#description' => t('If unchecked, the form for public notes about hidden content will be disabled, only predefined reasons are available.')
  );
  return system_settings_form($form);
}

/********************************************************************
 * Reasons
 */

/**
 * Menu callback; Hidden admin change reasons for hiding
 */
function hidden_reasons_admin() {
  $enabled = array();
  $disabled = array();
  $reasons = hidden_reason_get_all(FALSE);
  foreach ($reasons as $reason) {
    if ($reason->enabled) {
      $enabled[] = array(
        check_plain($reason->title),
        l(t('edit'), 'admin/content/hidden/reasons/edit/'. $reason->rid),
        l(t('disable'), 'admin/content/hidden/reasons/disable/'. $reason->rid),
      );
    }
    else {
      $disabled[] = array(
        check_plain($reason->title),
        l(t('edit'), 'admin/content/hidden/reasons/edit/'. $reason->rid),
        l(t('enable'), 'admin/content/hidden/reasons/enable/'. $reason->rid),
      );
    }
  }
  if (count($enabled) == 0) {
    $enabled[] = array(t('No enabled reasons'), '', '');
  }
  if (count($disabled) == 0) {
    $disabled[] = array(t('No disabled reasons'), '', '');
  }

  $table = theme('item_list', array(l(t('Add new reason'), 'admin/content/hidden/reasons/add/'))); // yes I know one item
  $header = array(t('Reason'), array('data' => t('Operations'), 'colspan' => '2'));
  $table .= theme('table', $header, $enabled);
  $page = theme('box', t('Reasons for hiding'), $table);
  $table = theme('table', $header, $disabled);
  $page .= theme('box', t('Disabled reasons for hiding'), $table);

  return $page;
}

/**
 * Menu callback; Admin new or edit hidden reason
 *
 * @ingroup forms
 * @see hidden_reasons_admin_form_submit
 */
function hidden_reasons_admin_form(&$form_state, $op, $rid=0) {
  $rid = (int)$rid;
  if ($op == 'edit') {
    if (!($item = db_fetch_array(db_query('SELECT * FROM {hidden_reasons} WHERE rid = %d', $rid)))) {
      drupal_not_found();
      return;
    }
  }
  elseif ($op == 'add') {
    $item=array('rid' => '0', 'enabled' => '1');
  }
  else {
    _hidden_log(HIDDEN_LOG_DEBUG, 'hidden_reasons_admin_form() passed invalid $op');
    drupal_goto('admin/content/hidden/reasons');
  }

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => check_plain($item['title']),
    '#maxlength' => 255,
    '#description' => t('The name of the reason for hiding.'),
    '#required' => TRUE,
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $item['description'], // I want to filter this
    '#description' => t('The description of the reason. This could include a link to editorial guidelines.'),
  );
  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enabled'),
    '#default_value' => $item['enabled'],
    '#description' => t('If this reason is available to be chosen from the hiding menu.'),
  );
  $form['rid'] = array('#type' => 'value', '#value' => $item['rid']);
  $form['submit'] = array('#type' => 'submit', '#value' => t('Submit'));

  return $form;
}

/**
 * Implementation of form API hook; submit hidden_reasons_admin_form().
 */
function hidden_reasons_admin_form_submit($form, &$form_state) {
  $title = (string) $form_state['values']['title'];
  $description = (string) $form_state['values']['description'];
  $rid = (int) $form_state['values']['rid'];
  $enabled = (bool) $form_state['values']['enabled'];
  
  if ($rid) {
    $result = hidden_reason_update($title, $description, $enabled, $rid);
  }
  else {
    $result = hidden_reason_create($title, $description, $enabled);
  }

  $t_args = array('%title' => $title);
  if ($result == FALSE) {
    drupal_set_message(t('Error saving reason %title', $t_args), 'error');
    _hidden_log(HIDDEN_LOG_ERROR, t('Error saving reason %title', $t_args));
  }
  elseif ($result == $rid) {
    drupal_set_message(t('The reason %title has been updated.', $t_args));
    _hidden_log(HIDDEN_LOG_REASONS, t('Reason %title changed.', $t_args), 'reason', $rid);
  }
  else {
    drupal_set_message(t('The reason %title has been added.', $t_args));
    _hidden_log(HIDDEN_LOG_REASONS, t('Reason %title added.', $t_args), 'reason', $rid);
  }

  $form_state['redirect'] = 'admin/content/hidden/reasons';
}

/**
 * Menu callback; hidden_reasons_admin en/dis-able a reason
 */
function hidden_reasons_admin_able($action, $rid) {
  $rid = (int)$rid;

  if ($action == 'enable') {
    $result = hidden_reason_enable($rid);
  }
  elseif ($action == 'disable') {
    $result = hidden_reason_disable($rid);
  }
  else {
    _hidden_log(HIDDEN_LOG_DEBUG, 'hidden_reasons_admin_able() called with invalid $action');
  }

  if ($result) {
    $action = ($action == 'enable') ? 'enabled' : 'disabled';
    drupal_set_message(t('Reason %action', array('%action' => $action)));
  }
  else {
    $action = ($action == 'enable') ? 'enabling' : 'disabling';
    drupal_set_message(t('Error %action reason', array('%action' => $action)));
  }

  drupal_goto('admin/content/hidden/reasons');
}

/********************************************************************
 * Filters
 */

/**
 * Menu callback; administer filters.
 */
function hidden_filters_admin() {
  $enabled = theme('item_list', array(l(t('Add new filter'), 'admin/content/hidden/filters/add')));
  $enabled .= drupal_get_form('hidden_filters_admin_enabled_form');
  $page = theme('box', t('Enabled filters'), $enabled);

  $result = hidden_filter_all_get_pager(0, 0);
  $header = array(t('filter name'), t('hits'), t('last hit'), array('data' => t('operations'), '#colspan' => 2));
  $rows = array();
  while ($filter = db_fetch_object($result)) {
    $rows[] = array(
      check_plain($filter->title),
      $filter->hits,
      ($filter->date==0) ? t('Never') : format_date($filter->date, 'small'),
      l(t('edit'), 'admin/content/hidden/filters/edit/'. $filter->hfid),
      l(t('enable'), 'admin/content/hidden/filters/enable/'. $filter->hfid),
    );
  }
  if (count($rows)==0) {
    $rows[] = array(t('No disabled filters'), '', '', '', '');
  }
  $disabled = theme('table', $header, $rows);
  $page .= theme('box', t('Disabled filters'), $disabled);

  return $page;
}

/**
 * Menu callback; form to add or edit a filter
 *
 * @ingroup forms
 * @see hidden_filters_admin_form_validate()
 * @see hidden_filters_admin_form_submit()
 */
function hidden_filters_admin_form(&$form_state, $op, $hfid=0) {
  $hfid = (int) $hfid;
  if ($op == 'edit') {
    if (! $item=hidden_filter_load($hfid)) {
      drupal_not_found();
      return;
    }
  }
  elseif ($op == 'add') {
    $item = (object)array('hfid' => 0, 'enabled' => 1, 'rid' => 0, 'delay' => 1, 'type' => HIDDEN_FILTER_PLAIN, 'weight' => 0);
  }
  else {
    _hidden_log(HIDDEN_LOG_DEBUG, 'hidden_filters_admin_form() passed invalid $op');
    drupal_goto('admin/content/hidden/filters');
  }

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => isset($item->title) ? check_plain($item->title) : '',
    '#maxlength' => 255,
    '#description' => t('A name for the filter.'),
    '#required' => TRUE,
  );
  $form['filter'] = array(
    '#type' => 'textfield',
    '#title' => t('Filter'),
    '#default_value' => isset($item->filter) ? $item->filter : '',
    '#maxlength' => 255,
    '#description' => t('What to filter for.'),
    '#required' => TRUE,
  );
  $form['type'] = array(
    '#type' => 'radios',
    '#title' => t('Type of filter'),
    '#default_value' => $item->type,
    '#options' => array(HIDDEN_FILTER_PLAIN => t('Plain text'), HIDDEN_FILTER_PREG => t('Regular Expression')),
    '#description' => t('Plain text will search contents for the filter. A regular expression needs to be in the form of a Perl Compatible Regular Expression enclosed in /'),
    '#required' => TRUE,
  );
  $form['delay'] = array(
    '#type' => 'select',
    '#title' => t('Delay'),
    '#default_value' => $item->delay,
    '#options' => array(0 => t('immediate'), 1 => t('next cron job'), 1800 => t('30 minutes'), 3600 => t('1 hour'), 7200 => t('2 hours')),
    '#description' => t('If the hiding should be delayed. Immediate will report the hide to the poster. Others depend on cron so may take longer depending on how often cron is run'),
  );
  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => $item->weight,
    '#description' => t('The weight of the filter. Run is descending order. Needed if certain filters should be run first. Filtering stops when first match is found.'),
  );
  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enabled'),
    '#default_value' => $item->enabled,
    '#description' => t('If this filter is enabled.'),
  );
  if (isset($item->uid)) {
    $hide_user = user_load($item->uid);
    $item->name = is_object($hide_user) ? $hide_user->name : '';
  }
  // @TODO move _hidden_hide_form? 
  require_once('hidden.action-pages.inc');
  global $user;
  $form['hidden'] = _hidden_hide_form($user, $item);
  $form['hidden']['#type'] = 'fieldset';
  $form['hidden']['#title'] = t('Reason to use for hiding');
  $form['hfid'] = array('#type' => 'value', '#value' => $item->hfid);
  $form['submit'] = array('#type' => 'submit', '#value' => t('Submit'));
  return $form;
}

/**
 * Implementation of form API hook; validate hidden_filters_admin_form().
 */
function hidden_filters_admin_form_validate($form, &$form_state) {
  if ($form_state['values']['type']==HIDDEN_FILTER_PREG) {
    $msg = '';
    if (! _hidden_filter_preg_test($form_state['values']['filter'], $msg)) {
      form_set_error('filter', $msg);
    }
  }
}

/**
 * Implementation of form API hook; submit hidden_filters_admin_form().
 */
function hidden_filters_admin_form_submit($form, &$form_state) {
  $filter = new stdClass();
  $filter->hfid = (int) $form_state['values']['hfid'];
  $filter->title = (string) $form_state['values']['title'];
  $filter->filter = (string) $form_state['values']['filter'];
  $filter->type = (int) $form_state['values']['type'];
  $filter->delay = (int) $form_state['values']['delay'];
  $filter->weight = (int) $form_state['values']['weight'];
  $filter->enabled = (int) $form_state['values']['enabled'];
  if (user_access('administer hidden')) {
    $user = user_load(array('name' => $form_state['values']['user']));
    $filter->uid = is_object($user) ? $user->uid : 0;
  }
  else {
    global $user;
    $filter->uid = $user->uid;
  }
  $filter->rid = (int) $form_state['values']['reason'];
  if (! hidden_reason_check($filter->rid)) {
    $filter->rid = 0;
  }
  $filter->publicnote = (string) $form_state['values']['publictext'];
  $filter->privatenote = (string) $form_state['values']['privatetext'];

  $result = hidden_filter_save($filter);
  if ($result == FALSE) {
    drupal_set_message('Error saving filter.');
  }

  $form_state['redirect'] = 'admin/content/hidden/filters';
}

/**
 * Menu callback; execute sql to enable or disable a filter
 *
 * @param $action
 *   string 'enable' or 'disable'
 * @param $hfid
 *   int hidden filter id
 */
function hidden_filters_admin_able($action, $hfid) {
  $hfid = (int)$hfid;

  if ($action == 'enable') {
    $able = 1;
  }
  elseif ($action == 'disable') {
    $able = 0;
  }
  else {
    _hidden_log(HIDDEN_LOG_DEBUG, 'hidden_filters_admin_able() aclled with invalid $action');
    drupal_goto('admin/content/hidden/filters');
  }

  if ($filter = hidden_filter_load($hfid)) {
    $t_args = array('%title' => check_plain($filter->title), '%action' => $action);
    if (! db_query('UPDATE {hidden_filters} SET enabled=%d WHERE hfid=%d', $able, $hfid)) {
      _hidden_log(HIDDEN_LOG_DEBUG, 'hidden_filters_admin_able() failed to UPDATE to '. $action, 'filter', $hfid);
      drupal_set_message(t('Database error trying to %action reason %title', $t_args));
    }
    else {
      _hidden_log(HIDDEN_LOG_DEBUG_FILTER, t('Filter %title changed.', $t_args), 'filter', $hfid);
      drupal_set_message(t('Filter %title updated', $t_args));
    }
  }
  else {
    _hidden_log(HIDDEN_LOG_DEBUG, 'hidden_admin_filter_able() passed $hfid that does not exist', 'filter', $hfid);
    drupal_set_message(t('Tried to update a filter that does not exist'), 'error');
  }

  drupal_goto('admin/content/hidden/filters');
}

/**
 * form; list enabled filters, submission changes weight order.
 * 
 * @ingroup forms
 * @see hidden_filters_admin_enabled_form_submit()
 */
function hidden_filters_admin_enabled_form() {
  $form['header'] = array(
    '#type' => 'value',
    '#value' => array(
      array('data' => t('filter name')),
      array('data' => t('hits')),
      array('data' => t('date')),
      array('data' => t('weight')),
      array('data' => t('operations'), 'colspan' => 2),
    )
  );
  $result = hidden_filter_all_get_pager(0, 1);
  $form['weight'] = array('#tree' => TRUE);
  while ($filter = db_fetch_object($result)) {
    $form['title'][$filter->hfid] = array('#value' => check_plain($filter->title));
    $form['hits'][$filter->hfid] = array('#value' => $filter->hits);
    $form['date'][$filter->hfid] = array('#value' => ($filter->date==0) ? t('Never') : format_date($filter->date, 'small'));
    $form['weight'][$filter->hfid] = array('#type' => 'weight', '#default_value' => $filter->weight, '#tree' => TRUE);
    $form['edit'][$filter->hfid] = array('#value' => l(t('edit'), 'admin/content/hidden/filters/edit/'. $filter->hfid));
    $form['enable'][$filter->hfid] = array( '#value' => l(t('disable'), 'admin/content/hidden/filters/disable/'. $filter->hfid));
  }

  $form['submit'] = array('#value' => t('Update weights'),  '#type' => 'submit');
  return $form;
}

/**
 * theme hidden_filters_admin_enabled_form().
 *
 * @ingroup themeable
 */
function theme_hidden_filters_admin_enabled_form($form) {
  $rows = array();
  if (isset($form['title']) && is_array($form['title'])) {
    foreach (element_children($form['title']) as $key) {
      $row = array();
      $row[] = drupal_render($form['title'][$key]);
      $row[] = drupal_render($form['hits'][$key]);
      $row[] = drupal_render($form['date'][$key]);
      $row[] = drupal_render($form['weight'][$key]);
      $row[] = drupal_render($form['edit'][$key]);
      $row[] = drupal_render($form['enable'][$key]);
      $rows[] = $row;
    }
  }
  else {
    $rows[] = array(array('data' => t('No filters enabled'), 'colspan' => '6'));
  }

  $output = theme('table', $form['header']['#value'], $rows);
  $output .= drupal_render($form);
  return $output;
}

/**
 * Implementation of form API hook; submit hidden_filters_admin_form().
 */
function hidden_filters_admin_enabled_form_submit($form, &$form_state) {
  $error = FALSE;
  foreach ($form_state['values']['weight'] as $hfid => $weight) {
    $weight = array('hfid' => $hfid, 'weight' => $weight);
    if (! drupal_write_record('hidden_filters', $weight, array('hfid'))) {
      $error = TRUE;
    }
  }

  if ($error) {
    drupal_set_message(t('Problem updating weights'), 'error');
  }
  else {
    drupal_set_message(t('Weights updated'));
  }
}

/***********************************************************************
 * Theme
 */

/**
 * theme hidden_admin_settings() form.
 *
 * @ingroup themeable.
 */
function theme_hidden_admin_settings($form) {
  $rows = array();
  foreach (element_children($form['hidden_logging']) as $key) {
    $row = array();
    if (is_array($form['hidden_logging'][$key]['name'])) {
      $row[] = drupal_render($form['hidden_logging'][$key]['name']);
      $row[] = drupal_render($form['hidden_logging'][$key]['enabled']);
    }
    $rows[] = $row;
  }
  $logging = theme('table', $form['hidden_logging']['header']['#value'], $rows);
  unset($form['hidden_logging']['header']);

  $rows = array();
  foreach (element_children($form['hidden_email']) as $key) {
    $row = array();
    if (is_array($form['hidden_email'][$key]['when'])) {
      $row[] = drupal_render($form['hidden_email'][$key]['name']);
      $row[] = drupal_render($form['hidden_email'][$key]['when']);
    }
    $rows[] = $row;
  }
  $table = theme('table', $form['hidden_email']['header']['#value'], $rows);
  unset($form['hidden_email']['header']);
  $email = drupal_render($form['hidden_email']) . $table;

  $output = theme('box', t('Logging'), $logging);
  $output .= theme('box', t('e-mailing'), $email);

  $output .= drupal_render($form);

  return $output;
}
