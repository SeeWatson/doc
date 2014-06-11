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
			'fd' => t('Content Types'),
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
function doc_documentation($form,$form_state) {
  $vars = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE, // Added
    '#collapsed' => FALSE,  // Added
  );

  $form['documentation'] = array(
    '#title' => t('Site Documentation'),
  )+$vars;
  
	$form['documentation']['overview'] = array(
    '#type' => 'fieldset',
    '#title' => t('Overview'),
    'contents' => doc_overview(),
  )+$vars;

	$types = doc_types();
	$form['documentation']['types'] = array(
    '#title' => t('Content Types'),
    'contents' => $types
  )+$vars;
  
	$fields = doc_fields();
	$form['documentation']['fields'] = array(
    '#title' => t('Fields'),
    'contents' => $fields,
  )+$vars;

	//Build user role documentation
	$form['documentation']['roles'] = array(
    '#title' => t('Roles'),
    'contents' => doc_roles()
  )+$vars;

	//Build module documentation
	module_load_include('inc', 'system', 'system.admin');
	$form['documentation']['modules'] = array(
    '#title' => t('Modules'),
    'contents' => doc_modules($form),
  )+$vars;

	$form['documentation']['databases'] = array(
    '#title' => t('Databases'),
    '#value' => doc_databases(),
  )+$vars;
 
	return $form;
}
/**
 *   Function to display general documentation contents.
 *
 *   @param:
 *     $table - table name to show.
 *
 *   @return: HTML string
 */
function doc_overview() {
	$output = array(
		'#theme' => 'table',
//		'#header' => array('Core Site Documentation'),
		'#rows' => array(
			0 => array(
					variable_get('doc_configuration_site'),
				),
		),
	);
	return $output;
}

/**
 *   Function to display database contents.
 *
 *   @param:
 *     $table - table name to show.
 *
 *   @return: HTML string
 */
function doc_databases($optimize_tables=FALSE, $show_index=FALSE) {
  global $base_url, $databases;
  $output = '';
  
// $databases may be an array if using multiple databases.
  if (is_array($databases)) {
    foreach ($databases as $key => $value) {
      $dbname = $key;
      
      // Switch databases
      db_set_active($key);
      $fieldset = array(
        '#type' => 'fieldset',
        '#title' => $dbname,
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#value' => '<div class="doc_subsection">'. _doc_database($dbname, $value, $optimize_tables, $show_index) .'</div>',
        );
      $output .=  drupal_render($fieldset);
    }
     // Return to default database
    db_set_active('default');
  }
  else {
    $fieldset = array(
      '#title' => t('Database Information'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#value' => _doc_database('database', $databases, $optimize_tables, $show_index),
      );
    $output .=  theme('fieldset', array('element' => $fieldset));
  }
  return $output;
}

// Format the database information.
// This was split out when I was informed that the $databases variable could be an array.
function _doc_database($actdb, $databases, $optimize_tables=FALSE, $show_index=FALSE) {
  global $base_url;
  $output = '';

  $rows = array();
  $header = array();

  $db_types = array(
    'mysql' => 'MySQL',
    'mysqli' => 'MySQLi',
    'pgsql' => 'Postgres',
  );

  $output .= '<table cellpadding="5"><tr>';

  $output .= '<td valign="top">';
  $output .= '<h5>'. t('Overview') ."</h5>";
  $database_type = $db_types[ $databases[$actdb]['driver'] ];
  $database_version = Database::getConnection()->version();
  $rows[] = array(t('Database type'), isset($database_type) ? $database_type : t('Unknown'));
  $vers = isset($database_version) ? $database_version : t('Unknown');
  $rows[] = array(t('Version'), l($vers, 'admin/reports/status/sql'));

  $db = $databases[$actdb];

// Don't show password
  $db['password'] = str_repeat('&bull;', strlen($db['password']));
  $dbrows = array();
  foreach ($db as $key => $value) {
    $dbrows[] = array($key, $value);
  }
  $rows[] = array(t('Database URL'), theme('table', array('rows' => $dbrows)));

  $rows[] = array(t('Base URL'), $base_url);
  $rows[] = array(t('Database prefix'), empty($db_prefix) ? '- none -' : $db_prefix);
  if ($database_type == 'mysql' || $database_type == 'mysqli') {
    $grants = db_query('SHOW GRANTS');
    $privileges = trim(str_replace('GRANT', ' ', substr($grants, 0, strpos($grants, ' ON'))));
    $rows[] = array(t('Privileges'), array('data' => $privileges, 'colspan' => 5));
  }

  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $output .= "</td>";

  if ($database_type == 'pgsql') {
    $output .= "</tr></table>";
    return $output;
  }

  // Check if we have a version of MySql that supports the variable and status checks
  // TODO: Can Postgres do something similar?
  if (($database_type == 'mysql' && version_compare($database_version, '4.1.0', '>='))
    || $database_type == 'mysqli') {

    // The next two things are done as lists to
    //   a) simplify building the query,
    //   b) create the possibility of future settings page selection.

    // Get selected variables from the database.
    $variables_list = array(
      'character_set_database',
      'character_set_results',
      'collation_connection',
      'collation_database',
      'collation_server',
      'have_dynamic_loading',
      'have_innodb',
      'have_isam',
      'have_query_cache',
      'have_raid',
      'max_connections',
      'query_cache_size',
      'query_cache_type',
      );
    $output .= _doc_dbvars_list('VARIABLES', t('Selected Variables'), $variables_list, $database_version);

    // Get selected status information from the database.
    $variables_list = array(
      'Qcache_free_memory',
      'Qcache_hits',
      'Qcache_not_cached',
      'Qcache_queries_in_cache',
      'Max_used_connections',
      'Threads_running',
      'Threads_cached',
      'Threads_connected',
      'Threads_created',
      );
    $output .= _doc_dbvars_list('STATUS', t('Selected Status'), $variables_list, $database_version);

    $output .= "</tr></table>  ";
  } /* end MySql check */

  // Do the table status section.
  $result = db_query("SHOW TABLE STATUS");

  $header = array(t('Table'),t('Engine'),t('Version'),t('Row Format'),t('Rows'),t('Data Length'),t('Index Length'),t('Overhead'));
  
  // I used a class to align fields rather than 'align' because the some themes override 'align' in headers.
  $statrpt = "  <table><thead><tr>";
  $statrpt .= '<th>'. t('Table') .'</th>'
    .'<th>'. t('Engine') .'</th>'
    .'<th>'. t('Version') .'</th>'
    .'<th>'. t('Row Format') .'</th>'
    .'<th class="doc_right">'. t('Rows') .'</th>'
    .'<th class="doc_right">'. t('Data Length') .'</th>'
    .'<th class="doc_right">'. t('Index Length') .'</th>'
    .'<th class="doc_right">'. t('Overhead') .'</th>'
    .'</tr></thead><tbody>';
  $datalen = 0;
  $indexlen = 0;
  $overhead = 0;
  $row_count = 0;
  $rows = 0;
  $row_classes = array('even', 'odd');
  
  foreach ($result as $table) {
    $extras = array();
    (empty($table->Comment) ? NULL : $extras[] = $table->Comment);
    ($show_index ? $extras[] = _doc_show_index($table->Name) : NULL);
    
    ++$rows;
    $row_class = $row_classes[$rows & 1];
    $row_count += $table->Rows;
    $datalen += $table->Data_length;
    $indexlen += $table->Index_length;
    $overhead += $table->Data_free;
    $r = number_format($table->Rows);
    $d = number_format($table->Data_length);
    $i = number_format($table->Index_length);
    // Is there some overhead?
    if ($table->Data_free) {
      $o = number_format($table->Data_free);
      // Do we want to release it?
      if ($optimize_tables) {
        $worked = db_query('OPTIMIZE TABLE {' . $table->Name . '}');
        if ($worked) {
          $o .= '<br />'. t('released');
        }
      }
    }
    else { $o = NULL; }
    $statrpt .= "  ".'<tr class="'. $row_class .'">'
      .'<td>'. $table->Name .'</td>'
      .'<td>'. $table->Engine .'</td>'
      .'<td align="center">'. $table->Version .'</td>'
      .'<td>'. $table->Row_format .'</td>'
      .'<td class="doc_right">'. $r .'</td>'
      .'<td class="doc_right">'. $d .'</td>'
      .'<td class="doc_right">'. $i .'</td>'
      .'<td class="doc_right">'. ($table->Data_free ? $o : '') .'</td>'
      . (empty($table->Comment) ? NULL : '</tr><tr class="'. $row_class .'"><td></td><td colspan="10"><img src="/misc/menu-collapsed.png" width="7" height="7"> <em>'. $table->Comment .'</em></td>');
    if ($show_index) {
      $statrpt .= '</tr><tr class="'. $row_class .'"><td></td><td valign="top" align="right"><em>'. t('Indexes') .':</em></td><td colspan="10">'. _doc_show_index($table->Name) .'</td>';
    }
    $statrpt .= '</tr>';
    
    $row[] = array($table->Name,$table->Engine,$table->Version,$table->Row_format,$r,$d,$i,($table->Data_free ? $o : ''));

  } /* end while */
  $statrpt .= '<tr><td colspan="4">&nbsp;</td>'
             .'<td class="doc_right">----------</td>'
             .'<td class="doc_right">----------</td>'
             .'<td class="doc_right">----------</td>'
             .'<td class="doc_right">----------</td>'
             .'</tr>';
  $statrpt .= '<tr><td colspan="4"><em>total</em></td>'
             .'<td class="doc_right">'. number_format($row_count) .'</td>'
             .'<td class="doc_right">'. number_format($datalen) .'</td>'
             .'<td class="doc_right">'. number_format($indexlen) .'</td>'
             .'<td class="doc_right">'. number_format($overhead) .'</td>'
             .'</tr>';
  $statrpt .=  "  </table>";

  $rows = $row;
  
  dpm($rows);
  
  $tables_table = array(
    '#theme' => 'table',
    '#header' => $header, 
    '#rows' => $rows      
  );
  
  $fieldset = array(
    '#title' => t('Table Status'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#value' => theme($tables_table),
    //'#value' => '<div class="doc_subsection">'. $statrpt .'</div>',
  );
  $output .=  theme('fieldset', array('element' => $fieldset));

  $totsize = $datalen + $indexlen;
  $kb = $totsize / 1024;
  $mb = $kb / 1024;
  $gb = $mb / 1024;
  $units = 'KB';
  $howmany = $kb;
  if ($mb >= 1) {
    if ($gb >= 1) {
      $units = 'GB'; $howmany = $gb;
    }
    else  {
      $units = 'MB'; $howmany = $mb;
    }
  }

  $output .= t('Total database size is !num !unit', array('!num' => sprintf('%.1f', $howmany), '!unit' => $units)) .'</p>';
  drupal_set_message(t('!count tables found in !db.', array('!count' => $rows, '!db' => $actdb)), 'status');

  return $output;
}

/*
 *  Helper function to show the index structure of a database table.
 */
function _doc_show_index($table_name) {
  $output = NULL;
  $index_list = db_query('SHOW INDEX FROM '. $table_name);
  $indices = array();
  $header = array(t('Key Name'), t('Columns'), t('Collation'), t('Cardinality'));
  foreach ($index_list as $index) {
    if ($index->Seq_in_index == 1) {
      $indices[$index->Key_name] = array(
        'name' => $index->Column_name . ($index->Sub_part ? ' ('. $index->Sub_part .')' : NULL),
        'collation' => $index->Collation,
        'count' => $index->Cardinality,
        );
    }
    else {
      $indices[$index->Key_name]['name'] .= ', '. $index->Column_name . ($index->Sub_part ? ' ('. $index->Sub_part .')' : NULL);
    }
  }
  if (count($indices)) {
    $rows = array();
    foreach ($indices as $key_name => $values) {
      $rows[] = array($key_name,
        $values['name'],
        array('data' => $values['collation'], 'align' => 'center'),
        array('data' => $values['count'], 'align' => 'center')
        );
    }
    return theme('table', array('header' => $header, 'rows' => $rows));
  }
  else {
    return '<span class="admin-missing">No index found.</span> '. _doc_img_warning();
  }
}

/**
 *   Helper function for database variable list
 */
function _doc_dbvars_list($type, $title, $variables_list, $db_vers) {
  if (count($variables_list)) {
    $rows = array();
    $output .= '<td valign="top">';
    $output .= '<h5>'. $title ."</h5>";
  $show = 'SHOW '. $type;
  $vers = explode('.', $db_vers);
  switch ($vers[0]) {
    case 4:
      foreach ($variables_list as $key => $name) {
          $list = $show ." LIKE '". $name ."'";
          $result = db_query($list)->fetchAssoc();
          foreach ($result as $data) {
            $rows[] = array($data['Variable_name'], $data['Value']);
          }
    } /* end foreach */
        $output .= theme('table', array('header' => $header, 'rows' => $rows));
        $output .= "</td>  ";
    break;

    case 5:
        $list = $show ." WHERE Variable_Name LIKE '". implode("' OR Variable_Name LIKE '", $variables_list) ."'";
        $result = db_query($list)->fetchAssoc();
        foreach ($result as $data) {
          $rows[] = array($data['Variable_name'], $data['Value']);
        }
        $output .= theme('table', array('header' => $header, 'rows' => $rows));
        $output .= "</td>  ";
    break;

      default:
      drupal_set_message('Site Documentation: Unknown database version'. $vers[0], 'error');

  } /* end switch */
  } /* end count variables */
  return $output;
}

/**
 *   Function to display table contents.
 *
 *   @param:
 *     $table - table name to show.
 *     $rows_per_page - how many rows to format per page. Defaults to 20.
 *
 *   @return: HTML string
 */
function doc_show_table($table = NULL, $rows_per_page = 20) {
  if (!$table || !db_table_exists($table)) {
    drupal_set_message(t('You must supply a valid database table name.'), 'error');
    drupal_access_denied();
  }

  // We get the first (or only) part of the Primary key to be added to the sort sequence.
  $result = db_query("SHOW INDEX FROM {$table}")->fetchAssoc();
  $x = $result;
  if ($x === FALSE) {
    drupal_set_message(t("The '@table' table has no index defined. This is probably normal.", array('@table' => $table)), 'status');
    $first_key = NULL;
  }
  else {
    $first_key = $x['Column_name'];
  }

  drupal_set_title(t('@table Table Contents', array('@table' => ucwords($table))));
  $output = '<p>'. t('Click on a column title to sort by that column.') .'</p><br/>';
  $rows = array();

  // Now we get the column names from the table and build the header.
  $header = array();
  $result = db_query("SHOW COLUMNS FROM {$table}")->fetchAssoc();

  foreach ($result as $col_desc) {
    $header[] = array(
      'data' => ucwords(str_replace('_', ' ', $col_desc['Field'])),
      'field' => '`'. $col_desc['Field'] .'`',
      );
  }

  // Get the data rows from the table.
  $select = "SELECT * FROM {$table}";
  // Set it up so that the user can sort on any column, but the primary key will always be the last value to sort on.
  $select .= tablesort_sql($header) . ($first_key ? (', '. $first_key .' ASC') : NULL);
  // Do the query so that we can page the data.
  $result = pager_query($select, $rows_per_page);

  while ($row = db_fetch_array($result)) {
    $line = array();
    foreach ($row as $key => $value) {
      // We use check_plain for security.
      $line[] = check_plain($value);
    }
    $rows[] = $line;
  }

  // Build the displayable table.
  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $output .= theme('pager', $rows_per_page);
  return $output;
}

/**
 *   Function to display table contents.
 *
 *   @param:
 *     $table - table name to show.
 *     $rows_per_page - how many rows to format per page. Defaults to 20.
 *
 *   @return: HTML string
 */
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
 * Lists all defined fields for quick reference.
 */
function doc_fields() {
  $instances = field_info_instances();
  $field_types = field_info_field_types();
  $bundles = field_info_bundles();

  $modules = system_rebuild_module_data();

  $header = array(t('Field name'), t('Field type'), t('Used in'),t('Documentation'));
  $rows = array();
  foreach ($instances as $entity_type => $type_bundles) {
    foreach ($type_bundles as $bundle => $bundle_instances) {
      foreach ($bundle_instances as $field_name => $instance) {
        $field = field_info_field($field_name);

        // Initialize the row if we encounter the field for the first time.
        if (!isset($rows[$field_name])) {
          $rows[$field_name]['class'] = $field['locked'] ? array('menu-disabled') : array('');
          $rows[$field_name]['data'][0] = $field['locked'] ? t('@field_name (Locked)', array('@field_name' => $field_name)) : $field_name;
          $module_name = $field_types[$field['type']]['module'];
          $rows[$field_name]['data'][1] = $field_types[$field['type']]['label'] . ' ' . t('(module: !module)', array('!module' => $modules[$module_name]->info['name']));
        }

        // Add the current instance.
        $admin_path = _field_ui_bundle_admin_path($entity_type, $bundle);
        $rows[$field_name]['data'][2][] = $admin_path ? l($bundles[$entity_type][$bundle]['label'], $admin_path . '/fields') : $bundles[$entity_type][$bundle]['label'];
		
		$rows[$field_name]['data'][3] = variable_get('doc_field_'.$field_name,'test');
      }
    }
  }
  foreach ($rows as $field_name => $cell) {
    $rows[$field_name]['data'][2] = implode(', ', $cell['data'][2]);
  }
    // Sort rows by field name.
    ksort($rows);
    $output = array(
		'#theme' => 'table', 
		'#header' => $header, 
		'#rows' => $rows,
		'#empty' => t('No content types available.'),
	);
  return $output;
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
  $modules = array('#tree' => TRUE);

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
    $modules[$module->info['package']][$filename] = _system_modules_build_row($module->info, $extra);
  }

  // Add basic information to the fieldsets.
  foreach (element_children($modules) as $package) {
    $modules[$package] += array(
      '#type' => 'fieldset',
      '#title' => t($package),
      '#collapsible' => true,
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
  uasort($modules, 'element_sort_by_title');

  return $modules;
}

/**
 * Form to display roles.
 *
 * @ingroup forms
 * @see theme_user_admin_roles()
 */
function doc_roles() {
  $roles = user_roles();
  $header = array ('Role Name','Documentation');
  
  foreach($roles as $role) {
    $row[] = array($role,variable_get("doc_role_$role"));
  }
  
  $rows = $row;
  
  $output = array(
    '#theme' => 'table', 
    '#header' => $header, 
    '#rows' => $rows,
    '#empty' => t('No content types available.'),
  );
  return $output;
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
 * Overrides the core system module's theme_system_modules_fieldset.
 * Used to add the Documentation links to the modules pages
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
/**
 * Used to save documentation whenever a new module is installed.
 */
function doc_module_install_create_documentation($form,$form_state) {
  if ($form_state['values']['project_url']) {
    $field = 'project_url';
    $local_cache = update_manager_file_get($form_state['values']['project_url']);
    if (!$local_cache) {
//      form_set_error($field, t('Unable to retrieve Drupal project from %url.', array('%url' => $form_state['values']['project_url'])));
      return;
    }
  }
  elseif ($_FILES['files']['name']['project_upload']) {
    $validators = array('file_validate_extensions' => array(archiver_get_extensions()));
    $field = 'project_upload';
    if (!($finfo = file_save_upload($field, $validators, NULL, FILE_EXISTS_REPLACE))) {
      // Failed to upload the file. file_save_upload() calls form_set_error() on
      // failure.
      return;
    }
    $local_cache = $finfo->uri;
  }

  $directory = _update_manager_extract_directory();
  try {
    $archive = update_manager_archive_extract($local_cache, $directory);
  }
  catch (Exception $e) {
//    form_set_error($field, $e->getMessage());
    return;
  }

  $files = $archive->listContents();
  if (!$files) {
//    form_set_error($field, t('Provided archive contains no files.'));
    return;
  }

  // Unfortunately, we can only use the directory name to determine the project
  // name. Some archivers list the first file as the directory (i.e., MODULE/)
  // and others list an actual file (i.e., MODULE/README.TXT).
  $project = strtok($files[0], '/\\');

  $archive_errors = update_manager_archive_verify($project, $local_cache, $directory);
  if (!empty($archive_errors)) {
//    form_set_error($field, array_shift($archive_errors));
    // @todo: Fix me in D8: We need a way to set multiple errors on the same
    // form element and have all of them appear!
    if (!empty($archive_errors)) {
      foreach ($archive_errors as $error) {
//        drupal_set_message($error, 'error');
      }
    }
    return;
  }
  
  variable_set("doc_module_$project",doc_isset($form_state['values'],'documentation'));
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
function doc_module_create_documentation($form,$form_state,$module_name) {
  $path = drupal_get_path('module',$module_name);
  $title = Updater::getProjectTitle($path);

  $form['#module_id'] = $module_name;
  $form['#submit'] = array('doc_module_create_documentation_save');
  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
        '#type' => 'submit',
        '#value' => "Submit documentation for the $title module",
    )
  );
  return $form;
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
function doc_module_create_documentation_save($form,$form_state) {
  variable_set('doc_module_'.$form['#module_id'],$form_state['values']['documentation']);
}