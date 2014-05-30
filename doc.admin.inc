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
function doc_documentation() {
	return "Testing";
}
function doc_admin_settings_validate($form, &$form_state) {
	// Validation logic.
}
function doc_admin_settings_submit($form, &$form_state) {
	// Submission logic.
}

function doc_form_alter_submit_handler($form,&$form_state) {
	global $doc_entities;
	$form_id = $form['#form_id'];

	$entity = $doc_entities[$form_id]['#entity'];
	$id = doc_convertArrayToKeys($doc_entities[$form_id]['#submit_id'],$form_state);

	dsm(get_defined_vars());
	
	$docValue = variable_get("doc_{$entity}_{$id}"); //Get previous value
		
	$docValue = doc_isset($form['doc']['fieldset']['documentation']['#value'],false,$docValue);
	
	variable_set("doc_{$entity}_{$id}",$docValue);
}