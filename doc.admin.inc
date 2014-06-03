<?php

/**
 * @file
 * Administrative page callbacks for the googleanalytics module.
 */

/**
 * Implements hook_admin_settings() for module settings configuration.
 */
function doc_admin_settings_form($form,&$form_state) {
	$form['entities'] = array(
		'#type' => 'fieldset',
		'#title' => t('Documentation Module Settings'),
	);

	$form['entities']['doc_what'] = array(
	'#type' => 'checkboxes',
	'#title' => t('What to Document'),
		'#options' => array(
			'dr' => t('Drupal Implementation Overview'),
			'db' => t('Database'),
			'ct' => t('Content Types'),
			'md' => t('Modules'),
			'tx' => t('Taxonomies'),
			'mn' => t('Menus'),
			'th' => t('Themes'),
			'bl' => t('Blocks'),
			'vr' => t('System Variables'),
			'us' => t('Users'),
			'ur' => t('User Roles'),
		),
	);
	
	return system_settings_form($form);
}
/**
 * Returns HTML for the role order and new role form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function doc_documentation($form,$form_state) {
	$types = doc_types();
	$form['types'] = $types;
	
	//Build user role documentation
	$form = doc_roles($form,$form_state);

	//Build module documentation
	module_load_include('inc', 'system', 'system.admin');
	$form = doc_modules($form);

	return $form;
}
/**
 * Returns HTML for the role order and new role form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */

function doc_admin_settings_validate($form, &$form_state) {
}
/**
 * Returns HTML for the role order and new role form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function doc_admin_settings_submit($form, &$form_state) {
	// Submission logic.
}
/**
 * Returns HTML for the role order and new role form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function doc_form_alter_submit_handler($form,&$form_state) {
	global $doc_entities;
	$form_id = $form['#form_id'];

	$entity = $doc_entities[$form_id]['#entity'];
	$id = doc_convertArrayToKeys($doc_entities[$form_id]['#submit_id'],$form_state);

	$docValue = variable_get("doc_{$entity}_{$id}"); //Get previous value
		
	$docValue = doc_isset($form['doc']['fieldset']['documentation']['#value'],false,$docValue);
	
	variable_set("doc_{$entity}_{$id}",$docValue);
}
/**
 * Returns HTML for the role order and new role form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */

function doc_modules($form, $form_state = array()) {
  // Get current list of modules.
  $files = system_rebuild_module_data();

  // Remove hidden modules from display list.
  $visible_files = $files;
  foreach ($visible_files as $filename => $file) {
    if (!empty($file->info['hidden'])) {
      unset($visible_files[$filename]);
    }
  }

  uasort($visible_files, 'system_sort_modules_by_info_name');

  $modules = array();
  $form['modules'] = array('#tree' => TRUE);

  // Iterate through each of the modules.
  foreach ($visible_files as $filename => $module) {
    $extra = array();
    $extra['enabled'] = (bool) $module->status;
    $extra['disabled'] = TRUE;

    // If this module is required by other modules, list those
    foreach ($module->required_by as $required_by => $v) {
      // Hidden modules are unset already.
      if (isset($visible_files[$required_by])) {
        if ($files[$required_by]->status == 1 && $module->status == 1) {
          $extra['required_by'][] = t('@module (<span class="admin-enabled">enabled</span>)', array('@module' => $files[$required_by]->info['name']));
        }
        else {
          $extra['required_by'][] = t('@module (<span class="admin-disabled">disabled</span>)', array('@module' => $files[$required_by]->info['name']));
        }
      }
    }
    $form['modules'][$module->info['package']][$filename] = _system_modules_build_row($module->info, $extra);
  }

  // Add basic information to the fieldsets.
  foreach (element_children($form['modules']) as $package) {
    $form['modules'][$package] += array(
      '#type' => 'fieldset',
      '#title' => t($package),
      '#collapsible' => false,
      '#theme' => 'doc_modules',
      '#header' => array(
        array('data' => t('Enabled'), 'class' => array('checkbox')),
        t('Name'),
        t('Version'),
        t('Description'),
        array('data' => t('Documentation'), 'colspan' => 3),
      ),
      // Ensure that the "Core" package fieldset comes first.
      '#weight' => $package == 'Core' ? -10 : NULL,
    );
  }

  // Lastly, sort all fieldsets by title.
  uasort($form['modules'], 'element_sort_by_title');

  return $form;
}

/**
 * Form to display roles.
 *
 * @ingroup forms
 * @see theme_user_admin_roles()
 */
function doc_roles($form, $form_state) {
  $roles = user_roles();

  $form['roles'] = array(
    '#tree' => TRUE,
  );
  $order = 0;
  foreach ($roles as $rid => $name) {
    $form['roles'][$rid]['#role'] = (object) array(
      'rid' => $rid,
      'name' => $name,
      'weight' => $order,
    );
  }
  
  return $form;
}

function doc_types() {
  $types = node_type_get_types();
  $names = node_type_get_names();
  $field_ui = module_exists('field_ui');
  $header = array(t('Name'), array('data' => t('Documentation')));
  $rows = array();

  foreach ($names as $key => $name) {
    $type = $types[$key];
	$documentation = variable_get('doc_content_type_'.$name);
	
    if (node_hook($type->type, 'form')) {
      $type_url_str = str_replace('_', '-', $type->type);
      $row = array(theme('node_admin_overview', array('name' => $name, 'type' => $type)));
      // Set the documentation column.
      $row[] = array('data' => t($documentation));

      $rows[] = $row;
    }
  }

  $build['node_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No content types available.'),
  );

  return $build;
}
/**
 * Returns HTML for generating the module report.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_doc_modules($variables) {
  $form = $variables['form'];

  // Individual table headers.
  $rows = array();
  // Iterate through all the modules, which are
  // children of this fieldset.
  foreach (element_children($form) as $key) {
    // Stick it into $module for easier accessing.
    $module = $form[$key];
    $row = array();
    unset($module['enable']['#title']);
    $row[] = array('class' => array('checkbox'), 'data' => drupal_render($module['enable']));
    $label = '<label';
    if (isset($module['enable']['#id'])) {
      $label .= ' for="' . $module['enable']['#id'] . '"';
    }
    $row[] = $label . '><strong>' . drupal_render($module['name']) . '</strong></label>';
    $row[] = drupal_render($module['version']);
    // Add the description, along with any modules it requires.
    $description = drupal_render($module['description']);
    if ($module['#requires']) {
      $description .= '<div class="admin-requirements">' . t('Requires: !module-list', array('!module-list' => implode(', ', $module['#requires']))) . '</div>';
    }
    if ($module['#required_by']) {
      $description .= '<div class="admin-requirements">' . t('Required by: !module-list', array('!module-list' => implode(', ', $module['#required_by']))) . '</div>';
    }
    $row[] = array('data' => $description, 'class' => array('description'));

	$row[] = variable_get('doc_module_'.$key);
    $rows[] = $row;
  }

  return theme('table', array('header' => $form['#header'], 'rows' => $rows));
}
/**
 * Returns HTML for the role order and new role form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_doc_roles($variables) {
  $form = $variables['form'];

  $header = array(t('Name'), t('Weight'), array('data' => t('Operations'), 'colspan' => 2));
  foreach (element_children($form['roles']) as $rid) {
    $name = $form['roles'][$rid]['#role']->name;
    $row = array();
    if (in_array($rid, array(DRUPAL_ANONYMOUS_RID, DRUPAL_AUTHENTICATED_RID))) {
      $row[] = t('@name <em>(locked)</em>', array('@name' => $name));
      $row[] = drupal_render($form['roles'][$rid]['weight']);
      $row[] = '';
      $row[] = l(t('edit permissions'), 'admin/people/permissions/' . $rid);
    }
    else {
      $row[] = check_plain($name);
      $row[] = drupal_render($form['roles'][$rid]['weight']);
      $row[] = l(t('edit role'), 'admin/people/permissions/roles/edit/' . $rid);
      $row[] = l(t('edit permissions'), 'admin/people/permissions/' . $rid);
    }
    $rows[] = array('data' => $row);
  }
  $rows[] = array(array('data' => drupal_render($form['name']) . drupal_render($form['add']), 'colspan' => 4, 'class' => 'edit-name'));

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'user-roles')));
  $output .= drupal_render_children($form);

  return $output;
}
/**
 * Returns HTML for the module listing/editing page.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */

function doc_theme_system_modules_fieldset($variables) {
  $form = $variables['form'];

  // Individual table headers.
  $rows = array();
  // Iterate through all the modules, which are
  // children of this fieldset.
  foreach (element_children($form) as $key) {
    // Stick it into $module for easier accessing.
    $module = $form[$key];
    $row = array();
    unset($module['enable']['#title']);
    $row[] = array('class' => array('checkbox'), 'data' => drupal_render($module['enable']));
    $label = '<label';
    if (isset($module['enable']['#id'])) {
      $label .= ' for="' . $module['enable']['#id'] . '"';
    }
    $row[] = $label . '><strong>' . drupal_render($module['name']) . '</strong></label>';
    $row[] = drupal_render($module['version']);
    // Add the description, along with any modules it requires.
    $description = drupal_render($module['description']);
    if ($module['#requires']) {
      $description .= '<div class="admin-requirements">' . t('Requires: !module-list', array('!module-list' => implode(', ', $module['#requires']))) . '</div>';
    }
    if ($module['#required_by']) {
      $description .= '<div class="admin-requirements">' . t('Required by: !module-list', array('!module-list' => implode(', ', $module['#required_by']))) . '</div>';
    }
    $row[] = array('data' => $description, 'class' => array('description'));
    // Display links (such as help or permissions) in their own columns.
    foreach (array('help', 'permissions', 'configure','documentation') as $key) {
      $row[] = array('data' => drupal_render($module['links'][$key]), 'class' => array($key));
    }
    $rows[] = $row;
  }

  return theme('table', array('header' => $form['#header'], 'rows' => $rows));
}